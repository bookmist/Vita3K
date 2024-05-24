// Vita3K emulator project
// Copyright (C) 2023 Vita3K team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <module/module.h>
#include <modules/SceKernelModulemgr/SceModulemgr.h>
#include <util/tracy.h>

TRACY_MODULE_NAME(taihen);

#include "taihen/error.h"
#include "taihen/hen.h"
#include "taihen/module.h"
#include "taihen/patches.h"
#include "taihen/plugin.h"
#include "taihen/proc_map.h"
#include "taihen/taihen.h"
#include "taihen/taihen_internal.h"

/**
 * @brief      Add a hook given an absolute address
 *
 *             If target is the kernel, use KERNEL_PID as `pid`.
 *
 * @param[in]  pid        The pid of the target
 * @param[out] p_hook     A reference that can be used by the hook function
 * @param      dest_func  The function to patch (must be in the target address
 *                        space)
 * @param[in]  hook_func  The hook function (must be in the target address
 *                        space)
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to hook
 *             - TAI_ERROR_INVALID_KERNEL_ADDR if `pid` is kernel and address is in shared memory region
 */
EXPORT(SceUID, taiHookFunctionAbs, SceUID pid, tai_hook_ref_t *p_hook, Ptr<void> dest_func, Ptr<const void> hook_func) {
    return tai_hook_func_abs(emuenv, p_hook, pid, dest_func, hook_func);
}

/**
 * @brief      Add a hook to a module function export
 *
 *             If target is the kernel, use KERNEL_PID as `pid`. Since a module
 *             can have two libraries that export the same NID, you can
 *             optionally pass in the library NID of the one to hook. Otherwise,
 *             use `TAI_ANY_LIBRARY` and the first one found will be used.
 *
 * @param[in]  pid          The pid of the target
 * @param[out] p_hook       A reference that can be used by the hook function
 * @param[in]  module       Name of the target module.
 * @param[in]  library_nid  Optional. NID of the target library.
 * @param[in]  func_nid     The function NID. If `library_nid` is
 *                          `TAI_ANY_LIBRARY`, then the first export with the
 *                          NID will be hooked.
 * @param[in]  hook_func    The hook function (must be in the target address
 *                          space)
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to
 *               hook
 *             - TAI_ERROR_INVALID_KERNEL_ADDR if `pid` is kernel and address is
 *               in shared memory region
 *             - TAI_ERROR_INVALID_MODULE if `module` is `TAI_MAIN_MODULE`
 *               and `pid` is kernel
 */
EXPORT(SceUID, taiHookFunctionExportForKernel, SceUID pid, tai_hook_ref_t *p_hook, const char *module, uint32_t library_nid, uint32_t func_nid, Ptr<const void> hook_func) {
    int ret;
    Address func;

    ret = module_get_export_func(pid, module, library_nid, func_nid, &func);
    if (ret < 0) {
        LOG("Failed to find export for %s, NID:0x%08X: 0x%08X", module, func_nid, ret);
        return ret;
    }
    return CALL_EXPORT(taiHookFunctionAbs, pid, p_hook, Ptr<void>(func), hook_func);
}

/**
 * @brief      Add a hook to a module function import
 *
 *             If target is the kernel, use KERNEL_PID as `pid`. This will let
 *             you hook calls from one module to another without having to hook
 *             all calls to that module.
 *
 * @param[in]  pid                 The pid of the target
 * @param[out] p_hook              A reference that can be used by the hook
 *                                 function
 * @param[in]  module              Name of the target module.
 * @param[in]  import_library_nid  The imported library from the target module
 * @param[in]  import_func_nid     The function NID of the import
 * @param[in]  hook_func           The hook function (must be in the target
 *                                 address space)
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to
 *               hook
 *             - TAI_ERROR_INVALID_KERNEL_ADDR if `pid` is kernel and address is
 *               in shared memory region
 *             - TAI_ERROR_STUB_NOT_RESOLVED if the import has not been resolved
 *               yet. You should hook `sceKernelLoadStartModule`,
 *               `sceSysmoduleLoadModule` or whatever the application uses to
 *               start the imported module and add this hook after the module is
 *               loaded. Be sure to also hook module unloading to remove the
 *               hook BEFORE the imported module is unloaded!
 *             - TAI_ERROR_INVALID_MODULE if `module` is `TAI_MAIN_MODULE`
 *               and `pid` is kernel
 */
