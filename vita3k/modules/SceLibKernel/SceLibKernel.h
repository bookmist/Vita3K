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

DECL_EXPORT(SceInt32, sceKernelGetThreadCurrentPriority);

DECL_EXPORT(Ptr<void>, sceClibMspaceCalloc, Ptr<void> space, uint32_t elements, uint32_t size);

DECL_EXPORT(Ptr<void>, sceClibMspaceCreate, Ptr<void> base, uint32_t capacity);

DECL_EXPORT(uint32_t, sceClibMspaceDestroy, Ptr<void> space);

DECL_EXPORT(void, sceClibMspaceFree, Ptr<void> space, Ptr<void> address);

DECL_EXPORT(int, sceClibMspaceIsHeapEmpty);

DECL_EXPORT(Ptr<void>, sceClibMspaceMalloc, Ptr<void> space, uint32_t size);
