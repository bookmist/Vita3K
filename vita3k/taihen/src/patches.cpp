/* patches.c -- main patch system
 *
 * Copyright (C) 2016 Yifan Lu
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "taihen/patches.h"
#include "kernel/cpu_protocol.h"
#include "kernel/types.h"
// #include "substitute/lib/substitute.h"
#include "kernel/state.h"
#include "taihen/error.h"
#include "taihen/proc_map.h"
#include "taihen/slab.h"
#include "taihen/taihen.h"

#include <emuenv/state.h>
#include <mem/functions.h>
#include <util/log.h>

/**
 * @brief      Hooks are added to a linked list and injections are written
 *             directly.
 *
 *             The original code/data is always stored so it can be restored.
 *             The ordering of hooks is not defined and the developer should
 *             expect that the hooks will execute in any order.
 */

/** Size of the heap pool for storing patches and patch metadata in bytes. */
#define PATCHES_POOL_SIZE 0x10000

/** Number of buckets in proc map. */
#define NUM_PROC_MAP_BUCKETS 16

/** Helper macro to make function pointer to data pointer. */
#define FUNC_TO_UINTPTR_T(x) (((uintptr_t)(x)) & 0xFFFFFFFE)

/** Address range for public (shared) memory. */
#define MEM_SHARED_START ((void *)0xE0000000)

/**
 * @brief      Callback to initialize a patch
 *
 * @param      dat   The patch to initialize
 *
 * @return     Zero
 */
static int init_patch(void *dat) {
    tai_patch_t *patch;

    patch = (tai_patch_t *)dat;
    LOG("init of: %p", patch);
    return 0;
}

/**
 * @brief      Callback to free a patch
 *
 * @param      dat   The patch to free
 *
 * @return     Zero
 */
static int free_patch(void *dat) {
    tai_patch_t *patch;

    patch = (tai_patch_t *)dat;
    LOG("cleanup of: %p", patch);
    return 0;
}

/**
 * @brief      Initializes the patch system
 *
 * Requires `proc_map_init` to be called first! Should be called on startup.
 *
 * @return     Zero on success, < 0 on error
 */
int patches_init(EmuEnvState &emuenv) {
    // SceKernelHeapCreateOpt opt;
    int ret;

    // memset(&opt, 0, sizeof(opt));
    // opt.size = sizeof(opt);
    // opt.uselock = 1;
    auto module_data = get_module_data(emuenv);
    module_data->g_patch_pool = alloc(emuenv.mem, PATCHES_POOL_SIZE, "tai_patches");
    // g_patch_pool = ksceKernelCreateHeap("tai_patches", PATCHES_POOL_SIZE, &opt);
    LOG("ksceKernelCreateHeap(tai_patches): {:X}", module_data->g_patch_pool);
    if (module_data->g_patch_pool < 0) {
        return module_data->g_patch_pool;
    }
    *Ptr<uint32_t>(module_data->g_patch_pool).get(emuenv.mem) = 4; // use easyest high watermark memory manager. first 4 bytes store watermark itself
    module_data->g_map = proc_map_alloc(NUM_PROC_MAP_BUCKETS);
    if (module_data->g_map == nullptr) {
        LOG("Failed to create proc map.");
        return TAI_ERROR_SYSTEM;
    }
    /*
    g_hooks_lock = ksceKernelCreateMutex("tai_hooks_lock", SCE_KERNEL_MUTEX_ATTR_RECURSIVE, 0, NULL);
    LOG("ksceKernelCreateMutex(tai_hooks_lock): 0x%08X", g_hooks_lock);
    if (g_hooks_lock < 0) {
        return g_hooks_lock;
    }*/
    /*
    ret = ksceKernelCreateClass(&g_taihen_class, "taiHENClass", ksceKernelGetUidClass(), sizeof(tai_patch_t), init_patch, free_patch);
    LOG("ksceKernelCreateClass(taiHENClass): 0x%08X", ret);
    if (ret < 0) {
        return ret;
    }*/
    return TAI_SUCCESS;
}

