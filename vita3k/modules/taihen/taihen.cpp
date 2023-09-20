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

#include "nids/functions.h"

#include <module/module.h>
#include <util/tracy.h>

#include <kernel/state.h>
#include <kernel/types.h>

TRACY_MODULE_NAME(taihen);

/** PID for kernel process */
#define KERNEL_PID 0x10005

/** Fake library NID indicating that any library NID would match. */
#define TAI_ANY_LIBRARY 0xFFFFFFFF

/** Fake module NID indicating that any module NID would match. */
#define TAI_IGNORE_MODULE_NID 0xFFFFFFFF

/** Fake module name indicating the current process's main module. */
#define TAI_MAIN_MODULE ((void *)0)

/**
 * @brief      Extended module information
 *
 *             This supplements the output of `sceKernelGetModuleInfo`
 */
typedef struct _tai_module_info {
    uint32_t size; ///< Structure size, set to sizeof(tai_module_info_t)
    SceUID modid; ///< Module UID
    uint32_t module_nid; ///< Module NID
    char name[27]; ///< Module name
    Ptr<int> exports_start; ///< Pointer to export table in process address space
    Ptr<int> exports_end; ///< Pointer to end of export table
    Ptr<int> imports_start; ///< Pointer to import table in process address space
    Ptr<int> imports_end; ///< Pointer to end of import table
} tai_module_info_t;

/**
 * @brief      Pass hook arguments to kernel
 */
typedef struct _tai_hook_args {
    uint32_t size;
    Ptr<const char> module;
    uint32_t library_nid;
    uint32_t func_nid;
    Ptr<const void> hook_func;
} tai_hook_args_t;

/**
 * @brief      Pass offset arguments to kernel
 */
typedef struct _tai_offset_args {
    uint32_t size;
    SceUID modid;
    int segidx;
    uint32_t offset;
    int thumb;
    Ptr<const void> source;
    uint32_t source_size;
} tai_offset_args_t;

/**
 * @brief      Pass module arguments to kernel
 */
typedef struct _tai_module_args {
    uint32_t size;
    SceUID pid;
    uint32_t args;
    Ptr<const void> argp;
    int flags;
} tai_module_args_t;

/**
 * @brief      Hook information
 *
 *             This reference is created on new hooks and is up to the caller to
 *             keep track of. The client is responsible for cleanup by passing
 *             the reference back to taiHEN when needed.
 */
typedef Ptr<uint32_t> tai_hook_ref_t;

/**
 * @brief      Internal structure
 */
struct _tai_hook_user {
    Ptr<uint32_t> next;
    Ptr<void> func;
    Ptr<void> old;
};

#define TAI_SUCCESS 0
#define TAI_ERROR_SYSTEM 0x90010000
#define TAI_ERROR_MEMORY 0x90010001
#define TAI_ERROR_NOT_FOUND 0x90010002
#define TAI_ERROR_INVALID_ARGS 0x90010003
#define TAI_ERROR_INVALID_KERNEL_ADDR 0x90010004
#define TAI_ERROR_PATCH_EXISTS 0x90010005
#define TAI_ERROR_HOOK_ERROR 0x90010006
#define TAI_ERROR_NOT_IMPLEMENTED 0x90010007
#define TAI_ERROR_USER_MEMORY 0x90010008
#define TAI_ERROR_NOT_ALLOWED 0x90010009
#define TAI_ERROR_STUB_NOT_RESOLVED 0x9001000A
#define TAI_ERROR_INVALID_MODULE 0x9001000B
#define TAI_ERROR_MODULE_OVERFLOW 0x9001000C
#define TAI_ERROR_BLOCKING 0x9001000D

/** @name Kernel Hooks
 * Hooks exports to kernel
 */