EXPORT(SceUID, taiHookFunctionImportForKernel, SceUID pid, tai_hook_ref_t *p_hook, const char *module, uint32_t import_library_nid, uint32_t import_func_nid, Ptr<const void> hook_func) {
    int ret;
    Address stubptr;
    uint32_t stub[3];

    ret = module_get_import_func(pid, module, import_library_nid, import_func_nid, &stubptr);
    if (ret < 0) {
        LOG("Failed to find stub for %s, NID:0x%08X: 0x%08X", module, import_func_nid, ret);
        return ret;
    }
    /*
    ret = tai_memcpy_to_kernel(pid, stub, Ptr<const void>(stubptr & ~1), sizeof(stub));
      if (ret < 0) {
        LOG("Failed to read stub %p, %x", stubptr, ret);
        return ret;
      }
      // FIXME: find a better way to do this
      if (stub[0] == 0xE24FC008 && stub[1] == 0xE12FFF1E) {
        LOG("stub for %p has not been resolved yet!", import_func_nid);
        return TAI_ERROR_STUB_NOT_RESOLVED;
      }
      */
    return CALL_EXPORT(taiHookFunctionAbs, pid, p_hook, Ptr<void>(stubptr), hook_func);
}

/**
 * @brief      Add a hook to a module manually with an offset
 *
 *             If target is the kernel, use KERNEL_PID as `pid`. The caller is
 *             responsible for checking that the module is of the correct
 *             version!
 *
 * @param[in]  pid        The pid of the target
 * @param[out] p_hook     A reference that can be used by the hook function
 * @param[in]  modid      The module UID from `taiGetModuleInfoForKernel`
 * @param[in]  segidx     The ELF segment index containing the function to patch
 * @param[in]  offset     The offset from the start of the segment
 * @param[in]  thumb      Set to 1 if this is a Thumb function
 * @param[in]  hook_func  The hook function (must be in the target address
 *                        space)
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to hook
 *             - TAI_ERROR_INVALID_KERNEL_ADDR if `pid` is kernel and address is in shared memory region
 */
EXPORT(SceUID, taiHookFunctionOffsetForKernel, SceUID pid, tai_hook_ref_t *p_hook, SceUID modid, int segidx, uint32_t offset, int thumb, Ptr<const void> hook_func) {
    int ret;
    Address addr;

    ret = module_get_offset(pid, modid, segidx, offset, &addr);
    if (ret < 0) {
        LOG("Failed to find offset for mod:%x, segidx:%d, offset:0x%08X: 0x%08X", modid, segidx, offset, ret);
        return ret;
    }
    if (thumb) {
        addr = addr | 1;
    }
    return CALL_EXPORT(taiHookFunctionAbs, pid, p_hook, Ptr<void>(addr), hook_func);
}

/**
 * @brief      Gets information on a currently loaded module
 *
 *             You should use this before calling
 *             `taiHookFunctionOffsetForKernel` in order to check that the
 *             module you wish to hook is currently loaded and that the module
 *             NID matches. The module NID changes in each version of the
 *             module.
 *
 * @param[in]  pid     The pid of the _caller_ (kernel should set to KERNEL_PID)
 * @param[in]  module  The name of the module
 * @param[out] info    The information to fill
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_MODULE if `module` is `TAI_MAIN_MODULE`
 *               and `pid` is kernel
 */
EXPORT(int, taiGetModuleInfoForKernel, SceUID pid, const char *module, tai_module_info_t *info) {
    return module_get_by_name_nid(pid, module, TAI_IGNORE_MODULE_NID, info);
}

/**
 * @brief      Release a hook
 *
 * @param[in]  tai_uid  The tai patch reference to free
 * @param[in]  hook     The hook to free
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to restore the function
 */
EXPORT(int, taiHookReleaseForKernel, SceUID tai_uid, tai_hook_ref_t hook) {
    return tai_hook_release(emuenv, tai_uid, hook);
}

/**
 * @brief      Injects data into a process bypassing MMU flags
 *
 * @param[in]  pid   The pid of the target (can be KERNEL_PID)
 * @param      dest  The destination in the process address space
 * @param[in]  src   The source in kernel address space
 * @param[in]  size  The size of the injection in bytes
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 */
EXPORT(SceUID, taiInjectAbsForKernel, SceUID pid, Ptr<void> dest, Ptr<const void> src, SceSize size) {
    return tai_inject_abs(emuenv, pid, dest, src, size);
}

/**
 * @brief      Inject data into a process bypassing MMU flags given an offset
 *
 * @param[in]  pid     The pid of the target (can be KERNEL_PID)
 * @param[in]  modid   The module UID from `taiGetModuleInfoForKernel`
 * @param[in]  segidx  Index of the ELF segment containing the data to patch
 * @param[in]  offset  The offset from the start of the segment
 * @param[in]  data    The data in kernel address space
 * @param[in]  size    The size of the injection in bytes
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 */
EXPORT(SceUID, taiInjectDataForKernel, SceUID pid, SceUID modid, int segidx, uint32_t offset, Ptr<const void> data, SceSize size) {
    int ret;
    Address addr;

    ret = module_get_offset(pid, modid, segidx, offset, &addr);
    if (ret < 0) {
        LOG("Failed to find offset for mod:%x, segidx:%d, offset:0x%08X: 0x%08X", modid, segidx, offset, ret);
        return ret;
    }
    return CALL_EXPORT(taiInjectAbsForKernel, pid, Ptr<void>(addr), data, size);
}

