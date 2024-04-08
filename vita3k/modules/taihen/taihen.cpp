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

#define LOG LOG_DEBUG

/* This file gets included multiple times to generate the host-visible and target-visible versions of each struct */

#define SCE_TYPE(type) type##_raw
#define SCE_PTR(type) Ptr<type>

#include <stdint.h>

struct SCE_TYPE(sce_module_exports);
struct SCE_TYPE(sce_module_imports);

typedef struct SCE_TYPE(sce_module_exports) {
    uint16_t size; /* Size of this struct, set to 0x20 */
    uint16_t version; /* 0x1 for normal export, 0x0 for main module export */
    uint16_t flags; /* 0x1 for normal export, 0x8000 for main module export */
    uint16_t num_syms_funcs; /* Number of function exports */
    uint32_t num_syms_vars; /* Number of variable exports */
    uint32_t num_syms_tls_vars; /* Number of TLS variable exports */
    uint32_t library_nid; /* NID of this library */
    SCE_PTR(const char *)
    library_name; /* Pointer to name of this library */
    SCE_PTR(uint32_t *)
    nid_table; /* Pointer to array of 32-bit NIDs to export */
    SCE_PTR(const void **)
    entry_table; /* Pointer to array of data pointers for each NID */
} SCE_TYPE(sce_module_exports);

typedef struct SCE_TYPE(sce_module_imports) {
    uint16_t size; /* Size of this struct, set to 0x34 */
    uint16_t version; /* Set to 0x1 */
    uint16_t flags; /* Set to 0x0 */
    uint16_t num_syms_funcs; /* Number of function imports */
    uint16_t num_syms_vars; /* Number of variable imports */
    uint16_t num_syms_tls_vars; /* Number of TLS variable imports */

    uint32_t reserved1;
    uint32_t library_nid; /* NID of library to import */
    SCE_PTR(const char *)
    library_name; /* Pointer to name of imported library, for debugging */
    uint32_t reserved2;
    SCE_PTR(uint32_t *)
    func_nid_table; /* Pointer to array of function NIDs to import */
    SCE_PTR(const void **)
    func_entry_table; /* Pointer to array of stub functions to fill */
    SCE_PTR(uint32_t *)
    var_nid_table; /* Pointer to array of variable NIDs to import */
    SCE_PTR(const void **)
    var_entry_table; /* Pointer to array of data pointers to write to */
    SCE_PTR(uint32_t *)
    tls_var_nid_table; /* Pointer to array of TLS variable NIDs to import */
    SCE_PTR(const void **)
    tls_var_entry_table; /* Pointer to array of data pointers to write to */
} SCE_TYPE(sce_module_imports);

/* alternative module imports struct with a size of 0x24 */
typedef struct SCE_TYPE(sce_module_imports_short) {
    uint16_t size; /* Size of this struct, set to 0x24 */
    uint16_t version; /* Set to 0x1 */
    uint16_t flags; /* Set to 0x0 */
    uint16_t num_syms_funcs; /* Number of function imports */
    uint16_t num_syms_vars; /* Number of variable imports */
    uint16_t num_syms_tls_vars; /* Number of TLS variable imports */

    uint32_t library_nid; /* NID of library to import */
    SCE_PTR(const char *)
    library_name; /* Pointer to name of imported library, for debugging */
    SCE_PTR(uint32_t *)
    func_nid_table; /* Pointer to array of function NIDs to import */
    SCE_PTR(const void **)
    func_entry_table; /* Pointer to array of stub functions to fill */
    SCE_PTR(uint32_t *)
    var_nid_table; /* Pointer to array of variable NIDs to import */
    SCE_PTR(const void **)
    var_entry_table; /* Pointer to array of data pointers to write to */
} SCE_TYPE(sce_module_imports_short);

