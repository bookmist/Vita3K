/* ref: https://github.com/bbu/userland-slab-allocator */

#pragma once

#include <kernel/cpu_protocol.h>
#include <mem/functions.h>
#include <mem/ptr.h>

#include <util/types.h>

extern const SceSize slab_pagesize;

struct slab_header {
    Ptr<slab_header> prev, next;
    uint64_t slots;
    Address refcount;
    Ptr<slab_header> page;
    SceUID write_res;
    SceUID exe_res;
    Address exe_data;
    Ptr<uint8_t> data;
};

struct slab_chain {
    SceSize itemsize, itemcount;
    SceSize slabsize, pages_per_alloc;
    uint64_t initial_slotmask, empty_slotmask;
    Address alignment_mask;
    Ptr<slab_header> partial, empty, full;
    SceUID pid;
};

void slab_init(MemState &, Ptr<slab_chain>, SceSize, SceUID);
Ptr<void> slab_alloc(MemState &, Ptr<slab_chain>, Address *);
void slab_free(MemState &, Ptr<slab_chain>, Ptr<void>);
Address slab_getmirror(MemState &, Ptr<slab_chain>, Ptr<void>);
void slab_traverse(MemState &, Ptr<slab_chain>, void (*)(Ptr<void>));
void slab_destroy(MemState &, Ptr<slab_chain>);
