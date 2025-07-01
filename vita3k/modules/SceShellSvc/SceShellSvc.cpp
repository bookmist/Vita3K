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
#include <modules/module_parent.h>

#include <util/tracy.h>

TRACY_MODULE_NAME(SceShellSvc)

EXPORT(int, sceShellSvcGetSvcObj) {
    TRACY_FUNC(sceShellSvcGetSvcObj);
    static Ptr<Address> svc_client;
    if (!svc_client) {
        svc_client = Ptr<Address>(alloc(emuenv.mem, sizeof(int), "ShellSvc"));
        *svc_client.get(emuenv.mem) = get_client_vtable(emuenv.mem).address();
    }

    STUBBED("STUBBED");
    return svc_client.address();
}

EXPORT(SceInt32, sceShellSvcInitialize) {
    TRACY_FUNC(sceShellSvcInitialize);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcRegisterCallback, SceUInt32 callbackId, Ptr<SceInt32(SceUInt32, ScePVoid)> pCallback, ScePVoid userData) {
    TRACY_FUNC(sceShellSvcRegisterCallback, callbackId, pCallback, userData);
    LOG_DEBUG("sceShellSvcRegisterCallback: callbackId: {}, pCallback: {}, userData: {}", callbackId, pCallback, userData);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcPeekClearEventFlag, SceUInt32 flagId, bool *pWasCancelled) {
    TRACY_FUNC(sceShellSvcPeekClearEventFlag, flagId, pWasCancelled);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcGetEventFlagSmth, SceUInt32 *info, SceInt32 flagId) {
    TRACY_FUNC(sceShellSvcGetEventFlagSmth, info, flagId);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcWaitEventFlag, SceUInt32 flagId) {
    TRACY_FUNC(sceShellSvcWaitEventFlag, flagId);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcUnregisterCallback, SceUInt32 callbackId) {
    TRACY_FUNC(sceShellSvcUnregisterCallback, callbackId);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcCheckCallback, SceUInt32 callbackId) {
    TRACY_FUNC(sceShellSvcCheckCallback, callbackId);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcClearEventFlag, SceInt32 flagId) {
    TRACY_FUNC(sceShellSvcClearEventFlag, flagId);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceShellSvcCancelCallback, SceInt32 callbackId) {
    TRACY_FUNC(sceShellSvcCancelCallback, callbackId);
    return UNIMPLEMENTED();
}