struct sce_module_info_raw { // size is 0x5C-bytes
    uint16_t attributes;
    uint16_t version; /* Set to 0x0101 */
    char name[27]; /* Name of the library */
    uint8_t type; /* 0x0 for executable, 0x6 for PRX */
    Ptr<const void>
        gp_value;
    Ptr<sce_module_exports_raw>
        export_top; /* Offset to start of export table */
    Ptr<sce_module_exports_raw>
        export_end; /* Offset to end of export table */
    Ptr<sce_module_imports_raw>
        import_top; /* Offset to start of import table */
    Ptr<sce_module_imports_raw>
        import_end; /* Offset to end of import table */
    uint32_t module_nid; /* NID of this module */
    uint32_t tls_start;
    uint32_t tls_filesz;
    uint32_t tls_memsz;
    Ptr<const void>
        module_start; /* Offset to function to run when library is started, 0 to disable */
    Ptr<const void>
        module_stop; /* Offset to function to run when library is exiting, 0 to disable */
    Ptr<const void>
        exidx_top; /* Offset to start of ARM EXIDX (optional) */
    Ptr<const void>
        exidx_end; /* Offset to end of ARM EXIDX (optional) */
    Ptr<const void>
        extab_top; /* Offset to start of ARM EXTAB (optional) */
    Ptr<const void>
        extab_end; /* Offset to end of ARM EXTAB (optional */
};

/*
 * Note - fw lower than 1.692 should use v5's process_param struct
 */
#define VITA_TOOLCHAIN_PROCESS_PARAM_NEW_FORMAT_VERSION (0x1692000)

typedef struct SCE_TYPE(sce_process_param_v5) {
    uint32_t size; /* 0x30                       */
    uint32_t magic; /* PSP2                       */
    uint32_t version; /* Unknown, but it could be 5 */
    uint32_t fw_version; /* SDK Version                */
    SCE_PTR(const char *)
    main_thread_name; /* Thread name pointer        */
    int32_t main_thread_priority; /* Thread initPriority        */
    uint32_t main_thread_stacksize; /* Thread stacksize           */
    uint32_t main_thread_attribute; /* Thread attribute           */
    SCE_PTR(const char *)
    process_name; /* Process name pointer       */
    uint32_t process_preload_disabled; /* Module load inhibit        */
    uint32_t main_thread_cpu_affinity_mask; /* Unknown                    */
    SCE_PTR(const void *)
    sce_libc_param; /* SceLibc param pointer      */
} SCE_TYPE(sce_process_param_v5);

typedef struct SCE_TYPE(sce_process_param_v6) {
    uint32_t size; /* 0x34                       */
    uint32_t magic; /* PSP2                       */
    uint32_t version; /* Unknown, but it could be 6 */
    uint32_t fw_version; /* SDK Version                */
    SCE_PTR(const char *)
    main_thread_name; /* Thread name pointer        */
    int32_t main_thread_priority; /* Thread initPriority        */
    uint32_t main_thread_stacksize; /* Thread stacksize           */
    uint32_t main_thread_attribute; /* Thread stacksize           */
    SCE_PTR(const char *)
    process_name; /* Process name pointer       */
    uint32_t process_preload_disabled; /* Module load inhibit        */
    uint32_t main_thread_cpu_affinity_mask; /* Unknown                    */
    SCE_PTR(const void *)
    sce_libc_param; /* SceLibc param pointer      */
    uint32_t unk; /* Unknown                    */
} SCE_TYPE(sce_process_param_v6);

