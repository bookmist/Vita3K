// LICENSE

// Original code is taken from taihen https://github.com/yifanlu/taiHEN and deeply modified to fit the needs of the project
#pragma once

#include "kernel/cpu_protocol.h"
#include "kernel/types.h"
// #include "proc_map.h"
#include "slab.h"

#include <stdint.h>
#include <string>

struct _tai_hook_user;
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
    Ptr<void> hook_func;
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
    Ptr<void> source;
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
typedef Ptr<_tai_hook_user> tai_hook_ref_t;

/**
 * @brief      Internal structure
 */
typedef struct _tai_hook_user {
    Ptr<_tai_hook_user> next; // next hook
    Ptr<void> func; // hook function (self)
    Ptr<void> old; // original function
} tai_hook_user_t;

/**
 * @brief      Internal functions and defines
 */

#define LOG LOG_INFO

/** Max size of a function patch */
constexpr int FUNC_SAVE_SIZE = 16;

/** Fake PID indicating memory is shared across all user processes. */
constexpr uint32_t SHARED_PID = 0x80000000;

/** Fallback if the current running fw version cannot be detected. */
constexpr uint32_t DEFAULT_FW_VERSION = 0x3600000;

/**
 * @addtogroup patches
 */
/** @{ */

/**
 * Type of patch
 */
typedef enum {
    HOOKS,
    INJECTION
} tai_patch_type_t;

struct _tai_patch;
/**
 * @brief      Hook data stored in address space of process to patch
 */
typedef struct _tai_hook {
    _tai_hook_user u; ///< Used by `TAI_CONTINUE` to find next hook to run
    // TODO: obfuscate these two kernel pointers as they might be stored in userland
    // also put a MAC over them
    Ptr<_tai_hook> next; ///< Next hook for this process + address
    Ptr<_tai_patch> patch; ///< The patch containing this hook
} tai_hook_t;

/**
 * @brief      Injection data
 */
typedef struct _tai_inject {
    Ptr<void> saved; ///< The original data (allocated on inject)
    SceSize size; ///< Size of original data
    Ptr<_tai_patch> patch; ///< The patch containing this injection
} tai_inject_t;

/**
 * @brief      A chain of hooks
 */
typedef struct _tai_hook_list {
    Ptr<void> func; ///< Address of the function to hook
    Ptr<void> old; ///< A function pointer used to call the original function
    Ptr<void> saved; ///< Data saved by libsubstitute to restore the function
    Ptr<_tai_hook> head; ///< The linked list of hooks on this process + address
} tai_hook_list_t;

/**
 * @brief      A patch containing either a hook chain or an injection
 */
typedef struct _tai_patch {
    // uint32_t sce_reserved[2]; ///< used by SCE object system
    union {
        struct _tai_inject inject; ///< Inject data
        struct _tai_hook_list hooks; ///< Hook chain data
    } data;
    tai_patch_type_t type; ///< Type of patch (hook chain or injection)
    SceUID uid; ///< Kernel object id of this object
    SceUID pid; ///< Process owning this object
    Address addr; ///< Address being patched
    SceSize size; ///< Size of the patch
    Ptr<_tai_patch> next; ///< Next patch in the linked list for this process
    Ptr<slab_chain> slab; ///< Slab chain for this process (copied from the owner `tai_proc_t`)
} tai_patch_t;

/** @} */

/**
 * @addtogroup proc_map
 */
/** @{ */

/**
 * @brief      Internal data for a process
 */
typedef struct _tai_proc {
    SceUID pid; ///< Process ID (the key in the map)
    Ptr<tai_patch_t> head; ///< Linked list of patches for this process
    slab_chain slab; ///< A slab allocator associated with this process
    Ptr<_tai_proc> next; ///< Next process in this map bucket
} tai_proc_t;

/** @} */
// struct _tai_proc_map;

struct taihen_module_data {
    /** Patches pool resource id. Also used in posix-compat.c */
    Address g_patch_pool;

    /** The map of processes to list of patches */
    //_tai_proc_map *g_map;

    /** Lock for handling hooks */
    std::mutex g_hooks_lock{};

    /** UID class for taiHEN */
    // SceClass g_taihen_class;

    // plugin.c -- Config and plugin loading
    /** Buffer for the config data */
    std::string g_config;
    std::vector<std::string> plugins_to_load;

    /** Mutex for accessing g_config */
    std::mutex g_config_lock;

    /** Set for delayed load of config */
    int g_delayed_load_config;

    /** Set for delayed load of kernel plugins */
    int g_delayed_load_kernel_plugins;

    /** Resource pointer for the heap pool */
    SceUID g_map_pool;
};
struct EmuEnvState;
taihen_module_data *get_module_data(EmuEnvState &emuenv);
