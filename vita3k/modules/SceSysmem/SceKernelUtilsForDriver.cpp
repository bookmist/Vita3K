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

#include <module/module.h>
#include <util/tracy.h>

TRACY_MODULE_NAME(SceKernelUtilsForDriver);

static uint32_t ksceMt19937UInt(uint32_t *pContext)

{
    uint32_t uVar2 = *pContext;
    uint32_t uVar3 = uVar2 + 1;
    if (uVar2 == 0x26f) {
        uVar3 = 0;
    }
    uint32_t uVar1 = pContext[uVar2 + 1];
    *pContext = uVar3;
    int iVar4 = uVar2 - 0xe3;
    if ((int)uVar2 < 0xe3) {
        iVar4 = uVar2 + 0x18d;
    }
    uint32_t uVar5 = pContext[iVar4 + 1] ^ (uVar1 & 0x80000000 | pContext[uVar3 + 1] & 0x7fffffff) >> 1;
    uVar1 = uVar1 ^ uVar1 >> 0xb;
    if ((pContext[uVar3 + 1] & 1) != 0) {
        uVar5 = uVar5 ^ 0x9908b0df;
    }
    pContext[uVar2 + 1] = uVar5;
    uVar1 = uVar1 ^ (uVar1 & 0x13a58ad) << 7;
    uVar1 = uVar1 ^ (uVar2 & 0xffff | 0xefc60000) & uVar1 << 0xf;
    return uVar1 ^ uVar1 >> 0x12;
}

static SceInt32 ksceMt19937Init(uint32_t *pContext, SceUInt32 seed)

{
    int iVar1 = 1;
    pContext[1] = seed;
    uint32_t *puVar2 = pContext + 2;
    while (true) {
        uint32_t uVar3 = (seed ^ seed >> 0x1e) * 0x6c078965 + iVar1;
        iVar1 = iVar1 + 1;
        *puVar2 = uVar3;
        if (puVar2 + 1 == pContext + 0x271)
            break;
        seed = *puVar2;
        puVar2 = puVar2 + 1;
    }
    *pContext = 0;
    for (int i = 0; i < 0x270; i++) {
        ksceMt19937UInt(pContext);
    }
    return 0;
}

EXPORT(SceUInt32, sceMt19937InitForDriver, uint32_t *pContext, SceUInt32 seed) {
    TRACY_FUNC(sceMt19937InitForDriver, pContext, seed);
    return ksceMt19937Init(pContext, seed);
}
EXPORT(SceUInt32, sceMt19937UIntForDriver, uint32_t *pContext) {
    TRACY_FUNC(sceMt19937UIntForDriver, pContext);
    return ksceMt19937UInt(pContext);
}