typedef struct SCE_TYPE(sce_libc_param) {
    struct {
        uint32_t size; /* 0x34 */
        uint32_t unk_0x4; /* Unknown, set to 1 */
        SCE_PTR(void *)
        malloc_init; /* Initialize malloc heap */
        SCE_PTR(void *)
        malloc_term; /* Terminate malloc heap */
        SCE_PTR(void *)
        malloc; /* malloc replacement */
        SCE_PTR(void *)
        free; /* free replacement */
        SCE_PTR(void *)
        calloc; /* calloc replacement */
        SCE_PTR(void *)
        realloc; /* realloc replacement */
        SCE_PTR(void *)
        memalign; /* memalign replacement */
        SCE_PTR(void *)
        reallocalign; /* reallocalign replacement */
        SCE_PTR(void *)
        malloc_stats; /* malloc_stats replacement */
        SCE_PTR(void *)
        malloc_stats_fast; /* malloc_stats_fast replacement */
        SCE_PTR(void *)
        malloc_usable_size; /* malloc_usable_size replacement */
    } _malloc_replace;

    struct {
        uint32_t size; /* 0x28 */
        uint32_t unk_0x4; /* Unknown, set to 1 */
        SCE_PTR(void *)
        operator_new; /* new operator replacement */
        SCE_PTR(void *)
        operator_new_nothrow; /* new (nothrow) operator replacement */
        SCE_PTR(void *)
        operator_new_arr; /* new[] operator replacement */
        SCE_PTR(void *)
        operator_new_arr_nothrow; /* new[] (nothrow) operator replacement */
        SCE_PTR(void *)
        operator_delete; /* delete operator replacement */
        SCE_PTR(void *)
        operator_delete_nothrow; /* delete (nothrow) operator replacement */
        SCE_PTR(void *)
        operator_delete_arr; /* delete[] operator replacement */
        SCE_PTR(void *)
        operator_delete_arr_nothrow; /* delete[] (nothrow) operator replacement */
    } _new_replace;

    struct {
        uint32_t size; /* 0x18 */
        uint32_t unk_0x4; /* Unknown, set to 1 */
        SCE_PTR(void *)
        malloc_init_for_tls; /* Initialise tls malloc heap */
        SCE_PTR(void *)
        malloc_term_for_tls; /* Terminate tls malloc heap */
        SCE_PTR(void *)
        malloc_for_tls; /* malloc_for_tls replacement */
        SCE_PTR(void *)
        free_for_tls; /* free_for_tls replacement */
    } _malloc_for_tls_replace;

    uint32_t size; /* 0x38 */
    uint32_t unk_0x04; /* Unknown */
    SCE_PTR(uint32_t *)
    heap_size; /* Heap size variable */
    SCE_PTR(uint32_t *)
    default_heap_size; /* Default heap size variable */
    SCE_PTR(uint32_t *)
    heap_extended_alloc; /* Dynamically extend heap size */
    SCE_PTR(uint32_t *)
    heap_delayed_alloc; /* Allocate heap on first call to malloc */
    uint32_t fw_version; /* SDK version */
    uint32_t unk_0x1C; /* Unknown, set to 9 */
    SCE_PTR(const void *)
    malloc_replace; /* malloc replacement functions */
    SCE_PTR(const void *)
    new_replace; /* new replacement functions */
    SCE_PTR(uint32_t *)
    heap_initial_size; /* Dynamically allocated heap initial size */
    SCE_PTR(uint32_t *)
    heap_unit_1mb; /* Change alloc unit size from 64k to 1M */
    SCE_PTR(uint32_t *)
    heap_detect_overrun; /* Detect heap buffer overruns */
    SCE_PTR(const void *)
    malloc_for_tls_replace; /* malloc_for_tls replacement functions */

    uint32_t _default_heap_size; /* Default SceLibc heap size - 0x40000 (256KiB) */
} SCE_TYPE(sce_libc_param);

#undef SCE_TYPE
#undef SCE_PTR

/**
 * @brief      Extended module information
 *
 *             This supplements the output of `sceKernelGetModuleInfo`
 */
struct tai_module_info_t {
    uint32_t size; ///< Structure size, set to sizeof(tai_module_info_t)
    SceUID modid; ///< Module UID
    uint32_t module_nid; ///< Module NID
    char name[27]; ///< Module name
    Ptr<sce_module_exports_raw> exports_start; ///< Pointer to export table in process address space
    Ptr<sce_module_exports_raw> exports_end; ///< Pointer to end of export table
    Ptr<sce_module_imports_raw> imports_start; ///< Pointer to import table in process address space
    Ptr<sce_module_imports_raw> imports_end; ///< Pointer to end of import table
};

/**
 * @brief      Pass hook arguments to kernel
 */
struct tai_hook_args_t {
    uint32_t size;
    Ptr<const char> module;
    uint32_t library_nid;
    uint32_t func_nid;
    Ptr<const void> hook_func;
};

/**
 * @brief      Pass offset arguments to kernel
 */
struct tai_offset_args_t {
    uint32_t size;
    SceUID modid;
    int segidx;
    uint32_t offset;
    int thumb;
    Ptr<const void> source;
    uint32_t source_size;
};

/**
 * @brief      Pass module arguments to kernel
 */
struct tai_module_args_t {
    uint32_t size;
    SceUID pid;
    uint32_t args;
    Ptr<const void> argp;
    int flags;
};

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
struct tai_hook_user {
    Ptr<uint32_t> next;
    Ptr<void> func;
    Ptr<void> old;
};

