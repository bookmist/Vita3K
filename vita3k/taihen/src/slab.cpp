/* ref: https://github.com/bbu/userland-slab-allocator */

#include "taihen/slab.h"
#include "kernel/cpu_protocol.h"
#include "taihen/taihen.h"

#include <mem/functions.h>

// #include <psp2kern/kernel/sysmem.h>

#include <cstddef>
#include <cstdint>
#include <util/log.h>

// #define assert(x) // turn off asserts

#define SLAB_DUMP_COLOURED

#ifdef SLAB_DUMP_COLOURED
#define GRAY(s) "\033[1;30m" s "\033[0m"
#define RED(s) "\033[0;31m" s "\033[0m"
#define GREEN(s) "\033[0;32m" s "\033[0m"
#define YELLOW(s) "\033[1;33m" s "\033[0m"
#else
#define GRAY(s) s
#define RED(s) s
#define GREEN(s) s
#define YELLOW(s) s
#endif

#define SLOTS_ALL_ZERO ((uint64_t)0)
#define SLOTS_FIRST ((uint64_t)1)
#define FIRST_FREE_SLOT(s) ((SceSize)std::countr_zero(s))
#define FREE_SLOTS(s) ((SceSize)std::popcount(s))
#define ONE_USED_SLOT(slots, empty_slotmask)                                     \
    (                                                                            \
        (                                                                        \
            (~(slots) & (empty_slotmask)) & ((~(slots) & (empty_slotmask)) - 1)) \
        == SLOTS_ALL_ZERO)

#define POWEROF2(x) ((x) != 0 && ((x) & ((x)-1)) == 0)

// #define LIKELY(exp) __builtin_expect(exp, 1)
// #define UNLIKELY(exp) __builtin_expect(exp, 0)

#define LIKELY(exp) (exp)
#define UNLIKELY(exp) (exp)

const SceSize slab_pagesize = 0x1000;

/**
 * @brief      Allocates a raw chunk of memory
 *
 * Returns a pointer that's kernel writable and another one that's executable.
 *
 * @param[in]  pid       PID to allocate memory for
 * @param      ptr       A kernel writable pointer
 * @param      exe_addr  Executable in the address spaces of PID process
 * @param      exe_res   UID for the executable mapping
 * @param[in]  align     Alignment
 * @param[in]  size      Size
 *
 * @return     UID of writable memory on success, < 0 on error
 */