/**
 * @brief      Release an injection
 *
 * @param[in]  tai_uid  The tai patch reference to free
 *
 * @return     Zero on success, < 0 on error
 */
EXPORT(int, taiInjectReleaseForKernel, SceUID tai_uid) {
    return tai_inject_release(emuenv, tai_uid);
}

/**
 * @brief      Parses the taiHEN config and loads all plugins for a titleid to a
 *             process
 *
 *             `flags` are ignored!
 *
 * @param[in]  pid      The pid to load to
 * @param[in]  titleid  The title to read from the config
 * @param[in]  flags    Ignored.
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_SYSTEM if the config file is invalid
 */
EXPORT(int, taiLoadPluginsForTitleForKernel, SceUID pid, const char *titleid, int flags) {
    return plugin_load_all(pid, titleid);
}

/**
 * @brief      Reloads config.txt from the default path and optionally loads
 *             kernel plugins.
 *
 *             If `schedule` is set, then if called from a plugin `module_start`
 *             handler, it will schedule a config reload (and optionally load
 *             kernel plugins) after the current config parsing is completed. If
 *             `load_kernel` is set, then load kernel plugins defined in
 *             `*KERNEL` from the config file as well after config is reloaded
 *             successfully.
 *
 * @param[in]  schedule     If blocking, schedule reload until after load is
 *                          complete.
 * @param[in]  load_kernel  Load all kernel plugins defined in config.
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_BLOCKING if attempted to call from plugin start and
 *               `schedule` _is not set_.
 */
EXPORT(int, taiReloadConfigForKernel, int schedule, int load_kernel) {
    int ret;

    ret = plugin_load_config();
    if (ret == TAI_ERROR_BLOCKING && schedule) {
        plugin_delayed_load_config(load_kernel);
        ret = TAI_SUCCESS;
    }
    return ret;
}

/**
 * @brief      Module entry point
 *
 *             This module should be loaded by a kernel exploit. taiHEN expects
 *             the kernel environment to be clean, which means that no outside
 *             hooks and patches which may interfere with taiHEN.
 *
 *             If the user hold the L button while starting taiHEN, kernel
 *             plugins will be skipped.
 *
 * @param[in]  argc  Size of arguments (unused)
 * @param[in]  args  The arguments (unused)
 *
 * @return     Success always
 */
int module_start(EmuEnvState &emuenv, SceSize argc, const void *args) {
    // SceCtrlData ctrl;
    int ret;
    LOG("starting taihen...");
    ret = proc_map_init();
    if (ret < 0) {
        LOG("proc map init failed: %x", ret);
        return SCE_KERNEL_START_FAILED;
    }
    ret = patches_init(emuenv);
    if (ret < 0) {
        LOG("patches init failed: %x", ret);
        return SCE_KERNEL_START_FAILED;
    }
    ret = plugin_init();
    if (ret < 0) {
        LOG("plugin init failed: %x", ret);
        return SCE_KERNEL_START_FAILED;
    }
    ret = hen_add_patches();
    if (ret < 0) {
        LOG("HEN patches failed: %x", ret);
        return SCE_KERNEL_START_FAILED;
    }
    // ksceCtrlPeekBufferPositive(0, &ctrl, 1);
    // LOG("buttons held: 0x%08X", ctrl.buttons);
    // if (!(ctrl.buttons & (SCE_CTRL_LTRIGGER | SCE_CTRL_L1))) {
    ret = plugin_load_config();
    if (ret < 0) {
        LOG("HEN config load failed: %x", ret);
        return SCE_KERNEL_START_FAILED;
    }
    plugin_load_all(KERNEL_PID, "KERNEL");
    //} else {
    //    LOG("skipping plugin loading");
    //}
    return SCE_KERNEL_START_SUCCESS;
}

/**
 * @brief      Alias to inhibit compiler warning
 * @private
 */
// void _start() __attribute__((weak, alias("module_start")));

/**
 * @brief      Module cleanup
 *
 *             This cleans up the system and removes all hooks and patches. All
 *             handles held by plugins will be invalid after this point! This is
 *             called by the kernel module manager. In usual operation, you
 *             should not unload taiHEN.
 *
 * @param[in]  argc  Size of arguments (unused)
 * @param[in]  args  The arguments (unused)
 *
 * @return     Success always
 */
int module_stop(EmuEnvState &emuenv, SceSize argc, const void *args) {
    // TODO: release everything
    hen_remove_patches();
    plugin_deinit();
    patches_deinit(emuenv);
    proc_map_deinit();
    return SCE_KERNEL_STOP_SUCCESS;
}

// taihen-kernel

DECL_EXPORT(int, sceKernelGetProcessId) {
    return 1;
}

