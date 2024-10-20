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

#pragma once

#include <module/module.h>

#include <modules/module_parent.h>

#include <kernel/state.h>
#include <kernel/types.h>

struct SceKernelAllocMemBlockOpt {
    SceSize size;
    SceUInt32 attr;
    SceSize alignment;
    SceUInt32 uidBaseBlock;
    const char *strBaseBlockName;
    int flags;
    int reserved[10];
};

DECL_EXPORT(SceUID, sceKernelFindMemBlockByAddr, Address addr, uint32_t size);
DECL_EXPORT(int, sceKernelFreeMemBlock, SceUID uid);
DECL_EXPORT(SceUID, sceKernelAllocMemBlock, const char *pName, SceKernelMemBlockType type, SceSize size, SceKernelAllocMemBlockOpt *optp);
DECL_EXPORT(int, sceKernelGetMemBlockBase, SceUID uid, Ptr<void> *basep);
