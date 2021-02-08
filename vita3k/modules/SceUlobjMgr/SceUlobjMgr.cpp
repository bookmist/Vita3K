// Vita3K emulator project
// Copyright (C) 2024 Vita3K team
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

#include <module/module.h>

EXPORT(int, _sceUlobjMgrRegisterLibultProtocolRevision) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceUlobjMgrStartSupportingUserlevelObject) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceUlobjMgrStopSupportingUserlevelObject) {
    return UNIMPLEMENTED();
}

//register something
EXPORT(int, SceUlobjDbg_D7F0F610, uint32_t param_1, uint32_t param_2, Ptr<uint32_t> param_3) {
    STUBBED("SceUlobjDbg_D7F0F610");
    if (!param_3) {
        return 0x8001000e;
    } else {
        if (((param_2 & 0xfffffffc) == 0) && ((param_1 & 7) == 0)) {
            *(param_3.get(emuenv.mem)) = 0x1fffffff;
            return 0x8001000c;
        } else {
            *(param_3.get(emuenv.mem)) = 0x0DEADBEE;
            return 0x80010016;
        }
    }
    //return UNIMPLEMENTED();
}
//unregister something
EXPORT(int, SceUlobjDbg_F9C0F5DA) {
    return UNIMPLEMENTED();
}