/**
 * @brief      Add a hook to a module function export for the calling process
 *
 * @see        taiHookFunctionExportForKernel
 *
 * @param[out] p_hook  A reference that can be used by the hook function
 * @param[in]  args    Call arguments
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to
 *               hook
 *             - TAI_ERROR_NOT_IMPLEMENTED if address is in shared memory region
 *             - TAI_ERROR_USER_MEMORY if pointers are incorrect
 *             - TAI_ERROR_INVALID_MODULE if `TAI_MAIN_MODULE` is specified and
 *               there are multiple main modules
 */
EXPORT(SceUID, taiHookFunctionExportForUser, tai_hook_ref_t *p_hook, tai_hook_args_t *args) {
    auto pid = CALL_EXPORT(sceKernelGetProcessId);
    return CALL_EXPORT(taiHookFunctionExportForKernel, pid, p_hook, args->module.get(emuenv.mem), args->library_nid, args->func_nid, args->hook_func);
}

/**
 * @brief      Add a hook to a module function import for the calling process
 *
 * @see        taiHookFunctionImportForKernel
 *
 * @param[out] p_hook  A reference that can be used by the hook function
 * @param[in]  args    Call arguments
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to
 *               hook
 *             - TAI_ERROR_NOT_IMPLEMENTED if address is in shared memory
 *               region. You should hook an import from another module instead.
 *             - TAI_ERROR_USER_MEMORY if pointers are incorrect
 *             - TAI_ERROR_STUB_NOT_RESOLVED if the import has not been resolved
 *               yet. You should hook `sceKernelLoadStartModule`,
 *               `sceSysmoduleLoadModule` or whatever the application uses to
 *               start the imported module and add this hook after the module is
 *               loaded. Be sure to also hook module unloading to remove the
 *               hook BEFORE the imported module is unloaded!
 *             - TAI_ERROR_INVALID_MODULE if `TAI_MAIN_MODULE` is specified and
 *               there are multiple main modules
 */
EXPORT(SceUID, taiHookFunctionImportForUser, tai_hook_ref_t *p_hook, tai_hook_args_t *args) {
    auto pid = CALL_EXPORT(sceKernelGetProcessId);

    return CALL_EXPORT(taiHookFunctionImportForKernel, pid, p_hook, args->module.get(emuenv.mem), args->library_nid, args->func_nid, args->hook_func);
}

/**
 * @brief      Add a hook to a module manually with an offset for the calling
 *             process
 *
 * @see        taiHookFunctionOffsetForKernel
 *
 * @param[out] p_hook  A reference that can be used by the hook function
 * @param[in]  args    Call arguments
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to
 *               hook
 *             - TAI_ERROR_NOT_IMPLEMENTED if address is in shared memory region
 *             - TAI_ERROR_USER_MEMORY if pointers are incorrect
 */
EXPORT(SceUID, taiHookFunctionOffsetForUser, tai_hook_ref_t *p_hook, tai_offset_args_t *args) {
    auto pid = CALL_EXPORT(sceKernelGetProcessId);
    return CALL_EXPORT(taiHookFunctionOffsetForKernel, pid, p_hook, args->modid, args->segidx, args->offset, args->thumb, args->source);
}

/**
 * @brief      Gets information on a currently loaded module
 *
 *             You can use the macro `TAI_MAIN_MODULE` for `module` to specify
 *             the main module. This is usually the module that is loaded first
 *             and is usually the eboot.bin. This will only work if there is
 *             only one module loaded in the main memory space. Not all
 *             processes have this property! Make sure you check the return
 *             value.
 *
 * @see        taiGetModuleInfoForKernel
 *
 * @param[in]  module  The name of the module or `TAI_MAIN_MODULE`.
 * @param[out] info    The information to fill
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_USER_MEMORY if `info->size` is too small or large or
 *               `module` is invalid
 *             - TAI_ERROR_INVALID_MODULE if `TAI_MAIN_MODULE` is specified and
 *               there are multiple main modules
 */
EXPORT(int, taiGetModuleInfo, const char *module, tai_module_info_t *info) {
    auto pid = CALL_EXPORT(sceKernelGetProcessId);
    return CALL_EXPORT(taiGetModuleInfoForKernel, pid, module, info);
}

/**
 * @brief      Release a hook for the calling process
 *
 * @see        taiHookReleaseForKernel
 *
 * @param[in]  tai_uid  The tai patch reference to free
 * @param[in]  hook     The hook to free
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_HOOK_ERROR if an internal error occurred trying to restore the function
 */
EXPORT(int, taiHookRelease, SceUID tai_uid, tai_hook_ref_t hook) {
    return CALL_EXPORT(taiHookReleaseForKernel, tai_uid, hook);
}

/**
 * @brief      Injects data into the current process bypassing MMU flags
 *
 * @see taiInjectAbsForKernel
 *
 * @param      dest  The address to inject
 * @param[in]  src   Source data
 * @param[in]  size  The size of the injection in bytes
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 */
EXPORT(SceUID, taiInjectAbs, Ptr<void> dest, Ptr<const void> src, SceSize size) {
    auto pid = CALL_EXPORT(sceKernelGetProcessId);
    return CALL_EXPORT(taiInjectAbsForKernel, pid, dest, src, size);
}

