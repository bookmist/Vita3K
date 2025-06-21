// Vita3K emulator project
// Copyright (C) 2026 Vita3K team
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

#include "../SceLibKernel/SceLibKernel.h"
#include "SceSysmem.h"
#include <module/module.h>
#include <util/tracy.h>

TRACY_MODULE_NAME(SceSysmemForDriver)

EXPORT(int, ksceGUIDClose) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceGUIDReferObject) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceGUIDReferObjectWithClass) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceGUIDReferObjectWithClassLevel) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceGUIDReleaseObject) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelAddressSpaceVAtoPABySW) {
    return UNIMPLEMENTED();
}

EXPORT(Ptr<void>, ksceKernelAllocHeapMemory, SceUID uid, SceSize size) {
    STUBBED("");
    return CALL_EXPORT(sceClibMspaceMalloc, Ptr<void>(uid), size);
    // return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelAllocHeapMemoryFromGlobalHeap) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelAllocHeapMemoryFromGlobalHeapWithOpt) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelAllocHeapMemoryWithOpt1) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelAllocHeapMemoryWithOption) {
    return UNIMPLEMENTED();
}

#define VITASDK_BUILD_ASSERT_EQ(a, b) static_assert(sizeof(b) == a, "Size mismatch")

typedef struct SceKernelAddrPair { // do not use
    uint32_t addr; //!< Address
    uint32_t length; //!< Length
} SceKernelAddrPair;
VITASDK_BUILD_ASSERT_EQ(8, SceKernelAddrPair);

typedef struct SceKernelVARange { // size is 0x8
    uint32_t addr;
    SceSize size;
} SceKernelVARange;
VITASDK_BUILD_ASSERT_EQ(8, SceKernelVARange);

typedef struct SceKernelPARange { // size is 0x8
    uint32_t addr;
    SceSize size;
} SceKernelPARange;
VITASDK_BUILD_ASSERT_EQ(8, SceKernelPARange);

typedef struct SceKernelPAVector { // size is 0x14
    SceSize size; //!< Size of this structure
    union {
        struct {
            uint32_t ranges_size; //!< Ex: 8
            uint32_t data_in_vector; //!< Must be <= 8
            uint32_t count;
            Ptr<SceKernelPARange> ranges;
        };
        struct { // do not use.
            uint32_t list_size; //!< Size in elements of the list array
            uint32_t ret_length; //!< Total physical size of the memory pairs
            uint32_t ret_count; //!< Number of elements of list filled by ksceKernelVARangeToPAVector
            Ptr<SceKernelAddrPair> list; //!< Array of physical addresses and their lengths pairs
        };
    };
} SceKernelPAVector;
VITASDK_BUILD_ASSERT_EQ(0x14, SceKernelPAVector);

typedef SceKernelPAVector SceKernelPaddrList; // do not use.

typedef struct SceKernelAllocMemBlockKernelOpt {
    SceSize size; //!< sizeof(SceKernelAllocMemBlockKernelOpt)
    SceUInt32 field_4;
    SceUInt32 attr; //!< OR of SceKernelAllocMemBlockAttr
    SceUInt32 field_C;
    SceUInt32 paddr;
    SceSize alignment;
    SceUInt32 extraLow;
    SceUInt32 extraHigh;
    SceUInt32 mirror_blockid;
    SceUID pid;
    Ptr<SceKernelPaddrList> paddr_list;
    SceUInt32 field_2C;
    SceUInt32 field_30;
    SceUInt32 field_34;
    SceUInt32 field_38;
    SceUInt32 field_3C;
    SceUInt32 field_40;
    SceUInt32 field_44;
    SceUInt32 field_48;
    SceUInt32 field_4C;
    SceUInt32 field_50;
    SceUInt32 field_54;
} SceKernelAllocMemBlockKernelOpt;
VITASDK_BUILD_ASSERT_EQ(0x58, SceKernelAllocMemBlockKernelOpt);

EXPORT(SceUID, ksceKernelAllocMemBlock, const char *pName, SceKernelMemBlockType type, SceSize size, SceKernelAllocMemBlockKernelOpt *optp) {
    TRACY_FUNC(ksceKernelAllocMemBlock, pName, (int)type, size, optp);
    LOG_CONSOLE(ksceKernelAllocMemBlock, pName, (int)type, size, optp);
    SceKernelAllocMemBlockOpt opt;
    SceKernelAllocMemBlockOpt *popt = nullptr;
    if (optp) {
        popt = &opt;
        opt.size = sizeof(SceKernelAllocMemBlockOpt);
        opt.attr = optp->attr;
        opt.alignment = optp->alignment;
    }
    return CALL_EXPORT(sceKernelAllocMemBlock, pName, type, size, popt);
}

EXPORT(int, ksceKernelAllocMemBlockWithInfo) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelCreateClass) {
    return UNIMPLEMENTED();
}

typedef struct SceKernelHeapCreateOpt {
    SceSize size;
    union { //<! Union for compatibility
        SceUInt32 attr;
        SceUInt32 uselock; //<! Do not use uselock as it will be deprecated.
    };
    SceUInt32 field_8;
    SceUInt32 field_C;
    SceUInt32 memtype;
    SceUInt32 field_14;
    SceUInt32 field_18;
} SceKernelHeapCreateOpt;

