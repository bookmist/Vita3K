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

#include "SceDisplayUser.h"

#include <util/tracy.h>

TRACY_MODULE_NAME(SceDisplayUser);

EXPORT(SceInt32, sceDisplayGetFrameBuf, SceDisplayFrameBuf *pFrameBuf, SceDisplaySetBufSync sync) {
    TRACY_FUNC(sceDisplayGetFrameBuf, pFrameBuf, sync);
    uint32_t pFrameBuf_size = pFrameBuf->size;
    return CALL_EXPORT(_sceDisplayGetFrameBuf, pFrameBuf, sync, &pFrameBuf_size);
}

EXPORT(int, sceDisplayGetFrameBufInternal) {
    TRACY_FUNC(sceDisplayGetFrameBufInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceDisplayGetMaximumFrameBufResolution) {
    TRACY_FUNC(sceDisplayGetMaximumFrameBufResolution);
    return UNIMPLEMENTED();
}

EXPORT(int, sceDisplayGetResolutionInfoInternal) {
    TRACY_FUNC(sceDisplayGetResolutionInfoInternal);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceDisplaySetFrameBuf, const SceDisplayFrameBuf *pFrameBuf, SceDisplaySetBufSync sync) {
    TRACY_FUNC(sceDisplaySetFrameBuf, pFrameBuf, sync);
    uint32_t pFrameBuf_size = pFrameBuf->size;
    return CALL_EXPORT(_sceDisplaySetFrameBuf, pFrameBuf, sync, &pFrameBuf_size);
}

EXPORT(int, sceDisplaySetFrameBufForCompat) {
    TRACY_FUNC(sceDisplaySetFrameBufForCompat);
    return UNIMPLEMENTED();
}

EXPORT(int, sceDisplaySetFrameBufInternal, uint32_t maybe_buffer_idx, uint32_t unkn, SceDisplayFrameBuf *pFrameBuf, SceDisplaySetBufSync sync) {
    TRACY_FUNC(sceDisplaySetFrameBufInternal, maybe_buffer_idx, unkn, pFrameBuf, sync);
    // only render for frame buffer 0 or we'll get double fps
    if (maybe_buffer_idx != 0)
        return 0;
    // size does not match (is 4 bytes larger)
    pFrameBuf->size = 0x18;
    return CALL_EXPORT(_sceDisplaySetFrameBuf, pFrameBuf, sync, nullptr);
}

BRIDGE_IMPL(sceDisplayGetFrameBuf)
BRIDGE_IMPL(sceDisplayGetFrameBufInternal)
BRIDGE_IMPL(sceDisplayGetMaximumFrameBufResolution)
BRIDGE_IMPL(sceDisplayGetResolutionInfoInternal)
BRIDGE_IMPL(sceDisplaySetFrameBuf)
BRIDGE_IMPL(sceDisplaySetFrameBufForCompat)
BRIDGE_IMPL(sceDisplaySetFrameBufInternal)
