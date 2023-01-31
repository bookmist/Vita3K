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

#include "SceFios2Kernel.h"

#include <util/tracy.h>
TRACY_MODULE_NAME(SceFios2Kernel);

EXPORT(int, _sceFiosKernelOverlayAdd) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayAddForProcess) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayDHChstatSync) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayDHCloseSync) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayDHOpenSync) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayDHReadSync) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayDHStatSync) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayDHSyncSync) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayGetInfo) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayGetInfoForProcess) {
    return UNIMPLEMENTED();
}

typedef int32_t SceFiosKernelOverlayID;

struct SceFiosGetListSyscallArgs {
    SceFiosKernelOverlayID *out_ids;
    int data_0x04;
    int data_0x08;
    SceSize data_0x0C;
    int data_0x10;
    int data_0x14;
};

EXPORT(int, _sceFiosKernelOverlayGetList, SceUID pid, SceUInt8 min_order, SceUInt8 max_order, SceFiosGetListSyscallArgs *args) {
    TRACY_FUNC(_sceFiosKernelOverlayGetList, pid, min_order, max_order, args);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayGetRecommendedScheduler) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayModify) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayModifyForProcess) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayRemove) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayRemoveForProcess) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayResolveSync) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayResolveWithRangeSync, SceUID pid, int resolveFlag, const char *pInPath, sceFiosKernelOverlayResolveWithRangeSync_opt *opt) {
    TRACY_FUNC(_sceFiosKernelOverlayResolveWithRangeSync, pid, resolveFlag, pInPath, opt);
    STUBBED("Using strncpy");
    strncpy(opt->pOutPath.get(emuenv.mem), pInPath, opt->maxPath);

    return 0;
}

EXPORT(int, _sceFiosKernelOverlayThreadIsDisabled) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceFiosKernelOverlayThreadSetDisabled, SceInt32 disabled) {
    TRACY_FUNC(_sceFiosKernelOverlayThreadSetDisabled, disabled);
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(_sceFiosKernelOverlayAdd)
BRIDGE_IMPL(_sceFiosKernelOverlayAddForProcess)
BRIDGE_IMPL(_sceFiosKernelOverlayDHChstatSync)
BRIDGE_IMPL(_sceFiosKernelOverlayDHCloseSync)
BRIDGE_IMPL(_sceFiosKernelOverlayDHOpenSync)
BRIDGE_IMPL(_sceFiosKernelOverlayDHReadSync)
BRIDGE_IMPL(_sceFiosKernelOverlayDHStatSync)
BRIDGE_IMPL(_sceFiosKernelOverlayDHSyncSync)
BRIDGE_IMPL(_sceFiosKernelOverlayGetInfo)
BRIDGE_IMPL(_sceFiosKernelOverlayGetInfoForProcess)
BRIDGE_IMPL(_sceFiosKernelOverlayGetList)
BRIDGE_IMPL(_sceFiosKernelOverlayGetRecommendedScheduler)
BRIDGE_IMPL(_sceFiosKernelOverlayModify)
BRIDGE_IMPL(_sceFiosKernelOverlayModifyForProcess)
BRIDGE_IMPL(_sceFiosKernelOverlayRemove)
BRIDGE_IMPL(_sceFiosKernelOverlayRemoveForProcess)
BRIDGE_IMPL(_sceFiosKernelOverlayResolveSync)
BRIDGE_IMPL(_sceFiosKernelOverlayResolveWithRangeSync)
BRIDGE_IMPL(_sceFiosKernelOverlayThreadIsDisabled)
BRIDGE_IMPL(_sceFiosKernelOverlayThreadSetDisabled)
