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

#include "SceGxmInternal.h"

#include "SceGxm.h"

#include <gxm/types.h>
#include <mem/ptr.h>

EXPORT(int, sceGxmCheckMemoryInternal) {
    return UNIMPLEMENTED();
}

struct SceGxmRenderTarget;

EXPORT(int, sceGxmCreateRenderTargetInternal, const SceGxmRenderTargetParams *params, Ptr<SceGxmRenderTarget> *renderTarget) {
    return CALL_EXPORT(sceGxmCreateRenderTarget, params, renderTarget);
}

EXPORT(int, sceGxmGetDisplayQueueThreadIdInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmGetRenderTargetMemSizeInternal, const SceGxmRenderTargetParams *params, uint32_t *hostMemSize) {
    return CALL_EXPORT(sceGxmGetRenderTargetMemSize, params, hostMemSize);
}

EXPORT(int, sceGxmGetTopContextInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmInitializedInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmIsInitializationInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmMapFragmentUsseMemoryInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmMapVertexUsseMemoryInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmRenderingContextIsWithinSceneInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmSetCallbackInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmSetInitializeParamInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmUnmapFragmentUsseMemoryInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceGxmUnmapVertexUsseMemoryInternal) {
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceGxmCheckMemoryInternal)
BRIDGE_IMPL(sceGxmCreateRenderTargetInternal)
BRIDGE_IMPL(sceGxmGetDisplayQueueThreadIdInternal)
BRIDGE_IMPL(sceGxmGetRenderTargetMemSizeInternal)
BRIDGE_IMPL(sceGxmGetTopContextInternal)
BRIDGE_IMPL(sceGxmInitializedInternal)
BRIDGE_IMPL(sceGxmIsInitializationInternal)
BRIDGE_IMPL(sceGxmMapFragmentUsseMemoryInternal)
BRIDGE_IMPL(sceGxmMapVertexUsseMemoryInternal)
BRIDGE_IMPL(sceGxmRenderingContextIsWithinSceneInternal)
BRIDGE_IMPL(sceGxmSetCallbackInternal)
BRIDGE_IMPL(sceGxmSetInitializeParamInternal)
BRIDGE_IMPL(sceGxmUnmapFragmentUsseMemoryInternal)
BRIDGE_IMPL(sceGxmUnmapVertexUsseMemoryInternal)