/**
 * @brief      Inject data into the current process bypassing MMU flags given an
 *             offset
 *
 * @see        taiInjectDataForKernel
 *
 * @param[in]  args   Call arguments
 *
 * @return     A tai patch reference on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if the address is already patched
 */
EXPORT(SceUID, taiInjectDataForUser, tai_offset_args_t *args) {
    auto pid = CALL_EXPORT(sceKernelGetProcessId);
    return CALL_EXPORT(taiInjectDataForKernel, pid, args->modid, args->segidx, args->offset, args->source, args->source_size);
}

/**
 * @brief      Release an injection for the calling process
 *
 * @see        taiInjectReleaseForKernel
 *
 * @param[in]  tai_uid  The tai patch reference to free
 *
 * @return     Zero on success, < 0 on error
 */
EXPORT(int, taiInjectRelease, SceUID tai_uid) {
    return CALL_EXPORT(taiInjectReleaseForKernel, tai_uid);
}

/**
 * @brief      Loads a kernel module
 *
 * @param[in]  path   The path to the skprx
 * @param[in]  flags  The flags
 * @param      opt    Optional arguments, set to NULL
 *
 * @return     A module reference on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `opt` is not NULL
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(SceUID, taiLoadKernelModule, char *path, int flags, void *opt) {
    if (/*ksceSblACMgrIsShell(0)*/ false) {
        return CALL_EXPORT(_sceKernelLoadModule, path, flags, nullptr);
    } else {
        return TAI_ERROR_NOT_ALLOWED;
    }
}

/**
 * @brief      Starts a kernel module
 *
 * @param[in]  modid  The id from `taiLoadKernelModule`
 * @param[in]  args   The arguments
 * @param      opt    Optional arguments, set to NULL
 * @param      res    Return value of `module_start`
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `args` is too large or `opt` is not
 *               NULL
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiStartKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
        tai_module_args_t kargs;
        char buf[MAX_ARGS_SIZE];
        uint32_t state;
        int ret;
        int k_res;
        SceUID pid;

        ENTER_SYSCALL(state);
        pid = ksceKernelGetProcessId();
        if (ksceSblACMgrIsShell(0)) {
            kargs.size = 0;
            ksceKernelMemcpyUserToKernel(&kargs, (uintptr_t)args, sizeof(kargs));
            if (kargs.size == sizeof(kargs)) {
                if (kargs.args <= MAX_ARGS_SIZE && opt == NULL) {
                    ret = ksceKernelKernelUidForUserUid(pid, modid);
                    if (ret >= 0) {
                        modid = ret;
                        ret = ksceKernelMemcpyUserToKernel(buf, (uintptr_t)kargs.argp, kargs.args);
                        if (ret >= 0) {
                            k_res = 0;
                            ret = ksceKernelStartModule(modid, kargs.args, buf, kargs.flags, NULL, &k_res);
                            if (res) {
                                ksceKernelMemcpyKernelToUser((uintptr_t)res, &k_res, sizeof(*res));
                            }
                        }
                    } else {
                        LOG("Error getting kernel uid for %x: %x", modid, ret);
                    }
                } else {
                    LOG("invalid args size: %x", kargs.size);
                    ret = TAI_ERROR_USER_MEMORY;
                }
            } else {
                ret = TAI_ERROR_INVALID_ARGS;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Loads and starts a kernel module
 *
 * @param[in]  path  The path of the skprx
 * @param[in]  args  The arguments
 *
 * @return     A module reference on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `args` is too large
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(SceUID, taiLoadStartKernelModuleForUser, const char *path, tai_module_args_t *args) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
    tai_module_args_t kargs;
    char buf[MAX_ARGS_SIZE];
    char k_path[MAX_NAME_LEN];
    uint32_t state;
    SceUID modid;
    int ret;
    SceUID pid;

    ENTER_SYSCALL(state);
    pid = ksceKernelGetProcessId();
    if (ksceSblACMgrIsShell(0)) {
        kargs.size = 0;
        ksceKernelMemcpyUserToKernel(&kargs, (uintptr_t)args, sizeof(kargs));
        if (kargs.size == sizeof(kargs)) {
            if (kargs.args <= MAX_ARGS_SIZE) {
                ret = ksceKernelMemcpyUserToKernel(buf, (uintptr_t)kargs.argp, kargs.args);
                if (ret >= 0) {
                    if (ksceKernelStrncpyUserToKernel(k_path, (uintptr_t)path, MAX_NAME_LEN) < MAX_NAME_LEN) {
                        ret = ksceKernelLoadStartModule(k_path, kargs.args, buf, kargs.flags, NULL, NULL);
                        LOG("loaded %s: %x", k_path, ret);
                        if (ret >= 0) {
                            ret = ksceKernelCreateUserUid(pid, ret);
                            LOG("user uid: %x", ret);
                        }
                    } else {
                        ret = TAI_ERROR_USER_MEMORY;
                    }
                }
            } else {
                ret = TAI_ERROR_INVALID_ARGS;
            }
        } else {
            LOG("invalid args size: %x", kargs.size);
            ret = TAI_ERROR_USER_MEMORY;
        }
    } else {
        ret = TAI_ERROR_NOT_ALLOWED;
    }
    EXIT_SYSCALL(state);
    return ret;*/
}

