// LICENSE

// Original code is taken from taihen https://github.com/yifanlu/taiHEN and deeply modified to fit the needs of the project
#pragma once

#include "kernel/cpu_protocol.h"
#include "kernel/types.h"
#include "proc_map.h"

#include <stdint.h>

/**
 * @defgroup   taihen API Interface
 * @brief      Provides basic helper utilities for plugins that aid in user to
 *             kernel interaction.
 *
 * @details    taiHEN proves three types of exports. First is a patch system for
 *             modifying code and read-only data. Second is basic peek/poke for
 *             the kernel. Third is support for loading kernel modules.
 *
 *             A common question is: when should I use hooks, injections, and
 *             peek/poke? If you wish to patch writable data in the kernel and
 *             you know the address, then `taiMemcpyKernelToUser` works. If you
 *             don't know the address but you know the offset from its ELF
 *             segment then use an injection. If it is read-only data, then use
 *             an injection. Finally, if you wish to patch a _function_ to run
 *             your own code, you should use a hook.
 */
/** @{ */

/** PID for kernel process */
constexpr uint32_t KERNEL_PID = 0x10005;

/** Fake library NID indicating that any library NID would match. */
constexpr uint32_t TAI_ANY_LIBRARY = 0xFFFFFFFF;

/** Fake module NID indicating that any module NID would match. */
constexpr uint32_t TAI_IGNORE_MODULE_NID = 0xFFFFFFFF;

/** Fake module name indicating the current process's main module. */
constexpr char *TAI_MAIN_MODULE = nullptr;

/**
 * @brief      Extended module information
 *
 *             This supplements the output of `sceKernelGetModuleInfo`
 */
typedef struct _tai_module_info {
    SceSize size; ///< Structure size, set to sizeof(tai_module_info_t)
    SceUID modid; ///< Module UID
    uint32_t module_nid; ///< Module NID
    char name[27]; ///< Module name
    Ptr<void> exports_start; ///< Pointer to export table in process address space
    Ptr<void> exports_end; ///< Pointer to end of export table
    Ptr<void> imports_start; ///< Pointer to import table in process address space
    Ptr<void> imports_end; ///< Pointer to end of import table
} tai_module_info_t;

/**
 * @brief      Pass hook arguments to kernel
 */
typedef struct _tai_hook_args {
    SceSize size;
    Ptr<const char> module;
    uint32_t library_nid;
    uint32_t func_nid;
    Ptr<const void> hook_func;
} tai_hook_args_t;

/**
 * @brief      Pass offset arguments to kernel
 */
typedef struct _tai_offset_args {
    SceSize size;
    SceUID modid;
    int segidx;
    uint32_t offset;
    int thumb;
    Ptr<const void> source;
    SceSize source_size;
} tai_offset_args_t;

/**
 * @brief      Pass module arguments to kernel
 */
typedef struct _tai_module_args {
    SceSize size;
    SceUID pid;
    SceSize args;
    Ptr<void> argp;
    int flags;
} tai_module_args_t;

/**
 * @brief      Hook information
 *
 *             This reference is created on new hooks and is up to the caller to
 *             keep track of. The client is responsible for cleanup by passing
 *             the reference back to taiHEN when needed.
 */
typedef Ptr<void> tai_hook_ref_t;

/**
 * @brief      Internal structure
 */
typedef struct _tai_hook_user {
    Ptr<_tai_hook_user> next;
    Ptr<void> func;
    Ptr<void> old;
} tai_hook_user_t;

struct _tai_proc_map;

struct taihen_module_data {
    /** Patches pool resource id. Also used in posix-compat.c */
    SceUID g_patch_pool;

    /** The map of processes to list of patches */
    _tai_proc_map *g_map;

    /** Lock for handling hooks */
    std::mutex g_hooks_lock{};

    /** UID class for taiHEN */
    // SceClass g_taihen_class;
};
struct EmuEnvState;
taihen_module_data *get_module_data(EmuEnvState &emuenv);
