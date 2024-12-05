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

#include "SceFios2Kernel02.h"

#include "io/functions.h"

#include <util/tracy.h>
TRACY_MODULE_NAME(SceFios2Kernel);

template <>
std::string to_debug_str<SceFiosOverlayResolveMode>(const MemState &mem, SceFiosOverlayResolveMode type);

EXPORT(int, sceFiosKernelOverlayAddForProcess02, SceUID processId, SceFiosProcessOverlay *pOverlay, SceFiosOverlayID *pOutID) {
    TRACY_FUNC(sceFiosKernelOverlayAddForProcess02, processId, pOverlay, pOutID);
    if (pOverlay->type != SCE_FIOS_OVERLAY_TYPE_OPAQUE)
        LOG_WARN("Using unimplemented overlay type {}.", fmt::underlying(pOverlay->type));

    *pOutID = create_overlay(emuenv.io, pOverlay);

    return SCE_FIOS_OK;
}

EXPORT(int, sceFiosKernelOverlayGetInfoForProcess02) {
    TRACY_FUNC(sceFiosKernelOverlayGetInfoForProcess02);
    return UNIMPLEMENTED();
}

EXPORT(int, sceFiosKernelOverlayGetList02, SceUID processId, uint32_t minOrder, uint32_t maxOrder, sceFiosKernelOverlayGetList02_opt *opt) {
    TRACY_FUNC(sceFiosKernelOverlayGetList02, processId, minOrder, maxOrder, opt->pOutIDs, opt->maxIDs, opt->pActualIDs);
    const std::lock_guard<std::mutex> guard(emuenv.io.overlay_mutex);

    std::vector<SceFiosOverlayID> overlay_ids;
    for (const auto &overlay : emuenv.io.overlays) {
        if (overlay.order >= minOrder && overlay.order <= maxOrder)
            overlay_ids.push_back(overlay.id);
    }

    if (opt->pActualIDs)
        *opt->pActualIDs.get(emuenv.mem) = overlay_ids.size();

    if (opt->pOutIDs)
        memcpy(opt->pOutIDs.get(emuenv.mem), overlay_ids.data(), std::min<uint32_t>(overlay_ids.size(), opt->maxIDs) * sizeof(SceFiosOverlayID));

    return SCE_FIOS_OK;
}

EXPORT(int, sceFiosKernelOverlayGetRecommendedScheduler02, int param1, const char *path) {
    TRACY_FUNC(sceFiosKernelOverlayGetRecommendedScheduler02, param1, path);
    // reversed engineered
    if (param1 <= 1)
        return 0;

    // returns if path starts with hostk: with k an integer
    if (strlen(path) < strlen("host0:"))
        return 0;

    return memcmp(path, "host", 4) == 0 && path[4] <= '9' && path[5] == ':';
}

EXPORT(int, sceFiosKernelOverlayModifyForProcess02) {
    TRACY_FUNC(sceFiosKernelOverlayModifyForProcess02);
    return UNIMPLEMENTED();
}

EXPORT(int, sceFiosKernelOverlayRemoveForProcess02) {
    TRACY_FUNC(sceFiosKernelOverlayRemoveForProcess02);
    return UNIMPLEMENTED();
}

EXPORT(int, sceFiosKernelOverlayResolveSync02) {
    TRACY_FUNC(sceFiosKernelOverlayResolveSync02);
    return UNIMPLEMENTED();
}

EXPORT(int, sceFiosKernelOverlayResolveWithRangeSync02, SceUID processId, int resolveFlag, const char *pInPath, sceFiosKernelOverlayResolveWithRangeSync02_opt *opt) {
    TRACY_FUNC(sceFiosKernelOverlayResolveWithRangeSync02, processId, resolveFlag, pInPath, opt->pOutPath, opt->maxPath, opt->loOrderFilter, opt->hiOrderFilter);
    const std::string resolved = resolve_path(emuenv.io, pInPath, opt->loOrderFilter, opt->hiOrderFilter);
    strncpy(opt->pOutPath.get(emuenv.mem), resolved.c_str(), opt->maxPath);

    return SCE_FIOS_OK;
}

EXPORT(int, sceFiosKernelOverlayThreadIsDisabled02) {
    TRACY_FUNC(sceFiosKernelOverlayThreadIsDisabled02);
    return UNIMPLEMENTED();
}

EXPORT(int, sceFiosKernelOverlayThreadSetDisabled02) {
    TRACY_FUNC(sceFiosKernelOverlayThreadSetDisabled02);
    return UNIMPLEMENTED();
}