/**
 * @brief      Cleans up the patch system
 *
 * Should be called before exit.
 */
void patches_deinit(EmuEnvState &emuenv) {
    LOG("Cleaning up patches subsystem.");
    // TODO: Find out how to clean up class
    // ksceKernelDeleteMutex(g_hooks_lock);
    // ksceKernelDeleteHeap(g_patch_pool);
    auto module_data = get_module_data(emuenv);
    proc_map_free(module_data->g_map);
    module_data->g_map = nullptr;
    module_data->g_patch_pool = 0;
    // module_data->g_hooks_lock = 0;
    free(emuenv.mem, module_data->g_patch_pool);
}

Address alloc_guest_mem(EmuEnvState &emuenv, SceSize size) {
    auto module_data = get_module_data(emuenv);
    auto allocated_size = Ptr<int>(module_data->g_patch_pool).get(emuenv.mem);
    if (*allocated_size + size > PATCHES_POOL_SIZE) {
        LOG_CRITICAL("PATCHES_POOL overload");
        return 0;
    }
    auto res = module_data->g_patch_pool + *allocated_size;
    *allocated_size += size;
    return res;
}

void free_guest_mem(EmuEnvState &emuenv, Address data) {}

/**
 * @brief      Dump data to log
 *
 * @param[in]  paddr  The paddr
 * @param[in]  addr   The address
 * @param[in]  size   The size to dump
 */
static inline void hex_dump(uintptr_t paddr, const char *addr, unsigned int size) {
    unsigned int i;
    for (i = 0; i < (size >> 4); i++) {
        LOG("0x%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
            paddr,
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], addr[8],
            addr[9], addr[10], addr[11], addr[12], addr[13], addr[14], addr[15]);
        paddr += 0x10;
        addr += 0x10;
    }
}

/**
 * @brief      Flush L1 and L2 cache for an address
 *
 *             For thread safety, interrupts may be disabled for the duration of
 *             this call. That plus the act of cache flushing itself makes this
 *             an expensive operation.
 *
 * @param[in]  pid   The pid
 * @param[in]  vma   The vma
 * @param[in]  len   The length
 */
void cache_flush(EmuEnvState &emuenv, SceUID pid, Address vma, SceSize len) {
    emuenv.kernel.invalidate_jit_cache(vma, len);
}

/**
 * @brief      Used by `do_hooking`
 */
struct hook_args {
    SceUID pid;
    struct substitute_function_hook *hook;
    struct substitute_function_hook_record **saved;
};

/**
 * @brief      Adds a hook to a function using libsubstitute
 *
 * @param[in]  slab         The slab to allocate exec memory from
 * @param[in]  target_func  The function to hook
 * @param[in]  src_func     The hook function
 * @param[out] old          A pointer to call the original implementation
 * @param[out] saved        Saved data for freeing the hook
 *
 * @return     Zero on success, < 0 on error
 */
static int tai_hook_function(Ptr<slab_chain> slab, Ptr<void> target_func, Ptr<const void> src_func, Ptr<void> *old, Ptr<void> *saved) {
    if (target_func.address() == src_func.address()) {
        LOG("no hook needed");
        return TAI_SUCCESS; // no need for hook
    }

    /*
    struct hook_args uargs;
    struct substitute_function_hook hook;
    int ret;

    hook.function = target_func;
    hook.replacement = (void *)src_func;
    hook.old_ptr = old;
    hook.options = 0;
    hook.opt = slab;
    LOG("Calling substitute_hook_functions");
    // TODO: Take care of SHARED_PID
    uargs.pid = slab->pid;
    uargs.hook = &hook;
    uargs.saved = (struct substitute_function_hook_record **)saved;
    //ret = substitute_hook_functions(uargs->hook, 1, uargs->saved, SUBSTITUTE_RELAXED);
    LOG("Done hooking");
    if (ret != SUBSTITUTE_OK) {
        LOG("libsubstitute error: %s", substitute_strerror(ret));
        return TAI_ERROR_HOOK_ERROR;
    }
    return TAI_SUCCESS;
    */
}

