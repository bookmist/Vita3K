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

/** PID for kernel process */
#define KERNEL_PID 0x10005

/** Fake library NID indicating that any library NID would match. */
#define TAI_ANY_LIBRARY 0xFFFFFFFF

/** Fake module NID indicating that any module NID would match. */
#define TAI_IGNORE_MODULE_NID 0xFFFFFFFF

/** Fake module name indicating the current process's main module. */
#define TAI_MAIN_MODULE ((void *)0)

/** Functions for calling the syscalls with arguments */
#define HELPER inline static __attribute__((unused))

/**
 * @brief      Extended module information
 *
 *             This supplements the output of `sceKernelGetModuleInfo`
 */
typedef struct _tai_module_info {
  uint32_t size;                ///< Structure size, set to sizeof(tai_module_info_t)
  SceUID modid;               ///< Module UID
  uint32_t module_nid;        ///< Module NID
  char name[27];              ///< Module name
  Ptr<int> exports_start;    ///< Pointer to export table in process address space
  Ptr<int> exports_end;      ///< Pointer to end of export table
  Ptr<int> imports_start;    ///< Pointer to import table in process address space
  Ptr<int> imports_end;      ///< Pointer to end of import table
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
  uint32_t  size;
  SceUID modid;
  int segidx;
  uint32_t offset;
  int thumb;
  Ptr<const void>source;
  uint32_t source_size;
} tai_offset_args_t;

/**
 * @brief      Pass module arguments to kernel
 */
typedef struct _tai_module_args {
  uint32_t size;
  SceUID pid;
  uint32_t args;
  Ptr<const void>argp;
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

/** @name Kernel Hooks
 * Hooks exports to kernel
 */
/** @{ */
EXPORT(SceUID,taiHookFunctionAbs, SceUID pid, tai_hook_ref_t *p_hook, void *dest_func, const void *hook_func) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiHookFunctionExportForKernel, SceUID pid, tai_hook_ref_t *p_hook, const char *module, uint32_t library_nid, uint32_t func_nid, const void *hook_func) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiHookFunctionImportForKernel, SceUID pid, tai_hook_ref_t *p_hook, const char *module, uint32_t import_library_nid, uint32_t import_func_nid, const void *hook_func) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiHookFunctionOffsetForKernel, SceUID pid, tai_hook_ref_t *p_hook, SceUID modid, int segidx, uint32_t offset, int thumb, const void *hook_func) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiGetModuleInfoForKernel, SceUID pid, const char *module, tai_module_info_t *info) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiHookReleaseForKernel, SceUID tai_uid, tai_hook_ref_t hook) {
    return UNIMPLEMENTED();
}
/** @} */

/** 
 * @name User Hooks
 * Hooks exports to user 
 */
/** @{ */
EXPORT(SceUID,taiHookFunctionExportForUser, tai_hook_ref_t *p_hook, tai_hook_args_t *args) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiHookFunctionImportForUser, tai_hook_ref_t *p_hook, tai_hook_args_t *args) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiHookFunctionOffsetForUser, tai_hook_ref_t *p_hook, tai_offset_args_t *args) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiGetModuleInfo, const char *module, tai_module_info_t *info) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiHookRelease, SceUID tai_uid, tai_hook_ref_t hook) {
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
EXPORT(SceUID,taiInjectAbsForKernel, SceUID pid, void *dest, const void *src, size_t size) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiInjectDataForKernel, SceUID pid, SceUID modid, int segidx, uint32_t offset, const void *data, size_t size) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiInjectReleaseForKernel, SceUID tai_uid) {
    return UNIMPLEMENTED();
}
/** @} */

/** 
 * @name User Injections
 * Injection exports to user 
 */
/** @{ */
EXPORT(SceUID,taiInjectAbs, void *dest, const void *src, size_t size) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiInjectDataForUser, tai_offset_args_t *args) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiInjectRelease, SceUID tai_uid) {
    return UNIMPLEMENTED();
}

/** @} */

/** @} */

/**
 * @name Plugin loading
 * Kernel plugin loading plugins manually
 */
/** @{ */
EXPORT(int,taiLoadPluginsForTitleForKernel, SceUID pid, const char *titleid, int flags) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiReloadConfigForKernel, int schedule, int load_kernel) {
    return UNIMPLEMENTED();
}
/** @} */

/**
 * @name Skprx Load
 * Kernel module loading exports to user
 */
/** @{ */

EXPORT(SceUID,taiLoadKernelModule, const char *path, int flags, void *opt) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiStartKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiLoadStartKernelModuleForUser, const char *path, tai_module_args_t *args) {
    return UNIMPLEMENTED();
}
EXPORT(SceUID,taiLoadStartModuleForPidForUser, const char *path, tai_module_args_t *args) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiStopKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiUnloadKernelModule, SceUID modid, int flags, void *opt) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiStopUnloadKernelModuleForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiStopModuleForPidForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiUnloadModuleForPid, SceUID pid, SceUID modid, int flags, void *opt) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiStopUnloadModuleForPidForUser, SceUID modid, tai_module_args_t *args, void *opt, int *res) {
    return UNIMPLEMENTED();
}


/** @} */

/**
 * @name NID Lookup
 * Function NID Lookup Interface
 */
/** @{ */

EXPORT(int,taiGetModuleExportFunc, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func) {
    return UNIMPLEMENTED();
}

/** @} */

/**
 * @name Peek/Poke
 * Read/write kernel memory (no MMU bypass)
 */
/** @{ */

EXPORT(int,taiMemcpyUserToKernel, void *kernel_dst, const void *user_src, size_t len) {
    return UNIMPLEMENTED();
}
EXPORT(int,taiMemcpyKernelToUser, void *user_dst, const void *kernel_src, size_t len) {
    return UNIMPLEMENTED();
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
EXPORT(int,taiReloadConfig) {
    return UNIMPLEMENTED();
}


/** @} */

/**
 * @defgroup   module NID Lookup Interface
 */
/** @{ */

EXPORT(int,module_get_by_name_nid, SceUID pid, const char *name, uint32_t nid, tai_module_info_t *info) {
    return UNIMPLEMENTED();
}
EXPORT(int,module_get_offset, SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr) {
    return UNIMPLEMENTED();
}
EXPORT(int,module_get_export_func, SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func) {
    return UNIMPLEMENTED();
}
EXPORT(int,module_get_import_func, SceUID pid, const char *modname, uint32_t target_libnid, uint32_t funcnid, uintptr_t *stub) {
    return UNIMPLEMENTED();
}

/** @} */