/**
 * @brief      Loads and starts a user module for another process
 *
 * @param[in]  path  The path of the skprx
 * @param[in]  args  The arguments
 *
 * @return     A module reference on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `args` is too large
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(SceUID, taiLoadStartModuleForPidForUser, const char *path, tai_module_args_t *args) {
    return TAI_ERROR_NOT_ALLOWED;
    /*    tai_module_args_t kargs;
        char buf[MAX_ARGS_SIZE];
        char k_path[MAX_NAME_LEN];
        uint32_t state;
        SceUID modid;
        int ret;

        ENTER_SYSCALL(state);
        if (ksceSblACMgrIsShell(0)) {
            kargs.size = 0;
            ksceKernelMemcpyUserToKernel(&kargs, (uintptr_t)args, sizeof(kargs));
            if (kargs.size == sizeof(kargs)) {
                if (kargs.args <= MAX_ARGS_SIZE) {
                    ret = ksceKernelMemcpyUserToKernel(buf, (uintptr_t)kargs.argp, kargs.args);
                    if (ret >= 0) {
                        if (ksceKernelStrncpyUserToKernel(k_path, (uintptr_t)path, MAX_NAME_LEN) < MAX_NAME_LEN) {
                            ret = ksceKernelLoadStartModuleForPid(kargs.pid, k_path, kargs.args, buf, kargs.flags, NULL, NULL);
                            LOG("loaded %s: %x", k_path, ret);
                            if (ret >= 0) {
                                ret = ksceKernelCreateUserUid(kargs.pid, ret);
                                LOG("user uid: %x", ret);
                            }
                        } else {
                            ret = TAI_ERROR_USER_MEMORY;
                        }
                    }
                } else {
                    ret = TAI_ERROR_INVALID_ARGS;
                }
            } else {
                LOG("invalid args size: %x", kargs.size);
                ret = TAI_ERROR_USER_MEMORY;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Stops a kernel module
 *
 * @param[in]  modid  The loaded module reference
 * @param[in]  args   The arguments
 * @param      opt    Optional arguments, set to NULL
 * @param      res    Return value of `module_stop`
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `args` is too large or `opt` is not NULL
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiStopKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
        tai_module_args_t kargs;
        char buf[MAX_ARGS_SIZE];
        uint32_t state;
        int ret;
        int k_res;
        SceUID pid;
        SceUID kid;

        ENTER_SYSCALL(state);
        pid = ksceKernelGetProcessId();
        if (ksceSblACMgrIsShell(0)) {
            kargs.size = 0;
            ksceKernelMemcpyUserToKernel(&kargs, (uintptr_t)args, sizeof(kargs));
            if (kargs.size == sizeof(kargs)) {
                if (kargs.args <= MAX_ARGS_SIZE && opt == NULL) {
                    kid = ksceKernelKernelUidForUserUid(pid, modid);
                    if (kid >= 0) {
                        ret = ksceKernelMemcpyUserToKernel(buf, (uintptr_t)kargs.argp, kargs.args);
                        if (ret >= 0) {
                            k_res = 0;
                            ret = ksceKernelStopModule(kid, kargs.args, buf, kargs.flags, NULL, &k_res);
                            if (res) {
                                ksceKernelMemcpyKernelToUser((uintptr_t)res, &k_res, sizeof(*res));
                            }
                            if (ret >= 0) {
                                ksceKernelDeleteUserUid(pid, modid);
                            }
                        }
                    } else {
                        LOG("Error getting kernel uid for %x: %x", modid, kid);
                        ret = kid;
                    }
                } else {
                    ret = TAI_ERROR_INVALID_ARGS;
                }
            } else {
                LOG("invalid args size: %x", kargs.size);
                ret = TAI_ERROR_USER_MEMORY;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Unloads a kernel module directly
 *
 * @param[in]  modid  The loaded module reference
 * @param[in]  flags  The flags
 * @param      opt    Set to `NULL`
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiUnloadKernelModule, SceUID modid, int flags, void *opt) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
        uint32_t state;
        SceUID pid;
        SceUID kid;
        int ret;

        ENTER_SYSCALL(state);
        pid = ksceKernelGetProcessId();
        if (ksceSblACMgrIsShell(0)) {
            if (opt == NULL) {
                kid = ksceKernelKernelUidForUserUid(pid, modid);
                if (kid >= 0) {
                    ret = ksceKernelUnloadModule(kid, flags, NULL);
                    if (ret >= 0) {
                        ksceKernelDeleteUserUid(pid, modid);
                    }
                } else {
                    LOG("Error getting kernel uid for %x: %x", modid, kid);
                    ret = kid;
                }
            } else {
                ret = TAI_ERROR_INVALID_ARGS;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Stops and unloads a kernel module
 *
 * @param[in]  modid  The loaded module reference
 * @param[in]  args   The arguments
 * @param      opt    Optional arguments, set to NULL
 * @param      res    Return value of `module_stop`
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `args` is too large or `opt` is not NULL
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiStopUnloadKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
        tai_module_args_t kargs;
        char buf[MAX_ARGS_SIZE];
        uint32_t state;
        int ret;
        int k_res;
        SceUID pid;
        SceUID kid;

        ENTER_SYSCALL(state);
        pid = ksceKernelGetProcessId();
        if (ksceSblACMgrIsShell(0)) {
            kargs.size = 0;
            ksceKernelMemcpyUserToKernel(&kargs, (uintptr_t)args, sizeof(kargs));
            if (kargs.size == sizeof(kargs)) {
                if (kargs.args <= MAX_ARGS_SIZE && opt == NULL) {
                    kid = ksceKernelKernelUidForUserUid(pid, modid);
                    if (kid >= 0) {
                        ret = ksceKernelMemcpyUserToKernel(buf, (uintptr_t)kargs.argp, kargs.args);
                        if (ret >= 0) {
                            k_res = 0;
                            ret = ksceKernelStopUnloadModule(kid, kargs.args, buf, kargs.flags, NULL, &k_res);
                            if (res) {
                                ksceKernelMemcpyKernelToUser((uintptr_t)res, &k_res, sizeof(*res));
                            }
                            if (ret >= 0) {
                                ksceKernelDeleteUserUid(pid, modid);
                            }
                        }
                    } else {
                        LOG("Error getting kernel uid for %x: %x", modid, kid);
                        ret = kid;
                    }
                } else {
                    ret = TAI_ERROR_INVALID_ARGS;
                }
            } else {
                LOG("invalid args size: %x", kargs.size);
                ret = TAI_ERROR_USER_MEMORY;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Stops a user module for another process
 *
 * @param[in]  modid  The loaded module reference
 * @param[in]  args   The arguments
 * @param      opt    Optional arguments, set to NULL
 * @param      res    Return value of `module_stop`
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `args` is too large or `opt` is not NULL
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiStopModuleForPidForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
        tai_module_args_t kargs;
        char buf[MAX_ARGS_SIZE];
        uint32_t state;
        int ret;
        int k_res;
        SceUID kid;

        ENTER_SYSCALL(state);
        if (ksceSblACMgrIsShell(0)) {
            kargs.size = 0;
            ksceKernelMemcpyUserToKernel(&kargs, (uintptr_t)args, sizeof(kargs));
            if (kargs.size == sizeof(kargs)) {
                if (kargs.args <= MAX_ARGS_SIZE && opt == NULL) {
                    kid = ksceKernelKernelUidForUserUid(kargs.pid, modid);
                    if (kid >= 0) {
                        ret = ksceKernelMemcpyUserToKernel(buf, (uintptr_t)kargs.argp, kargs.args);
                        if (ret >= 0) {
                            k_res = 0;
                            ret = ksceKernelStopModuleForPid(kargs.pid, kid, kargs.args, buf, kargs.flags, NULL, &k_res);
                            if (res) {
                                ksceKernelMemcpyKernelToUser((uintptr_t)res, &k_res, sizeof(*res));
                            }
                            if (ret >= 0) {
                                ksceKernelDeleteUserUid(kargs.pid, modid);
                            }
                        }
                    } else {
                        LOG("Error getting kernel uid for %x: %x", modid, kid);
                        ret = kid;
                    }
                } else {
                    ret = TAI_ERROR_INVALID_ARGS;
                }
            } else {
                LOG("invalid args size: %x", kargs.size);
                ret = TAI_ERROR_USER_MEMORY;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Unloads a user module for a process directly
 *
 * @param[in]  modid  The loaded module reference
 * @param[in]  flags  The flags
 * @param      opt    Set to `NULL`
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiUnloadModuleForPid, SceUID pid, SceUID modid, int flags, void *opt) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
        uint32_t state;
        SceUID kid;
        int ret;

        ENTER_SYSCALL(state);
        if (ksceSblACMgrIsShell(0)) {
            if (opt == NULL) {
                kid = ksceKernelKernelUidForUserUid(pid, modid);
                if (kid >= 0) {
                    ret = ksceKernelUnloadModuleForPid(pid, kid, flags, NULL);
                    if (ret >= 0) {
                        ksceKernelDeleteUserUid(pid, modid);
                    }
                } else {
                    LOG("Error getting kernel uid for %x: %x", modid, kid);
                    ret = kid;
                }
            } else {
                ret = TAI_ERROR_INVALID_ARGS;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Stops and unloads a user module for a process
 *
 * @param[in]  modid  The loaded module reference
 * @param[in]  args   The arguments
 * @param      opt    Optional arguments, set to NULL
 * @param      res    Return value of `module_stop`
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_ARGS if `args` is too large or `opt` is not NULL
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiStopUnloadModuleForPidForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
        tai_module_args_t kargs;
        char buf[MAX_ARGS_SIZE];
        uint32_t state;
        int ret;
        int k_res;
        SceUID kid;

        ENTER_SYSCALL(state);
        if (ksceSblACMgrIsShell(0)) {
            kargs.size = 0;
            ksceKernelMemcpyUserToKernel(&kargs, (uintptr_t)args, sizeof(kargs));
            if (kargs.size == sizeof(kargs)) {
                if (kargs.args <= MAX_ARGS_SIZE && opt == NULL) {
                    kid = ksceKernelKernelUidForUserUid(kargs.pid, modid);
                    if (kid >= 0) {
                        ret = ksceKernelMemcpyUserToKernel(buf, (uintptr_t)kargs.argp, kargs.args);
                        if (ret >= 0) {
                            k_res = 0;
                            ret = ksceKernelStopUnloadModuleForPid(kargs.pid, kid, kargs.args, buf, kargs.flags, NULL, &k_res);
                            if (res) {
                                ksceKernelMemcpyKernelToUser((uintptr_t)res, &k_res, sizeof(*res));
                            }
                            if (ret >= 0) {
                                ksceKernelDeleteUserUid(kargs.pid, modid);
                            }
                        }
                    } else {
                        LOG("Error getting kernel uid for %x: %x", modid, kid);
                        ret = kid;
                    }
                } else {
                    ret = TAI_ERROR_INVALID_ARGS;
                }
            } else {
                LOG("invalid args size: %x", kargs.size);
                ret = TAI_ERROR_USER_MEMORY;
            }
        } else {
            ret = TAI_ERROR_NOT_ALLOWED;
        }
        EXIT_SYSCALL(state);
        return ret;*/
}