/**
 * @brief      Removes a hook using libsubstitute
 *
 * @param[in]  saved  The saved data from `tai_hook_function`
 *
 * @return     Zero on success, < 0 on error
 */
static int tai_unhook_function(Ptr<void> saved) {
    /*
    int ret;
    LOG("Calling substitute_free_hooks");
    //ret = ksceKernelRunWithStack(0x4000, do_unhooking, saved);
    //ret = substitute_free_hooks((struct substitute_function_hook_record *)saved, 1);
    if (ret != SUBSTITUTE_OK) {
        LOG("libsubstitute error: %s", substitute_strerror(ret));
        return TAI_ERROR_HOOK_ERROR;
    }
    return TAI_SUCCESS;
    */
}

/**
 * @brief      Memcpy within process without the pesky permissions
 *
 *             This function will write raw data from `src` to `dst` for `size`.
 *             It works even if `dst` is read only. All levels of caches will be
 *             flushed.
 *
 * @param[in]  dst_pid  The target process
 * @param      dst      The target address
 * @param[in]  src      The source kernel address
 * @param[in]  size     The size
 *
 * @return     Zero on success, < 0 on error
 */
static int tai_force_memcpy(EmuEnvState emuenv, SceUID dst_pid, Ptr<void> dst, Ptr<const void> src, SceSize size) {
    memcpy(dst.get(emuenv.mem), src.get(emuenv.mem), size);
    /*
    int ret;
    if (dst_pid == KERNEL_PID) {
        ret = ksceKernelCpuUnrestrictedMemcpy(dst, src, size);
        LOG("ksceKernelCpuUnrestrictedMemcpy(%p, %p, 0x%08X): 0x%08X", dst, src, size, ret);
    } else {
        ret = ksceKernelRxMemcpyKernelToUserForPid(dst_pid, (uintptr_t)dst, src, size);
        LOG("ksceKernelRxMemcpyKernelToUserForPid(%x, %p, %p, 0x%08X): 0x%08X", dst_pid, dst, src, size, ret);
    }*/
    cache_flush(emuenv, dst_pid, dst.address(), size);
    return 0;
}

/**
 * @brief      Memcpy from a process to kernel
 *
 * @param[in]  src_pid  The source process (can be kernel)
 * @param      dst      The target address
 * @param[in]  src      The source
 * @param[in]  size     The size
 *
 * @return     Zero on success, < 0 on error
 */
int tai_memcpy_to_kernel(SceUID src_pid, void *dst, const char *src, size_t size) {
    memcpy(dst, src, size);
    /*
    int ret;
    if (src_pid == KERNEL_PID) {
        memcpy(dst, src, size);
        LOG("memcpy(%p, %p, 0x%08X)", dst, src, size);
    } else {
        ret = ksceKernelMemcpyUserToKernelForPid(src_pid, dst, (uintptr_t)src, size);
        LOG("ksceKernelMemcpyUserToKernelForPid(%x, %p, %p, 0x%08X): 0x%08X", src_pid, dst, src, size, ret);
    }*/
    return 0;
}

/**
 * @brief      Adds a hook to a chain, patching the original function if needed
 *
 *             If this is the first hook in a chain, the original function will
 *             be patched. Otherwise, it will be placed into the chain. The
 *             order in the chain is not defined.
 *
 * @param      hooks  The chain of hooks to add to
 * @param      item   The hook to add
 *
 * @return     Zero if new hook added, 1 if it exists, < 0 on error
 */
