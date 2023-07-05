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

#include "SceRegMgrForGame.h"

EXPORT(int, sceRegMgrSystemIsBlueScreen) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSystemParamGetBin) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSystemParamGetInt, const int id, int *buf) {
    switch (id) {
    case 0x00023fc2: // /CONFIG/ACCESSIBILITY/large_text
    case 0x00611DC9: // /CONFIG/ACCESSIBILITY/bold_text
    case 0x008A2AD7: // /CONFIG/ACCESSIBILITY/contrast
        *buf = 0;
        LOG_WARN("Using default value");
        break;
    default:
        LOG_DEBUG("Unknown id: {}", log_hex(id));
        break;
    }

    return 0;
}

EXPORT(int, sceRegMgrSystemParamGetStr, const int id, SceChar8 *buf, const SceSize bufSize) {
    // LOG_DEBUG("paramId: {}, bufSize: {}", log_hex(id), bufSize);
    //  0x00186122: /CONFIG/SECURITY/PARENTAL/passcode
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSystemParamSetBin) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSystemParamSetInt) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSystemParamSetStr) {
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceRegMgrSystemIsBlueScreen)
BRIDGE_IMPL(sceRegMgrSystemParamGetBin)
BRIDGE_IMPL(sceRegMgrSystemParamGetInt)
BRIDGE_IMPL(sceRegMgrSystemParamGetStr)
BRIDGE_IMPL(sceRegMgrSystemParamSetBin)
BRIDGE_IMPL(sceRegMgrSystemParamSetInt)
BRIDGE_IMPL(sceRegMgrSystemParamSetStr)
