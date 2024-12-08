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

#include <../SceError/SceError.h>

TRACY_MODULE_NAME(SceErrorUser);
/*
typedef unsigned int uint;
typedef unsigned short ushort;

struct astruct1 {
    ushort field4_0x4;
    ushort field_ushort;
    char *field2;
};
struct astruct {
    uint field0_0x0;
    ushort field4_0x4;
    ushort error_messages_count;
    ushort *field6_0x8;
    ushort *field7_0xc;
    ushort error_codes_count;
    uint *field9_0x10;
};

SceUInt FUN_810001b8(astruct *param_1, char *out_buf, SceUInt error_code)

{
    char *pcVar5;
    int iVar8;
    if (!out_buf) {
        return 0;
    }
    SceUInt *puVar7 = param_1->field9_0x10;
    if (param_1->error_codes_count == 0) {
        snprintf(out_buf, 0x10, "E-%08x");
        return 0;
    }
    int iVar6 = 0;
    if (*puVar7 != error_code) {
        do {
            iVar6 = iVar6 + 1;
            if (param_1->error_codes_count <= iVar6) {
                snprintf(out_buf, 0x10, "E-%08x");
                return 0;
            }
            puVar7 = puVar7 + 1;
        } while (*puVar7 != error_code);
        if (param_1->error_messages_count < iVar6) {
            snprintf(out_buf, 0x10, "*-%08x");
            return 0;
        }
    }
    SceUInt uVar4 = (error_code << 4) >> 0x14;
    if (uVar4 < 0x300) {
        // pcVar5 = "NS";
    } else {
        pcVar5 = (char *)param_1;
        if (param_1->field4_0x4 != 0) {
            iVar8 = 0;
            ushort *puVar2 = param_1->field6_0x8;
            do {
                iVar8 = iVar8 + 1;
                if ((*puVar2 <= uVar4) && (uVar4 < (uint)*puVar2 + (uint)puVar2[1])) {
                    pcVar5 = (char *)(puVar2 + 2);
                    break;
                }
                puVar2 = puVar2 + 4;
            } while (iVar8 < (int)(uint)param_1->field4_0x4);
        }
    }
    iVar6 = (uint)param_1->field7_0xc + iVar6;
    if (iVar6 == 0) {
        iVar8 = 0;
    } else {
        int iVar1;
        iVar8 = 0;
        int iVar3 = iVar6;
        do {
            iVar1 = iVar3 / 10;
            iVar8 = iVar8 + iVar3 % 10;
            iVar3 = iVar1;
        } while (iVar1 != 0);
        iVar8 = iVar8 % 10;
    }
    snprintf(out_buf, 0x10, "%s-%d-%1d", pcVar5, iVar6, iVar8);
    return 0;
}
*/
EXPORT(SceInt32, sceErrorGetExternalString, char *result, uint32_t err) {
    TRACY_FUNC(sceErrorGetExternalString, result, err);
    return CALL_EXPORT(_sceErrorGetExternalString, result, err);
}

EXPORT(int, sceErrorHistoryClearError) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceErrorHistoryGetError) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceErrorHistoryPostError) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceErrorHistorySetDefaultFormat) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceErrorHistoryUpdateSequenceInfo) {
    return UNIMPLEMENTED();
}