/** @{ */
EXPORT(SceUID, taiHookFunctionAbs, SceUID pid, tai_hook_ref_t *p_hook, void *dest_func, const void *hook_func) {
    TRACY_FUNC(taiHookFunctionAbs, pid, p_hook, dest_func, hook_func);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiHookFunctionExportForKernel, SceUID pid, tai_hook_ref_t *p_hook, const char *module, uint32_t library_nid, uint32_t func_nid, const void *hook_func) {
    TRACY_FUNC(taiHookFunctionExportForKernel, pid, p_hook, module, library_nid, func_nid, hook_func);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiHookFunctionImportForKernel, SceUID pid, tai_hook_ref_t *p_hook, const char *module, uint32_t import_library_nid, uint32_t import_func_nid, const void *hook_func) {
    TRACY_FUNC(taiHookFunctionImportForKernel, pid, p_hook, module, import_library_nid, import_func_nid, hook_func);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiHookFunctionOffsetForKernel, SceUID pid, tai_hook_ref_t *p_hook, SceUID modid, int segidx, uint32_t offset, int thumb, const void *hook_func) {
    TRACY_FUNC(taiHookFunctionOffsetForKernel, pid, p_hook, modid, segidx, offset, thumb, hook_func);
    return UNIMPLEMENTED();
}
EXPORT(int, taiGetModuleInfoForKernel, SceUID pid, const char *module, tai_module_info_t *info) {
    TRACY_FUNC(taiGetModuleInfoForKernel, pid, module, info);
    return UNIMPLEMENTED();
}
EXPORT(int, taiHookReleaseForKernel, SceUID tai_uid, tai_hook_ref_t hook) {
    TRACY_FUNC(taiHookReleaseForKernel, tai_uid, hook);
    return UNIMPLEMENTED();
}
/** @} */

/**
 * @name User Hooks
 * Hooks exports to user
 */
/** @{ */
EXPORT(SceUID, taiHookFunctionExportForUser, tai_hook_ref_t *p_hook, tai_hook_args_t *args) {
    TRACY_FUNC(taiHookFunctionExportForUser, p_hook, args);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiHookFunctionImportForUser, tai_hook_ref_t *p_hook, tai_hook_args_t *args) {
    TRACY_FUNC(taiHookFunctionImportForUser, p_hook, args);
    LOG_CONSOLE(taiHookFunctionImportForUser, p_hook, args->module, args->library_nid, args->func_nid, args->hook_func);
    auto name = import_name(args->func_nid);
    LOG_TRACE("try to patch {} (nid:{})", name, log_hex(args->func_nid));
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiHookFunctionOffsetForUser, tai_hook_ref_t *p_hook, tai_offset_args_t *args) {
    TRACY_FUNC(taiHookFunctionOffsetForUser, p_hook, args);
    return UNIMPLEMENTED();
}

/**
 * @brief      Converts internal SCE structure to a usable form
 *
 *             This is needed since the internal SceKernelModulemgr structures
 *             change in different firmware versions.
 *
 * @param[in]  pid      The pid
 * @param[in]  sceinfo  Return from `ksceKernelGetModuleInternal`
 * @param[out] taiinfo  Output data structure
 *
 * @return     Zero on success, < 0 on error
 */
