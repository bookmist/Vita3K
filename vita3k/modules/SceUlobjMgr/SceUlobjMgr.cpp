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

#include <util/tracy.h>
TRACY_MODULE_NAME(SceUlobjMgr);

EXPORT(int, _sceUlobjMgrRegisterLibultProtocolRevision, SceUInt32 revision) {
    TRACY_FUNC(_sceUlobjMgrRegisterLibultProtocolRevision, revision);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceUlobjMgrStartSupportingUserlevelObject) {
    return UNIMPLEMENTED();
}

EXPORT(int, _sceUlobjMgrStopSupportingUserlevelObject) {
    return UNIMPLEMENTED();
}

// register something
EXPORT(int, SceUlobjDbg_D7F0F610, uint32_t param_1, uint32_t param_2, Ptr<uint32_t> param_3) {
    TRACY_FUNC(SceUlobjDbg_D7F0F610, param_1, param_2, param_3);
    STUBBED("SceUlobjDbg_D7F0F610");
    if (!param_3) {
        return 0x8001000e; // SCE_ERROR_ERRNO_EFAULT
    } else {
        if (((param_2 & 0xfffffffc) == 0) && ((param_1 & 7) == 0)) {
            *(param_3.get(emuenv.mem)) = 0x1fffffff;
            return 0x8001000c; // SCE_ERROR_ERRNO_ENOMEM
        } else {
            *(param_3.get(emuenv.mem)) = 0x0DEADBEE;
            STUBBED("");
            return 0x80010016; // SCE_ERROR_ERRNO_EINVAL
        }
    }
    // return UNIMPLEMENTED();
}
// unregister something
EXPORT(int, SceUlobjDbg_F9C0F5DA, uint32_t param_1) {
    TRACY_FUNC(SceUlobjDbg_F9C0F5DA, param_1);
    return UNIMPLEMENTED();
}