static SceUID sce_exe_alloc(MemState &mem, SceUID pid, Ptr<void> *ptr, Address *exe_addr, SceUID *exe_res, SceSize align, SceSize size) {
    //    SceKernelAllocMemBlockKernelOpt opt;
    //    SceKernelMemBlockType type;
    //    SceUID res, blkid;
    Address res;
    LOG("Allocating exec slab for %x size 0x%08X", pid, size);
    // allocate exe mem
    /*
    memset(&opt, 0, sizeof(opt));
    opt.size = sizeof(opt);
    opt.attr = 0xA0000000 | 0x400000;
    opt.alignment = align;
    if (align) {
        opt.attr |= SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_ALIGNMENT;
    }
    if (pid == KERNEL_PID) {
        type = SCE_KERNEL_MEMBLOCK_TYPE_KERNEL_RX;
    } else if (pid == SHARED_PID) {
        type = SCE_KERNEL_MEMBLOCK_TYPE_SHARED_RX;
    } else {
        type = SCE_KERNEL_MEMBLOCK_TYPE_USER_RX;
        opt.attr |= 0x80080;
        opt.pid = pid;
    }*/
    if (align) {
        *exe_addr = alloc_aligned(mem, size, "taislab", align);
    } else {
        *exe_addr = alloc(mem, size, "taislab");
    }
    *exe_res = *exe_addr;
    /*
    *exe_res = ksceKernelAllocMemBlock("taislab", type, size, &opt);
    LOG("ksceKernelAllocMemBlock(taislab): 0x%08X", *exe_res);
    if (*exe_res < 0) {
        return *exe_res;
    }
    res = ksceKernelGetMemBlockBase(*exe_res, (void **)exe_addr);
    LOG("ksceKernelGetMemBlockBase(%x): 0x%08X, addr: 0x%08X", *exe_res, res, *exe_addr);
    if (res < 0) {
        goto err2;
    }

    // TODO: Perhaps move this to execmem seal?
    if (pid != KERNEL_PID) {
        res = ksceKernelMapBlockUserVisible(*exe_res);
        LOG("ksceKernelMapBlockUserVisible: %x", res);
        if (res < 0) {
            goto err2;
        }
    }

    // map in every process if needed
    if (pid == SHARED_PID) {
        // FIXME: implement this
    }

    // allocate mirror
    memset(&opt, 0, sizeof(opt));
    opt.size = sizeof(opt);
    opt.attr = 0x1000040;
    opt.mirror_blockid = *exe_res;
    res = ksceKernelAllocMemBlock("taimirror", SCE_KERNEL_MEMBLOCK_TYPE_RW_UNK0, 0, &opt);
    LOG("ksceKernelAllocMemBlock(taimirror): 0x%08X", res);
    if (res < 0) {
        goto err2;
    }
    blkid = res;
    res = ksceKernelGetMemBlockBase(blkid, ptr);
    LOG("ksceKernelGetMemBlockBase(%x): 0x%08X, addr: 0x%08X", blkid, res, *ptr);
    if (res < 0) {
        goto err1;
    }

    return blkid;

err1:
    ksceKernelFreeMemBlock(blkid);
err2:
    ksceKernelFreeMemBlock(*exe_res);*/
    if (align) {
        res = alloc_aligned(mem, size, "taislab", align);
    } else {
        res = alloc(mem, size, "taislab");
    }
    *ptr = Ptr<void>(res);

    return res;
}

/**
 * @brief      Free chunk of memory
 *
 * @param[in]  write_res  The writable UID
 * @param[in]  exe_res    The executable UID
 *
 * @return     Zero
 */
static int sce_exe_free(MemState &mem, SceUID write_res, SceUID exe_res) {
    LOG("freeing slab %x, mirror %x", exe_res, write_res);
    free(mem, write_res);
    free(mem, exe_res);
    return 0;
}

/**
 * @brief      Compute the next largest power of two. Limit 32 bits.
 *
 * @param[in]  v     Input number
 *
 * @return     Next power of 2.
 */
static inline uint32_t next_pow_2(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    v += (v == 0);
    return v;
}

void slab_init(MemState &mem, const Ptr<slab_chain> sch_p, const SceSize itemsize, SceUID pid) {
    assert(sch != nullptr);
    assert(itemsize >= 1 && itemsize <= SIZE_MAX);
    assert(POWEROF2(slab_pagesize));
    auto sch = sch_p.get(mem);
    sch->itemsize = itemsize;
    sch->pid = pid;

    const SceSize data_offset = offsetof(struct slab_header, data);
    const SceSize least_slabsize = data_offset + 64 * sch->itemsize;
    sch->slabsize = (SceSize)next_pow_2(least_slabsize);
    sch->itemcount = 64;

    if (sch->slabsize - least_slabsize != 0) {
        const SceSize shrinked_slabsize = sch->slabsize >> 1;

        if (data_offset < shrinked_slabsize && shrinked_slabsize - data_offset >= 2 * sch->itemsize) {
            sch->slabsize = shrinked_slabsize;
            sch->itemcount = (shrinked_slabsize - data_offset) / sch->itemsize;
        }
    }

    sch->pages_per_alloc = sch->slabsize > slab_pagesize ? sch->slabsize : slab_pagesize;

    sch->empty_slotmask = ~SLOTS_ALL_ZERO >> (64 - sch->itemcount);
    sch->initial_slotmask = sch->empty_slotmask ^ SLOTS_FIRST;
    sch->alignment_mask = ~(sch->slabsize - 1);
    sch->partial = sch->empty = sch->full = nullptr;

    assert(slab_is_valid(sch));
}