/*
static int sce_to_tai_module_info(SceUID pid, SceKernelModuleInfo *sceinfo, tai_module_info_t *taiinfo) {
if (taiinfo->size < sizeof(tai_module_info_t)) {
 LOG_ERROR("Structure size too small: %d", taiinfo->size);
 return TAI_ERROR_SYSTEM;
}

auto info = sceinfo;

 taiinfo->modid = info->modid;
   strncpy(taiinfo->name,info->module_name,sizeof(taiinfo->name));
 //snprintf(taiinfo->name, 27, "%s", *(const char **)(info + 0x1C));
 taiinfo->name[26] = '\0';
 taiinfo->module_nid = info->modid;//*(uint32_t *)(info + 0x30);
 //taiinfo->exports_start = info->//*(uintptr_t *)(info + 0x20);
 taiinfo->exports_end = *(uintptr_t *)(info + 0x24);
 taiinfo->imports_start = *(uintptr_t *)(info + 0x28);
 taiinfo->imports_end = *(uintptr_t *)(info + 0x2C);
//} else if (fw_version >= 0x1692000) {
 if (pid == KERNEL_PID) {
   taiinfo->modid = *(SceUID *)(info + 0x0);
 } else {
   taiinfo->modid = *(SceUID *)(info + 0x4);
 }
 taiinfo->module_nid = *(uint32_t *)(info + 0x3C);
 snprintf(taiinfo->name, 27, "%s", (const char *)(info + 0xC));
 taiinfo->name[26] = '\0';
 taiinfo->exports_start = *(uintptr_t *)(info + 0x2C);
 taiinfo->exports_end = *(uintptr_t *)(info + 0x30);
 taiinfo->imports_start = *(uintptr_t *)(info + 0x34);
 taiinfo->imports_end = *(uintptr_t *)(info + 0x38);
return TAI_SUCCESS;
}*/
EXPORT(int, taiGetModuleInfo, const char *module, tai_module_info_t *info) {
    TRACY_FUNC(taiGetModuleInfo, module, info);
    LOG_CONSOLE(taiGetModuleInfo, module, info);
    if (module == nullptr) {
        SceUID main_module_id = 0;
        for (auto &[module_id, module] : emuenv.kernel.loaded_modules) {
            if (module->path == "app0:" + emuenv.self_path) {
                info->modid = module_id;
                strncpy(info->name, module->module_name, sizeof(info->name));
                for (auto &[module_nid, module_uid] : emuenv.kernel.module_uid_by_nid) {
                    if (module_uid == module_id) {
                        info->module_nid = module_nid;
                    }
                }
                break;
            }
        }
    }
    // LOG_TRACE("taiGetModuleInfo,module:{}",module);
    return UNIMPLEMENTED();
}
EXPORT(int, taiHookRelease, SceUID tai_uid, tai_hook_ref_t hook) {
    TRACY_FUNC(taiHookRelease, tai_uid, hook);
    return UNIMPLEMENTED();
}

/** @} */

/**
 * @defgroup   inject Injection Interface
 * @brief      Inject raw data into a module.
 *
 *             Sometimes, there is a need to inject data directly. This can also
 *             be used to inject code for functions too small to be hooked.
 *             Unlike hooks only one module can patch a given module and given
 *             address at a time. Also note that the original data will be saved
 *             by the kernel. That means huge patches are not recommended!
 */
/** @{ */

/** @name Kernel Injections
 * Injection exports to kernel
 */
/** @{ */
EXPORT(SceUID, taiInjectAbsForKernel, SceUID pid, void *dest, const void *src, uint32_t size) {
    TRACY_FUNC(taiInjectAbsForKernel, pid, dest, src, size);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiInjectDataForKernel, SceUID pid, SceUID modid, int segidx, uint32_t offset, const void *data, uint32_t size) {
    TRACY_FUNC(taiInjectDataForKernel, pid, modid, segidx, offset, data, size);
    return UNIMPLEMENTED();
}
EXPORT(int, taiInjectReleaseForKernel, SceUID tai_uid) {
    TRACY_FUNC(taiInjectReleaseForKernel, tai_uid);
    return UNIMPLEMENTED();
}
/** @} */

/**
 * @name User Injections
 * Injection exports to user
 */
/** @{ */
EXPORT(SceUID, taiInjectAbs, void *dest, const void *src, uint32_t size) {
    TRACY_FUNC(taiInjectAbs, dest, src, size);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiInjectDataForUser, tai_offset_args_t *args) {
    TRACY_FUNC(taiInjectDataForUser, args);
    return UNIMPLEMENTED();
}
EXPORT(int, taiInjectRelease, SceUID tai_uid) {
    TRACY_FUNC(taiInjectRelease, tai_uid);
    return UNIMPLEMENTED();
}

/** @} */

/** @} */

/**
 * @name Plugin loading
 * Kernel plugin loading plugins manually
 */
/** @{ */
EXPORT(int, taiLoadPluginsForTitleForKernel, SceUID pid, const char *titleid, int flags) {
    TRACY_FUNC(taiLoadPluginsForTitleForKernel, pid, titleid, flags);
    return UNIMPLEMENTED();
}
EXPORT(int, taiReloadConfigForKernel, int schedule, int load_kernel) {
    TRACY_FUNC(taiReloadConfigForKernel, schedule, load_kernel);
    return UNIMPLEMENTED();
}
/** @} */

/**
 * @name Skprx Load
 * Kernel module loading exports to user
 */
/** @{ */