static int hooks_add_hook(EmuEnvState emuenv, tai_hook_list_t *hooks, Ptr<tai_hook_t> item) {
    tai_hook_t *head;
    int ret;

    LOG("Adding hook %p to chain %p", item, hooks);
    auto module_data = get_module_data(emuenv);
    {
        std::lock_guard lock(module_data->g_hooks_lock);
        if (hooks->head.address() == 0) { // first hook for this list
            ret = tai_hook_function(item.get(emuenv.mem)->patch.get(emuenv.mem)->slab, hooks->func, item.get(emuenv.mem)->u.func, &hooks->old, &hooks->saved);
            if (ret >= 0) {
                hooks->head = item;
                item.get(emuenv.mem)->next = nullptr;
                item.get(emuenv.mem)->u.next = nullptr;
                item.get(emuenv.mem)->u.old = hooks->old;
                cache_flush(item.get(emuenv.mem)->patch.get(emuenv.mem)->pid, slab_getmirror(item.get(emuenv.mem)->patch.get(emuenv.mem)->slab.address(), item), sizeof(tai_hook_t));
            } else {
                LOG("Hook failed, do not add to chain");
            }
        } else {
            head = hooks->head.get(emuenv.mem);
            item.get(emuenv.mem)->next = head->next;
            item.get(emuenv.mem)->u.next = head->u.next;
            item.get(emuenv.mem)->u.old = hooks->old;
            head->next = item;
            head->u.next = slab_getmirror(item.get(emuenv.mem)->patch.get(emuenv.mem)->slab, item);
            LOG("Added hook to existing chain %p", head);
            // flush cache for head + item, which were modified
            cache_flush(item->patch->pid, slab_getmirror(item->patch->slab, head), sizeof(tai_hook_t));
            cache_flush(item->patch->pid, head->u.next, sizeof(tai_hook_t));
            ret = 1;
        }
    }

    return ret;
}

/**
 * @brief      Removes a hook from a chain, patching the original function if
 *             needed
 *
 *             If the hook to remove is the first hook in a chain, the patched
 *             function will be restored to its original state. If there is
 *             another hook in the chain, the function will be patched again to
 *             jump to that hook.
 *
 * @param      hooks  The chain of hooks to remove from
 * @param      item   The hook to remove
 *
 * @return     Zero on success, < 0 on error or if item is not found
 */
static int hooks_remove_hook(EmuEnvState &emuenv, tai_hook_list_t *hooks, Ptr<tai_hook_t> item) {
    Ptr<tai_hook_t> *cur;
    Address tmp;
    int ret;

    LOG("Removing hook %p for %p", item, hooks);
    auto module_data = get_module_data(emuenv);
    {
        std::lock_guard lock(module_data->g_hooks_lock);
        if (hooks->head == item) { // first hook for this list
            // we must remove the patch
            tai_unhook_function(hooks->saved);
            hooks->saved = nullptr;
            // set head to the next item
            hooks->head = item.get(emuenv.mem)->next;
            if (hooks->head.address() != 0) {
                // add a patch to the new head
                ret = tai_hook_function(item.get(emuenv.mem)->patch.get(emuenv.mem)->slab, hooks->func, hooks->head.get(emuenv.mem)->u.func, &hooks->old, &hooks->saved);
                // update the old pointers
                for (cur = &hooks->head; *cur->address() != 0; cur = &cur->get(emuenv.mem)->next) {
                    (*cur).get(emuenv.mem)->u.old = hooks->old;
                }
                // clear cache of mirror for the last item since it uses the old pointer
                cache_flush(emuenv, item.get(emuenv.mem)->patch.get(emuenv.mem)->pid, cur->address() - offsetof(tai_hook_t, next), sizeof(tai_hook_t));
            } else {
                ret = 0;
            }
        } else {
            cur = &hooks->head;
            ret = -1;
            while (1) {
                if (*cur) {
                    if (*cur == item) {
                        *cur = item.get(emuenv.mem)->next; // remove from list
                        //*cur_user = item.get(emuenv.mem)->u.next;
                        // clear cache since pointers were changed
                        cache_flush(emuenv, item.get(emuenv.mem)->patch.get(emuenv.mem)->pid, cur->address() - offsetof(tai_hook_t, next), sizeof(tai_hook_t));
                        ret = 0;
                        break;
                    } else {
                        cur = &(*cur).get(emuenv.mem)->next;
                        // cur_user = &(*cur).get(emuenv.mem)->u.next.address();
                    }
                } else {
                    break;
                }
            }
        }
    }
    return ret;
}