/**
 * @brief      Gets an exported function address for a module of the calling process
 *
 * @param[in]  modname  The name of module to lookup
 * @param[in]  libnid   NID of the exporting library. Can be `TAI_ANY_LIBRARY`.
 * @param[in]  funcnid  NID of the exported function
 * @param[out] func     Output address of the function
 *
 * @return     Zero on success, < 0 on error
 */
EXPORT(int, taiGetModuleExportFunc, const char *modname, uint32_t libnid, uint32_t funcnid, Address *func) {
    auto pid = CALL_EXPORT(sceKernelGetProcessId);
    return module_get_export_func(pid, modname, libnid, funcnid, func);
}

/**
 * @brief      Copies data from user to kernel
 *
 * @param      kernel_dst  The kernel address
 * @param[in]  user_src    The user address
 * @param[in]  len         The length
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiMemcpyUserToKernel, Ptr<void> kernel_dst, Ptr<const void> user_src, SceSize len) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
    uint32_t state;
    int ret;

    ENTER_SYSCALL(state);
    if (ksceSblACMgrIsShell(0)) {
        ret = 0;
    } else {
        ret = TAI_ERROR_NOT_ALLOWED;
    }
    EXIT_SYSCALL(state);
    if (ret == 0) {
        return ksceKernelMemcpyUserToKernel(kernel_dst, (uintptr_t)user_src, len);
    } else {
        return ret;
    }*/
}

