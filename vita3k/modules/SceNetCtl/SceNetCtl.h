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

#pragma once

#include <module/module.h>

BRIDGE_DECL(sceNetCtlAdhocDisconnect)
BRIDGE_DECL(sceNetCtlAdhocGetInAddr)
BRIDGE_DECL(sceNetCtlAdhocGetPeerList)
BRIDGE_DECL(sceNetCtlAdhocGetResult)
BRIDGE_DECL(sceNetCtlAdhocGetState)
BRIDGE_DECL(sceNetCtlAdhocRegisterCallback)
BRIDGE_DECL(sceNetCtlAdhocUnregisterCallback)
BRIDGE_DECL(sceNetCtlCheckCallback)
BRIDGE_DECL(sceNetCtlGetIfStat)
BRIDGE_DECL(sceNetCtlGetNatInfo)
BRIDGE_DECL(sceNetCtlGetPhoneMaxDownloadableSize)
BRIDGE_DECL(sceNetCtlInetGetInfo)
BRIDGE_DECL(sceNetCtlInetGetResult)
BRIDGE_DECL(sceNetCtlInetGetState)
BRIDGE_DECL(sceNetCtlInetRegisterCallback)
BRIDGE_DECL(sceNetCtlInetUnregisterCallback)
BRIDGE_DECL(sceNetCtlInit)
BRIDGE_DECL(sceNetCtlTerm)
BRIDGE_DECL(SceNetCtl_229EAA64)
BRIDGE_DECL(SceNetCtl_AC326DB7)
BRIDGE_DECL(SceNetCtl_C61F3E96)
BRIDGE_DECL(SceNetCtl_C33D7374)