/**
 * @brief      Inserts a hook given an absolute address and PID of the function
 *
 * @param[out] p_hook     Outputs a reference object if successful
 * @param[in]  pid        PID of the address space to hook
 * @param      dest_func  The destination function
 * @param[in]  hook_func  The hook function
 *
 * @return     UID for the hook on success, < 0 on error
 */
SceUID tai_hook_func_abs(EmuEnvState &emuenv, tai_hook_ref_t *p_hook, SceUID pid, Ptr<void> dest_func, Ptr<const void> hook_func) {
    SceCreateUidObjOpt opt;
    tai_patch_t *patch, *tmp;
    tai_hook_t *hook;
    int ret;
    struct slab_chain *slab;
    uintptr_t exe_addr;

    LOG("Hooking %p to %p for pid %x", hook_func, dest_func, pid);
    if (hook_func >= MEM_SHARED_START) {
        if (pid == KERNEL_PID) {
            return TAI_ERROR_INVALID_KERNEL_ADDR; // invalid hook address
        } else {
            return TAI_ERROR_NOT_IMPLEMENTED; // TODO: add support for this
        }
    }

    hook = NULL;
    if (pid == KERNEL_PID) {
        ret = ksceKernelCreateUidObj(&g_taihen_class, "tai_patch_hook", NULL, (SceObjectBase **)&patch);
    } else {
        memset(&opt, 0, sizeof(opt));
        opt.flags = 8;
        opt.pid = pid;
        ret = ksceKernelCreateUidObj(&g_taihen_class, "tai_patch_hook_user", &opt, (SceObjectBase **)&patch);
    }
    LOG("ksceKernelCreateUidObj(tai_patch_hook): 0x%08X, %p", ret, patch);
    if (ret < 0) {
        return ret;
    }

    ksceKernelLockMutex(g_hooks_lock, 1, NULL);
    patch->type = HOOKS;
    patch->uid = ret;
    patch->pid = pid;
    patch->addr = dest_func.address();
    patch->size = FUNC_SAVE_SIZE;
    patch->next = nullptr;
    patch->data.hooks.func = dest_func;
    patch->data.hooks.saved = nullptr;
    patch->data.hooks.head = nullptr;
    if (proc_map_try_insert(g_map, patch, &tmp) < 1) {
        // ret = ksceKernelDeleteUid(patch->uid);
        // LOG("ksceKernelDeleteUid(old): 0x%08X", ret);
        if (tmp == NULL || tmp->type != HOOKS) {
            // error
            LOG("this hook overlaps an existing hook");
            ret = TAI_ERROR_PATCH_EXISTS;
            goto err;
        } else {
            // we have an existing patch
            LOG("found existing patch %p, discarding %p", tmp, patch);
            patch = tmp;
        }
    }

    hook = slab_alloc(patch->slab, &exe_addr);
    if (hook == NULL) {
        ret = -1;
        goto err;
    }
    hook->u.func = (void *)hook_func;
    hook->patch = patch;

    ret = hooks_add_hook(emuenv, &patch->data.hooks, hook);
    if (ret < 0 && patch->data.hooks.head == NULL) {
        LOG("failed to add hook and patch is now empty, freeing hook %p", hook);
        slab_free(patch->slab, hook);
        hook = NULL;
        proc_map_remove(g_map, patch);
        ksceKernelDeleteUid(patch->uid);
        patch = NULL;
    } else if (ret >= 0) {
        ret = patch->uid;
        *p_hook = slab_getmirror(patch->slab, hook);
    }

err:
    // error and we have allocated a hook
    if (ret < 0 && patch && hook) {
        LOG("freeing hook %p", hook);
        slab_free(patch->slab, hook);
    }

    ksceKernelUnlockMutex(g_hooks_lock, 1);

    return ret;
}