Ptr<void> slab_alloc(MemState &mem, const Ptr<slab_chain> sch_p, Address *exe_addr) {
    assert(sch != nullptr);
    assert(slab_is_valid(sch));

    auto sch = sch_p.get(mem);

    if (LIKELY(sch->partial.address() != 0)) {
        /* found a partial slab, locate the first free slot */
        const SceSize slot = FIRST_FREE_SLOT(sch->partial.get(mem)->slots);
        sch->partial.get(mem)->slots ^= SLOTS_FIRST << slot;

        if (UNLIKELY(sch->partial.get(mem)->slots == SLOTS_ALL_ZERO)) {
            /* slab has become full, change state from partial to full */
            const auto tmp = sch->partial;

            /* skip first slab from partial list */
            if (LIKELY((sch->partial = sch->partial.get(mem)->next).address() != 0))
                sch->partial.get(mem)->prev = nullptr;

            if (LIKELY((tmp.get(mem)->next = sch->full).address() != 0))
                sch->full.get(mem)->prev = tmp;

            sch->full = tmp;
            *exe_addr = sch->full.get(mem)->exe_data + slot * sch->itemsize;
            return sch->full.get(mem)->data + slot * sch->itemsize;
        } else {
            *exe_addr = sch->partial.get(mem)->exe_data + slot * sch->itemsize;
            return sch->partial.get(mem)->data + slot * sch->itemsize;
        }
    } else if (LIKELY((sch->partial = sch->empty).address() != 0)) {
        /* found an empty slab, change state from empty to partial */
        if (LIKELY((sch->empty = sch->empty.get(mem)->next).address() != 0))
            sch->empty.get(mem)->prev = nullptr;

        sch->partial.get(mem)->next = nullptr;

        /* slab is located either at the beginning of page, or beyond */
        UNLIKELY(sch->partial.get(mem)->refcount != 0) ? sch->partial.get(mem)->refcount++ : sch->partial.get(mem)->page.get(mem)->refcount++;

        sch->partial.get(mem)->slots = sch->initial_slotmask;
        *exe_addr = sch->partial.get(mem)->exe_data;
        return sch->partial.get(mem)->data;
    } else {
        /* no empty or partial slabs available, create a new one */
        SceUID write_res, exe_res;
        Address exe_data;
        if ((write_res = sce_exe_alloc(mem, sch->pid, (Ptr<void> *)&sch->partial, &exe_data,
                 &exe_res, sch->slabsize, sch->pages_per_alloc))
            < 0) {
            *exe_addr = 0;
            return sch->partial = nullptr;
        }
        sch->partial.get(mem)->write_res = write_res;
        sch->partial.get(mem)->exe_res = exe_res;
        sch->partial.get(mem)->exe_data = exe_data + offsetof(struct slab_header, data);
        exe_data += sch->slabsize;

        Ptr<slab_header> prev{};

        const auto page_end = sch->partial.address() + sch->pages_per_alloc;

        auto curr = Ptr<slab_header>(sch->partial.address() + sch->slabsize);

        //__builtin_prefetch(sch->partial, 1);

        sch->partial.get(mem)->prev = sch->partial.get(mem)->next = nullptr;
        sch->partial.get(mem)->refcount = 1;
        sch->partial.get(mem)->slots = sch->initial_slotmask;

        if (LIKELY(curr.address() != page_end)) {
            auto curr_s = curr.get(mem);
            curr_s->prev = nullptr;
            curr_s->refcount = 0;
            curr_s->page = sch->partial;
            curr_s->write_res = write_res;
            curr_s->exe_res = exe_res;
            curr_s->exe_data = exe_data;
            exe_data += sch->slabsize;
            curr_s->slots = sch->empty_slotmask;
            sch->empty = prev = curr;

            while (LIKELY((curr = Ptr<slab_header>(curr.address() + sch->slabsize)).address() != page_end)) {
                prev.get(mem)->next = curr;
                curr_s->prev = prev;
                curr_s->refcount = 0;
                curr_s->page = sch->partial;
                curr_s->write_res = write_res;
                curr_s->exe_res = exe_res;
                curr_s->exe_data = exe_data;
                exe_data += sch->slabsize;
                curr_s->slots = sch->empty_slotmask;
                prev = curr;
            }

            prev.get(mem)->next = nullptr;
        }

        *exe_addr = sch->partial.get(mem)->exe_data;
        return sch->partial.get(mem)->data;
    }

    /* unreachable */
}