/**
 * @brief      Copies data from kernel to user
 *
 *             Does not bypass the MMU!
 *
 * @see        taiInjectData
 *
 * @param      user_dst    The user address
 * @param[in]  kernel_src  The kernel address
 * @param[in]  len         The length
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 */
EXPORT(int, taiMemcpyKernelToUser, void *user_dst, const void *kernel_src, size_t len) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
    uint32_t state;
    int ret;

    ENTER_SYSCALL(state);
    if (ksceSblACMgrIsShell(0)) {
        ret = 0;
    } else {
        ret = TAI_ERROR_NOT_ALLOWED;
    }
    EXIT_SYSCALL(state);
    if (ret == 0) {
        return ksceKernelMemcpyKernelToUser((uintptr_t)user_dst, kernel_src, len);
    } else {
        return ret;
    }*/
}

/**
 * @brief      Reloads config.txt from the default path
 *
 *             Note this cannot be called from a plugin start handler!
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_NOT_ALLOWED if caller does not have permission
 *             - TAI_ERROR_BLOCKING if attempted to call recursively
 */
EXPORT(int, taiReloadConfig) {
    return TAI_ERROR_NOT_ALLOWED;
    /*
    uint32_t state;
    int ret;

    ENTER_SYSCALL(state);
    if (ksceSblACMgrIsShell(0)) {
        ret = CALL_EXPORT(taiReloadConfigForKernel, 0, 0);
    } else {
        ret = TAI_ERROR_NOT_ALLOWED;
    }
    EXIT_SYSCALL(state);
    return ret;*/
}