/**
 * @brief      Removes a hook and restores original function if chain is empty
 *
 * @param[in]  uid       The uid reference
 * @param[in]  hook_ref  The hook
 *
 * @return     Zero on success, < 0 on error
 */
int tai_hook_release(SceUID uid, tai_hook_ref_t hook_ref) {
    tai_hook_t **cur, *hook;
    tai_patch_t *patch;
    struct slab_chain *slab;
    int ret;

    ret = ksceKernelGetObjForUid(uid, &g_taihen_class, (SceObjectBase **)&patch);
    LOG("ksceKernelGetObjForUid(%x): 0x%08X", uid, ret);
    if (ret < 0) {
        return ret;
    }
    ksceKernelLockMutex(g_hooks_lock, 1, NULL);
    slab = patch->slab;
    for (cur = &patch->data.hooks.head; *cur != NULL; cur = &(*cur)->next) {
        if (slab_getmirror(slab, *cur) == hook_ref) {
            hook = *cur;
            LOG("Found hook %p for ref %p", hook, hook_ref);
            ret = hooks_remove_hook(&patch->data.hooks, hook);
            *cur = hook->next;
            LOG("freeing hook");
            slab_free(slab, hook);
            if (patch->data.hooks.head == NULL) {
                LOG("patch is now empty, freeing it");
                proc_map_remove(g_map, patch);
                ksceKernelUidRelease(uid);
                ksceKernelDeleteUid(patch->uid);
            }
            ret = TAI_SUCCESS;
            goto end;
        }
    }
    LOG("Cannot find hook for uid %x ref %p", uid, hook_ref);
    ret = TAI_ERROR_NOT_FOUND;
    ksceKernelUidRelease(uid);
end:
    ksceKernelUnlockMutex(g_hooks_lock, 1);

    return ret;
}

/**
 * @brief      Inserts a raw data injection given an absolute address and PID of
 *             the address space
 *
 * @param[in]  pid   The pid of the src and dest pointers address space
 * @param      dest  The destination
 * @param[in]  src   The source
 * @param[in]  size  The size
 *
 * @return     UID for the injection on success, < 0 on error
 *             - TAI_ERROR_PATCH_EXISTS if a hook or injection is already
 *               inserted
 */
SceUID tai_inject_abs(EmuEnvState &emuenv, SceUID pid, void *dest, const void *src, size_t size) {
    tai_patch_t *patch, *tmp;
    void *saved;
    int ret;

    // TODO: Check that dest is not inside our slab structure... that could corrupt kernel code

    LOG("Injecting %p with %p for size 0x%08X at pid %x", dest, src, size, pid);
    ret = ksceKernelCreateUidObj(&g_taihen_class, "tai_patch_inject", NULL, (SceObjectBase **)&patch);
    LOG("ksceKernelCreateUidObj(tai_patch_inject): 0x%08X, %p", ret, patch);
    if (ret < 0) {
        return ret;
    }

    saved = ksceKernelAllocHeapMemory(g_patch_pool, size);
    LOG("ksceKernelAllocHeapMemory(g_patch_pool, 0x%08X): %p", size, saved);
    if (saved == NULL) {
        ksceKernelDeleteUid(ret);
        return TAI_ERROR_MEMORY;
    }

    // try to save old data
    if (tai_memcpy_to_kernel(pid, saved, dest, size) < 0) {
        LOG("Invalid address for memcpy");
        ksceKernelDeleteUid(ret);
        ksceKernelFreeHeapMemory(g_patch_pool, saved);
        return TAI_ERROR_INVALID_ARGS;
    }

    ksceKernelLockMutex(g_hooks_lock, 1, NULL);
    patch->type = INJECTION;
    patch->uid = ret;
    patch->pid = pid;
    patch->addr = (uintptr_t)dest;
    patch->size = size;
    patch->next = NULL;
    patch->data.inject.saved = saved;
    patch->data.inject.size = size;
    patch->data.inject.patch = patch;
    if (proc_map_try_insert(g_map, patch, &tmp) < 1) {
        ret = TAI_ERROR_PATCH_EXISTS;
    } else {
        ret = tai_force_memcpy(pid, dest, src, size);
    }

    if (ret < 0) {
        ksceKernelDeleteUid(patch->uid);
        ksceKernelFreeHeapMemory(g_patch_pool, saved);
    } else {
        ret = patch->uid;
    }

    ksceKernelUnlockMutex(g_hooks_lock, 1);

    return ret;
}