void slab_free(MemState &mem, const Ptr<slab_chain> sch, const Ptr<void> addr) {
    assert(sch != nullptr);
    assert(slab_is_valid(sch));
    assert(addr != nullptr);

    const auto slab = Ptr<slab_header>(addr.address() & sch.get(mem)->alignment_mask);

    const int slot = (addr.address() - slab.address() - offsetof(struct slab_header, data)) / sch.get(mem)->itemsize;

    if (UNLIKELY(slab.get(mem)->slots == SLOTS_ALL_ZERO)) {
        /* target slab is full, change state to partial */
        slab.get(mem)->slots = SLOTS_FIRST << slot;

        if (LIKELY(slab != sch.get(mem)->full)) {
            if (LIKELY((slab.get(mem)->prev.get(mem)->next = slab.get(mem)->next).address() != 0))
                slab.get(mem)->next.get(mem)->prev = slab.get(mem)->prev;

            slab.get(mem)->prev = nullptr;
        } else if (LIKELY((sch.get(mem)->full = sch.get(mem)->full.get(mem)->next).address() != 0)) {
            sch.get(mem)->full.get(mem)->prev = nullptr;
        }

        slab.get(mem)->next = sch.get(mem)->partial;

        if (LIKELY(sch.get(mem)->partial.address() != 0))
            sch.get(mem)->partial.get(mem)->prev = slab;

        sch.get(mem)->partial = slab;
    } else if (UNLIKELY(ONE_USED_SLOT(slab.get(mem)->slots, sch.get(mem)->empty_slotmask))) {
        /* target slab is partial and has only one filled slot */
        if (UNLIKELY(slab.get(mem)->refcount == 1 || (slab.get(mem)->refcount == 0 && slab.get(mem)->page.get(mem)->refcount == 1))) {
            /* unmap the whole page if this slab is the only partial one */
            if (LIKELY(slab != sch.get(mem)->partial)) {
                if (LIKELY((slab.get(mem)->prev.get(mem)->next = slab.get(mem)->next).address() != 0))
                    slab.get(mem)->next.get(mem)->prev = slab.get(mem)->prev;
            } else if (LIKELY((sch.get(mem)->partial = sch.get(mem)->partial.get(mem)->next).address() != 0)) {
                sch.get(mem)->partial.get(mem)->prev = nullptr;
            }

            auto page = UNLIKELY(slab.get(mem)->refcount != 0) ? slab : slab.get(mem)->page;
            auto page_end = page.address() + sch.get(mem)->pages_per_alloc;
            char found_head = 0;

            union {
                const char *c;
                const struct slab_header *const s;
            } s;

            for (auto s = page; s.address() != page_end; s = Ptr<slab_header>(s.address() + sch.get(mem)->slabsize)) {
                if (UNLIKELY(s == sch.get(mem)->empty))
                    found_head = 1;
                else if (UNLIKELY(s == slab))
                    continue;
                else if (LIKELY((s.get(mem)->prev.get(mem)->next = s.get(mem)->next).address() != 0))
                    s.get(mem)->next.get(mem)->prev = s.get(mem)->prev;
            }

            if (UNLIKELY(found_head && (sch.get(mem)->empty = sch.get(mem)->empty.get(mem)->next).address() != 0))
                sch.get(mem)->empty.get(mem)->prev = nullptr;

            sce_exe_free(mem, slab.get(mem)->write_res, slab.get(mem)->exe_res);
        } else {
            slab.get(mem)->slots = sch.get(mem)->empty_slotmask;

            if (LIKELY(slab != sch.get(mem)->partial)) {
                if (LIKELY((slab.get(mem)->prev.get(mem)->next = slab.get(mem)->next).address() != 0))
                    slab.get(mem)->next.get(mem)->prev = slab.get(mem)->prev;

                slab.get(mem)->prev = nullptr;
            } else if (LIKELY((sch.get(mem)->partial = sch.get(mem)->partial.get(mem)->next).address() != 0)) {
                sch.get(mem)->partial.get(mem)->prev = nullptr;
            }

            slab.get(mem)->next = sch.get(mem)->empty;

            if (LIKELY(sch.get(mem)->empty.address() != 0))
                sch.get(mem)->empty.get(mem)->prev = slab;

            sch.get(mem)->empty = slab;

            UNLIKELY(slab.get(mem)->refcount != 0) ? slab.get(mem)->refcount-- : slab.get(mem)->page.get(mem)->refcount--;
        }
    } else {
        /* target slab is partial, no need to change state */
        slab.get(mem)->slots |= SLOTS_FIRST << slot;
    }
}

