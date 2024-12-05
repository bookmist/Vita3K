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

#include "../SceFios2Kernel/SceFios2Kernel02.h"

#include "io/types.h"

TRACY_MODULE_NAME(SceFios2User);

EXPORT(int, sceFiosOverlayAddForProcess02, SceUID processId, SceFiosProcessOverlay *pOverlay, SceFiosOverlayID *pOutID) {
    TRACY_FUNC(sceFiosOverlayAddForProcess02, processId, pOverlay, pOutID);
    return CALL_EXPORT(sceFiosKernelOverlayAddForProcess02, processId, pOverlay, pOutID);
}

EXPORT(int, sceFiosOverlayGetInfoForProcess02) {
    TRACY_FUNC(sceFiosOverlayGetInfoForProcess02);
    return CALL_EXPORT(sceFiosKernelOverlayGetInfoForProcess02);
}

EXPORT(int, sceFiosOverlayGetList02, SceUID processId, uint32_t minOrder, uint32_t maxOrder, Ptr<SceFiosOverlayID> pOutIDs, SceUInt32 maxIDs, Ptr<SceUInt32> pActualIDs) {
    TRACY_FUNC(sceFiosOverlayGetList02, processId, minOrder, maxOrder, pOutIDs, maxIDs, pActualIDs);
    sceFiosKernelOverlayGetList02_opt opt{ pOutIDs, maxIDs, pActualIDs };
    return CALL_EXPORT(sceFiosKernelOverlayGetList02, processId, minOrder, maxOrder, &opt);
}

EXPORT(int, sceFiosOverlayGetRecommendedScheduler02, int param1, const char *path) {
    TRACY_FUNC(sceFiosOverlayGetRecommendedScheduler02, param1, path);
    return CALL_EXPORT(sceFiosKernelOverlayGetRecommendedScheduler02, param1, path);
}

EXPORT(int, sceFiosOverlayModifyForProcess02) {
    TRACY_FUNC(sceFiosOverlayModifyForProcess02);
    return CALL_EXPORT(sceFiosKernelOverlayModifyForProcess02);
}

EXPORT(int, sceFiosOverlayRemoveForProcess02) {
    TRACY_FUNC(sceFiosOverlayRemoveForProcess02);
    return CALL_EXPORT(sceFiosKernelOverlayRemoveForProcess02);
}

EXPORT(int, sceFiosOverlayResolveSync02) {
    TRACY_FUNC(sceFiosOverlayResolveSync02);
    return CALL_EXPORT(sceFiosKernelOverlayResolveSync02);
}

EXPORT(int, sceFiosOverlayResolveWithRangeSync02, SceUID processId, SceFiosOverlayResolveMode resolveFlag, const char *pInPath, Ptr<char> pOutPath, SceUInt32 maxPath, SceUInt32 min_order, SceUInt32 max_order) {
    TRACY_FUNC(sceFiosOverlayResolveWithRangeSync02, processId, resolveFlag, pInPath, pOutPath, maxPath, min_order, max_order);
    sceFiosKernelOverlayResolveWithRangeSync02_opt opt{ pOutPath, maxPath, (SceUInt8)min_order, (SceUInt8)max_order };
    return CALL_EXPORT(sceFiosKernelOverlayResolveWithRangeSync02, processId, resolveFlag, pInPath, &opt);
}

EXPORT(int, sceFiosOverlayThreadIsDisabled02) {
    TRACY_FUNC(sceFiosOverlayThreadIsDisabled02);
    return CALL_EXPORT(sceFiosKernelOverlayThreadIsDisabled02);
}

EXPORT(int, sceFiosOverlayThreadSetDisabled02) {
    TRACY_FUNC(sceFiosOverlayThreadSetDisabled02);
    return CALL_EXPORT(sceFiosKernelOverlayThreadSetDisabled02);
}