enum TaiErrorCode : uint32_t {
    TAI_SUCCESS = 0,
    TAI_ERROR_SYSTEM = 0x90010000,
    TAI_ERROR_MEMORY = 0x90010001,
    TAI_ERROR_NOT_FOUND = 0x90010002,
    TAI_ERROR_INVALID_ARGS = 0x90010003,
    TAI_ERROR_INVALID_KERNEL_ADDR = 0x90010004,
    TAI_ERROR_PATCH_EXISTS = 0x90010005,
    TAI_ERROR_HOOK_ERROR = 0x90010006,
    TAI_ERROR_NOT_IMPLEMENTED = 0x90010007,
    TAI_ERROR_USER_MEMORY = 0x90010008,
    TAI_ERROR_NOT_ALLOWED = 0x90010009,
    TAI_ERROR_STUB_NOT_RESOLVED = 0x9001000A,
    TAI_ERROR_INVALID_MODULE = 0x9001000B,
    TAI_ERROR_MODULE_OVERFLOW = 0x9001000C,
    TAI_ERROR_BLOCKING = 0x9001000D
};

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
    // LOG_CONSOLE(taiHookFunctionImportForUser, p_hook, args->module, args->library_nid, args->func_nid, args->hook_func);
    auto name = import_name(args->func_nid);
    LOG_TRACE("try to patch {} (nid:{})", name, log_hex(args->func_nid));
    return UNIMPLEMENTED();
}
EXPORT(SceUID, taiHookFunctionOffsetForUser, tai_hook_ref_t *p_hook, tai_offset_args_t *args) {
    TRACY_FUNC(taiHookFunctionOffsetForUser, p_hook, args);
    return UNIMPLEMENTED();
}

/**
 * @brief      Gets a loaded module by name or NID or both
 *
 *             If `name` is NULL, then only the NID is used to locate the loaded
 *             module. If `name` is not NULL then it will be used to lookup the
 *             loaded module. If NID is not `TAI_IGNORE_MODULE_NID`, then it
 *             will be used in the lookup too. If `name` is NULL and NID is
 *             `TAI_IGNORE_MODULE_NID` then the first module loaded for the
 *             process will be returned.
 *
 * @param[in]  pid      The pid
 * @param[in]  name  The name to lookup. Can be NULL.
 * @param[in]  nid   The nid to lookup. Can be `TAI_IGNORE_MODULE_NID`.
 * @param[out] info  The information
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_MODULE if both `name` and NID are undefined
 *               and `pid` is KERNEL_PID.
 *             - TAI_ERROR_MODULE_OVERFLOW if there are more than
 *               `MOD_LIST_SIZE` modules loaded for the process. This is a
 *               system error and should be reported.
 */
constexpr int MOD_LIST_SIZE = 256;

EXPORT(int, taiGetModuleInfo, const char *module_name, tai_module_info_t *info) {
    TRACY_FUNC(taiGetModuleInfo, module_name, info);
    LOG_CONSOLE(taiGetModuleInfo, module_name, info);
    if (info->size != sizeof(tai_module_info_t)) {
        LOG_ERROR("Structure size too small: %d", info->size);
        return TAI_ERROR_SYSTEM;
    }

    int module_id = 0;
    if (module_name == nullptr) {
        for (auto &[module_id_, module] : emuenv.kernel.loaded_modules) {
            if (module->info.path == "app0:" + emuenv.self_path) {
                module_id = module_id_;
            }
        }
    } else {
        for (auto &[module_id_, module] : emuenv.kernel.loaded_modules) {
            if (strncmp(module->info.module_name, module_name, sizeof(module->info.module_name)) == 0) {
                module_id = module_id_;
            }
        }
    }
    if (module_id == 0) {
        return TAI_ERROR_NOT_FOUND;
    }
    auto &module_info = emuenv.kernel.loaded_modules[module_id];
    const sce_module_info_raw *int_mod_info = reinterpret_cast<const sce_module_info_raw *>(module_info->info_segment_address.get(emuenv.mem) + module_info->info_offset);
    info->modid = module_id;
    info->module_nid = int_mod_info->module_nid;
    info->exports_start = int_mod_info->export_top;
    info->exports_end = int_mod_info->export_end;
    info->imports_start = int_mod_info->import_top;
    info->imports_end = int_mod_info->import_end;
    strncpy(info->name, int_mod_info->name, 27);
    info->name[26] = '\0';
    // LOG_TRACE("taiGetModuleInfo,module:{}",module_name);
    return 0; // UNIMPLEMENTED();
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