Address slab_getmirror(MemState &mem, const Ptr<slab_chain> sch, const Ptr<void> addr) {
    assert(sch != nullptr);
    assert(slab_is_valid(sch));
    assert(addr != nullptr);

    Ptr<slab_header> slab = Ptr<slab_header>(addr.address() & sch.get(mem)->alignment_mask);

    return slab.get(mem)->exe_data - offsetof(struct slab_header, data) + addr.address() - slab.address();
}

void slab_traverse(MemState &mem, const Ptr<const slab_chain> sch, void (*fn)(Ptr<void>)) {
    assert(sch != nullptr);
    assert(fn != nullptr);
    assert(slab_is_valid(sch));

    Ptr<const slab_header> slab;
    Address item, end;
    const SceSize data_offset = offsetof(struct slab_header, data);

    for (slab = sch.get(mem)->partial; slab; slab = slab.get(mem)->next) {
        item = slab.address() + data_offset;
        end = item + sch.get(mem)->itemcount * sch.get(mem)->itemsize;
        uint64_t mask = SLOTS_FIRST;

        do {
            if (!(slab.get(mem)->slots & mask))
                fn(Ptr<void>(item));

            mask <<= 1;
        } while ((item += sch.get(mem)->itemsize) != end);
    }

    for (slab = sch.get(mem)->full; slab; slab = slab.get(mem)->next) {
        item = slab.address() + data_offset;
        end = item + sch.get(mem)->itemcount * sch.get(mem)->itemsize;

        do
            fn(Ptr<void>(item));
        while ((item += sch.get(mem)->itemsize) != end);
    }
}

void slab_destroy(MemState &mem, const Ptr<const slab_chain> sch) {
    assert(sch != nullptr);
    assert(slab_is_valid(sch));

    std::array heads = { sch.get(mem)->partial, sch.get(mem)->empty, sch.get(mem)->full };
    Ptr<slab_header> pages_head{};
    Ptr<slab_header> pages_tail;

    for (SceSize i = 0; i < 3; ++i) {
        auto slab = heads[i];

        while (slab.address() != 0) {
            if (slab.get(mem)->refcount != 0) {
                const Ptr<slab_header> page = slab;
                slab = slab.get(mem)->next;

                if (UNLIKELY(pages_head.address() == 0))
                    pages_head = page;
                else
                    pages_tail.get(mem)->next = page;

                pages_tail = page;
            } else {
                slab = slab.get(mem)->next;
            }
        }
    }

    if (LIKELY(pages_head.address() != 0)) {
        pages_tail.get(mem)->next = nullptr;
        Ptr<slab_header> page = pages_head;

        do {
            auto target = page;
            page = page.get(mem)->next;
            sce_exe_free(mem, target.get(mem)->write_res, target.get(mem)->exe_res);
        } while (page.address() != 0);
    }
}
