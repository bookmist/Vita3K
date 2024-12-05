// Vita3K emulator project
// Copyright (C) 2025 Vita3K team
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

#pragma once

#include "io/types.h"
#include <module/module.h>
#include <util/tracy.h>

#include "io/functions.h"

enum SceFiosErrorCode {
    SCE_FIOS_OK = 0
};

typedef SceUID SceFiosOverlayID;

enum SceFiosOverlayResolveMode {
    SCE_FIOS_OVERLAY_RESOLVE_FOR_READ = 0,
    SCE_FIOS_OVERLAY_RESOLVE_FOR_WRITE = 1
};

struct sceFiosKernelOverlayResolveWithRangeSync02_opt {
    Ptr<char> pOutPath;
    SceSize maxPath;
    SceUInt8 loOrderFilter;
    SceUInt8 hiOrderFilter;
    SceUInt8 reserved1;
    SceUInt8 reserved2;
    int reserved3;
    int reserved4;
    int reserved5;
    int reserved6;
};

struct sceFiosKernelOverlayGetList02_opt { // size is 0x18
    Ptr<SceFiosOverlayID> pOutIDs;
    uint32_t maxIDs;
    Ptr<uint32_t> pActualIDs;
    int buffer_size;
    int reserved1;
    int reserved2;
};

DECL_EXPORT(int, sceFiosKernelOverlayAddForProcess02, SceUID processId, SceFiosProcessOverlay *pOverlay, SceFiosOverlayID *pOutID);
DECL_EXPORT(int, sceFiosKernelOverlayGetInfoForProcess02);
DECL_EXPORT(int, sceFiosKernelOverlayGetList02, SceUID processId, uint32_t minOrder, uint32_t maxOrder, sceFiosKernelOverlayGetList02_opt *opt);
DECL_EXPORT(int, sceFiosKernelOverlayGetRecommendedScheduler02, int param1, const char *path);
DECL_EXPORT(int, sceFiosKernelOverlayModifyForProcess02);
DECL_EXPORT(int, sceFiosKernelOverlayRemoveForProcess02);
DECL_EXPORT(int, sceFiosKernelOverlayResolveSync02);
DECL_EXPORT(int, sceFiosKernelOverlayResolveWithRangeSync02, SceUID processId, int resolveFlag, const char *pInPath, sceFiosKernelOverlayResolveWithRangeSync02_opt *opt);
DECL_EXPORT(int, sceFiosKernelOverlayThreadIsDisabled02);
DECL_EXPORT(int, sceFiosKernelOverlayThreadSetDisabled02);