typedef struct SceAllocOpt {
    SceSize size; // 0x14
    SceSize data04; // maybe len align?
    SceSize align;
    int data0C;
    int data10;
} SceAllocOpt;

EXPORT(int, ksceKernelCreateHeap, const char *name, SceSize size, SceKernelHeapCreateOpt *opt) {
    TRACY_FUNC(ksceKernelCreateHeap, name, size, opt);
    STUBBED("");
    int new_size = align(size, 0x8000);
    auto heap = CALL_EXPORT(sceKernelAllocMemBlock, name, SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, new_size, nullptr); // UNIMPLEMENTED();
    Ptr<void> base;
    CALL_EXPORT(sceKernelGetMemBlockBase, heap, &base);
    auto mspace = CALL_EXPORT(sceClibMspaceCreate, base, new_size);
    LOG_DEBUG("ksceKernelCreateHeap: {} {} {}", name, size, log_hex(mspace.address()));
    return mspace.address(); // UNIMPLEMENTED();
}

EXPORT(int, ksceKernelCreateUidObj2) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelCreateUidObjForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelCreateUserUidForClass) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelCreateUserUidForName) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelCreateUserUidForNameWithClass) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelDeleteHeap) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFindMemBlock) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFindMemBlockByAddr) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFindMemBlockByAddrForDefaultSize) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFindMemBlockByAddrForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFindMemBlockForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFirstDifferentBlock32User) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFirstDifferentBlock64User) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFirstDifferentBlock64UserForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFirstDifferentIntUserForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFreeHeapMemory) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFreeHeapMemoryFromGlobalHeap) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelFreeMemBlock) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGUIDGetObject) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetClassForPidForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetClassForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetMemBlockBase, SceUID uid, Ptr<void> *basep) {
    TRACY_FUNC(ksceKernelGetMemBlockBase, uid, basep);
    return CALL_EXPORT(sceKernelGetMemBlockBase, uid, basep);
}

EXPORT(int, ksceKernelGetMemBlockMappedBase) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetMemBlockPARange) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetMemBlockPaddrListForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetMemBlockVBase) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetNameForPidByUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetNameForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetNameForUid2) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetObjectForPidForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetObjectForUidForAttr) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetObjectForUidForClassTree) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetPaddrListForLargePage) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetPaddrListForSmallPage) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetPaddrPair) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetPaddrPairForLargePage) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetPaddrPairForSmallPage) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetPhysicalMemoryType) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetPidContext) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelGetUidClass) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelIsPaddrWithinSameSectionForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelKernelUidForUserUidForClass) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMapBlockUserVisible) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMapBlockUserVisibleWithFlag) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMapUserBlock) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockDecRefCounterAndReleaseUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockGetInfoEx) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockGetInfoExForVisibilityLevel) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockGetSomeSize) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockIncRefCounterAndReleaseUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockRelease) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockType2Memtype) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemBlockTypeGetPrivileges) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemRangeRelease) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemRangeReleaseForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemRangeReleaseWithPerm) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemRangeRetain) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemRangeRetainForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemRangeRetainWithPerm) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemcpyKernelToUser, void *dst, const void *src, SceSize len) {
    TRACY_FUNC(ksceKernelMemcpyKernelToUser, dst, src, len);
    memcpy(dst, src, len);
    return 0;
    // int ksceKernelCopyToUser(void *dst, const void *src, SceSize len);
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemcpyKernelToUserForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemcpyKernelToUserForPidUnchecked) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemcpyUserToKernel, void *dst, const void *src, SceSize len) {
    TRACY_FUNC(ksceKernelMemcpyUserToKernel, dst, src, len);
    memcpy(dst, src, len);
    return 0;
}

EXPORT(int, ksceKernelMemcpyUserToKernelForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemcpyUserToUser) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelMemcpyUserToUserForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelOpenUidForName) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelProcModeVAtoPA) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelProcUserMap) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelRemapBlock) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelRoMemcpyKernelToUserForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelSetNameForPidForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelSetObjectForUid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelStrncpyKernelToUser) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelStrncpyUserForPid) {
    return UNIMPLEMENTED();
}

EXPORT(SceSSize, ksceKernelStrncpyUserToKernel, Ptr<char> dst, const char *src, SceSize len) {
    strncpy(dst.get(emuenv.mem), src, len);
    return strnlen_s(src, len);
}

EXPORT(int, ksceKernelStrnlenUser) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelStrnlenUserForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelSwitchPidContext) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelSwitchVmaForPid) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelUnmapMemBlock) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelUserMap) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelVARangeToPAVector) {
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelVAtoPA) {
    return UNIMPLEMENTED();
}

EXPORT(int, kscePUIDClose) {
    return UNIMPLEMENTED();
}

EXPORT(int, kscePUIDOpenByGUID) {
    return UNIMPLEMENTED();
}

EXPORT(int, kscePUIDtoGUID) {
    return UNIMPLEMENTED();
}