EXPORT(SceUID, taiLoadKernelModule, const char *path, int flags, void *opt) {
    TRACY_FUNC(taiLoadKernelModule, path, flags, opt);
    return UNIMPLEMENTED();
}
EXPORT(int, taiStartKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    TRACY_FUNC(taiStartKernelModuleForUser, modid, args, opt, res);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiLoadStartKernelModuleForUser, const char *path, tai_module_args_t *args) {
    TRACY_FUNC(taiLoadStartKernelModuleForUser, path, args);
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiLoadStartModuleForPidForUser, const char *path, tai_module_args_t *args) {
    TRACY_FUNC(taiLoadStartModuleForPidForUser, path, args);
    return UNIMPLEMENTED();
}
EXPORT(int, taiStopKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    TRACY_FUNC(taiStopKernelModuleForUser, modid, args, opt, res);
    return UNIMPLEMENTED();
}
EXPORT(int, taiUnloadKernelModule, SceUID modid, int flags, void *opt) {
    TRACY_FUNC(taiUnloadKernelModule, modid, flags, opt);
    return UNIMPLEMENTED();
}
EXPORT(int, taiStopUnloadKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    TRACY_FUNC(taiStopUnloadKernelModuleForUser, modid, args, opt, res);
    return UNIMPLEMENTED();
}
EXPORT(int, taiStopModuleForPidForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    TRACY_FUNC(taiStopModuleForPidForUser, modid, args, opt, res);
    return UNIMPLEMENTED();
}
EXPORT(int, taiUnloadModuleForPid, SceUID pid, SceUID modid, int flags, void *opt) {
    TRACY_FUNC(taiUnloadModuleForPid, pid, modid, flags, opt);
    return UNIMPLEMENTED();
}
EXPORT(int, taiStopUnloadModuleForPidForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    TRACY_FUNC(taiStopUnloadModuleForPidForUser, modid, args, opt, res);
    return UNIMPLEMENTED();
}

/** @} */

/**
 * @name NID Lookup
 * Function NID Lookup Interface
 */
/** @{ */

EXPORT(int, taiGetModuleExportFunc, const char *modname, uint32_t libnid, uint32_t funcnid, Ptr<uint32_t> *func) {
    TRACY_FUNC(taiGetModuleExportFunc, modname, libnid, funcnid, func);
    return UNIMPLEMENTED();
}

/** @} */

/**
 * @name Peek/Poke
 * Read/write kernel memory (no MMU bypass)
 */
/** @{ */

EXPORT(int, taiMemcpyUserToKernel, void *kernel_dst, const void *user_src, uint32_t len) {
    TRACY_FUNC(taiMemcpyUserToKernel, kernel_dst, user_src, len);
    memcpy(kernel_dst, user_src, len);
    return 0;
}
EXPORT(int, taiMemcpyKernelToUser, void *user_dst, const void *kernel_src, uint32_t len) {
    TRACY_FUNC(taiMemcpyKernelToUser, user_dst, kernel_src, len);
    memcpy(user_dst, kernel_src, len);
    return 0;
}

/** @} */

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
    TRACY_FUNC(taiReloadConfig);
    return UNIMPLEMENTED();
}

/** @} */

/**
 * @defgroup   module NID Lookup Interface
 */
/** @{ */

EXPORT(int, module_get_by_name_nid, SceUID pid, const char *name, uint32_t nid, tai_module_info_t *info) {
    TRACY_FUNC(module_get_by_name_nid, pid, name, nid, info);
    return UNIMPLEMENTED();
}
EXPORT(int, module_get_offset, SceUID pid, SceUID modid, int segidx, uint32_t offset, Ptr<uint32_t> *addr) {
    TRACY_FUNC(module_get_offset, pid, modid, segidx, offset, addr);
    return UNIMPLEMENTED();
}
EXPORT(int, module_get_export_func, SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, Ptr<uint32_t> *func) {
    TRACY_FUNC(module_get_export_func, pid, modname, libnid, funcnid, func);
    return UNIMPLEMENTED();
}
EXPORT(int, module_get_import_func, SceUID pid, const char *modname, uint32_t target_libnid, uint32_t funcnid, Ptr<uint32_t> *stub) {
    TRACY_FUNC(module_get_import_func, pid, modname, target_libnid, funcnid, stub);
    return UNIMPLEMENTED();
}

/** @} */