/**
 * @brief      Removes an injection and restores the original data
 *
 * @param      inject  The injection
 *
 * @return     Zero on success, < 0 on error
 */
int tai_inject_release(EmuEnvState &emuenv, SceUID uid) {
    tai_inject_t *inject;
    tai_patch_t *patch;
    void *saved;
    void *dest;
    size_t size;
    int ret;
    SceUID pid;

    ret = ksceKernelGetObjForUid(uid, &g_taihen_class, (SceObjectBase **)&patch);
    LOG("ksceKernelGetObjForUid(%x): 0x%08X", uid, ret);
    if (ret < 0) {
        return ret;
    }
    if (patch->type != INJECTION || patch->uid != uid) {
        LOG("internal error: trying to free an invalid injection");
        ksceKernelUidRelease(uid);
        return TAI_ERROR_SYSTEM;
    }
    inject = &patch->data.inject;
    LOG("Releasing injection %p for patch %p", inject, patch);
    ksceKernelLockMutex(g_hooks_lock, 1, NULL);
    pid = patch->pid;
    dest = (void *)patch->addr;
    saved = inject->saved;
    size = inject->size;
    if (!proc_map_remove(g_map, patch)) {
        LOG("internal error, cannot remove patch from proc_map");
        ret = TAI_ERROR_SYSTEM;
        ksceKernelUidRelease(patch->uid);
    } else {
        ret = tai_force_memcpy(pid, dest, saved, size);
        ksceKernelFreeHeapMemory(g_patch_pool, saved);
        ksceKernelUidRelease(patch->uid);
        ksceKernelDeleteUid(patch->uid);
    }
    ksceKernelUnlockMutex(g_hooks_lock, 1);

    return ret;
}

/**
 * @brief      Called on process exist to force remove private hooks
 *
 *             It is the caller's responsibilty to clean up before it
 *             terminates! However in the case where that doesn't happen, we try
 *             to salvage the situation by manually freeing all patches for a
 *             PID. This is a dirty free that does not attempt to write back the
 *             original data, so it should only be used at process termination.
 *             THIS NOT NOTE FREE PUBLIC HOOKS! There is no free way of keeping
 *             track of which PIDs have handles to a public hook internally, so
 *             we assume that public hooks stay resident forever unless the
 *             release call is made by the caller.
 *
 * @param[in]  pid   The pid
 *
 * @return     Zero always
 */
int tai_try_cleanup_process(SceUID pid) {
    tai_patch_t *patch, *next;
    LOG("Calling patches cleanup for pid %x", pid);
    ksceKernelLockMutex(g_hooks_lock, 1, NULL);
    if (proc_map_remove_all_pid(g_map, pid, &patch) > 0) {
        while (patch != NULL) {
            next = patch->next;
            if (patch->type == INJECTION) {
                LOG("freeing injection saved data");
                ksceKernelFreeHeapMemory(g_patch_pool, patch->data.inject.saved);
            } else if (patch->type == HOOKS) {
                LOG("freeing hook saved data");
                free(patch->data.hooks.saved);
            }
            LOG("deleting patch: %x", patch->uid);
            ksceKernelDeleteUid(patch->uid);
            patch = next;
        }
    }
    ksceKernelUnlockMutex(g_hooks_lock, 1);
    return 0;
}
