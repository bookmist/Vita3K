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

#include <module/module.h>

#include "modules/module_parent.h"

#include <util/tracy.h>

TRACY_MODULE_NAME(SceIpmi);

namespace IPMI {
struct DataInfo {
    Ptr<void> pBuffer;
    SceSize bufferSize;
};
struct BufferInfo {
    Ptr<void> pBuffer;
    SceSize bufferSize;
    SceSize bufferWrittenSize; // size written by method
};
struct Client {
    struct EventNotifee {
        unsigned int data;
    };
};
} // namespace IPMI

// IPMI::Client::disconnect()
EXPORT(int, _ZN4IPMI6Client10disconnectEv) {
    return UNIMPLEMENTED();
}

// IPMI::Client::getUserData()
EXPORT(int, _ZN4IPMI6Client11getUserDataEv) {
    return UNIMPLEMENTED();
}

// IPMI::Client::tryGetResult(unsigned int, int*, void*, unsigned long*, unsigned long)
EXPORT(int, _ZN4IPMI6Client12tryGetResultEjPiPvPmm, IPMI::Client *self, unsigned int a1, int *a2, void *a3, unsigned long *a4, unsigned long a5) {
    return UNIMPLEMENTED();
}

// IPMI::Client::tryGetResult(unsigned int, unsigned int, int*, IPMI::BufferInfo*, unsigned int)
EXPORT(int, _ZN4IPMI6Client12tryGetResultEjjPiPNS_10BufferInfoEj, IPMI::Client *self,
    unsigned int, unsigned int, int *, IPMI::BufferInfo *, unsigned int) {
    return UNIMPLEMENTED();
}

// IPMI::Client::pollEventFlag(unsigned int, unsigned int, unsigned int, unsigned int*)
EXPORT(int, _ZN4IPMI6Client13pollEventFlagEjjjPj, IPMI::Client *self, unsigned int, unsigned int, unsigned int, unsigned int *) {
    return UNIMPLEMENTED();
}

// IPMI::Client::waitEventFlag(unsigned int, unsigned int, unsigned int, unsigned int*, unsigned int*)
EXPORT(int, _ZN4IPMI6Client13waitEventFlagEjjjPjS1_, IPMI::Client *self, unsigned int, unsigned int, unsigned int, unsigned int *, unsigned int *) {
    return UNIMPLEMENTED();
}

// IPMI::Client::invokeSyncMethod(unsigned int, IPMI::DataInfo const*, unsigned int, int*, IPMI::BufferInfo*, unsigned int)
EXPORT(int, _ZN4IPMI6Client16invokeSyncMethodEjPKNS_8DataInfoEjPiPNS_10BufferInfoEj,
    IPMI::Client *self, unsigned int method_code, IPMI::DataInfo const *data_info, unsigned int data_info_size, int *result, IPMI::BufferInfo *buffer_info, unsigned int buffer_info_size) {
    TRACY_FUNC(_ZN4IPMI6Client16invokeSyncMethodEjPKNS_8DataInfoEjPiPNS_10BufferInfoEj, self, method_code, data_info, data_info_size, result, buffer_info, buffer_info_size);
    LOG_TRACE("IPMI::Client::invokeSyncMethod({}, {}, {}, {}, {}, {})", log_hex(method_code), uint64_t(data_info), data_info_size, uint64_t(result), uint64_t(buffer_info), buffer_info_size);
    auto params = data_info ? *data_info->pBuffer.cast<int>().get(emuenv.mem) : 0;
    LOG_CONSOLE(_ZN4IPMI6Client16invokeSyncMethodEjPKNS_8DataInfoEjPiPNS_10BufferInfoEj, self, log_hex(method_code), params);
    if (result)
        *result = 0;
    if (buffer_info && buffer_info->pBuffer && buffer_info_size > 0) {
        memset(buffer_info->pBuffer.get(emuenv.mem), 0, buffer_info->bufferSize);
        buffer_info->bufferWrittenSize = buffer_info->bufferSize;
    }
    return UNIMPLEMENTED();
}

// IPMI::Client::invokeSyncMethod(unsigned int, void const*, unsigned int, int*, void*, unsigned int*, unsigned int)
EXPORT(int, _ZN4IPMI6Client16invokeSyncMethodEjPKvjPiPvPjj,
    IPMI::Client *self, unsigned int method_code, void const *data_info, unsigned int data_info_size, int *result, void *buffer_info, unsigned int *buffer_info_size, unsigned int) {
    return UNIMPLEMENTED();
}

// IPMI::Client::invokeAsyncMethod(unsigned int, IPMI::DataInfo const*, unsigned int, unsigned int*, IPMI::Client::EventNotifee const*)
EXPORT(int, _ZN4IPMI6Client17invokeAsyncMethodEjPKNS_8DataInfoEjPjPKNS0_12EventNotifeeE, IPMI::Client *self, int, IPMI::DataInfo const *, unsigned int, unsigned int *, IPMI::Client::EventNotifee const *) {
    return UNIMPLEMENTED();
}

// IPMI::Client::invokeAsyncMethod(unsigned int, void const*, unsigned int, int*, IPMI::Client::EventNotifee const*)
EXPORT(int, _ZN4IPMI6Client17invokeAsyncMethodEjPKvjPiPKNS0_12EventNotifeeE, IPMI::Client *self, unsigned int, void const *, unsigned int, int *, IPMI::Client::EventNotifee const *) {
    return UNIMPLEMENTED();
}

// IPMI::Client::terminateConnection()
EXPORT(int, _ZN4IPMI6Client19terminateConnectionEv, IPMI::Client *self) {
    return UNIMPLEMENTED();
}

// IPMI::Client::Config::estimateClientMemorySize()
EXPORT(int, _ZN4IPMI6Client6Config24estimateClientMemorySizeEv, IPMI::Client *self) {
    TRACY_FUNC(_ZN4IPMI6Client6Config24estimateClientMemorySizeEv);
    STUBBED("stubbed");
    return 0x100;
}

// IPMI::Client::create(IPMI::Client**, IPMI::Client::Config const*, void*, void*)
EXPORT(int, _ZN4IPMI6Client6createEPPS0_PKNS0_6ConfigEPvS6_, Ptr<void> *client, void const *config, Ptr<void> user_data, Ptr<void> client_memory) {
    TRACY_FUNC(_ZN4IPMI6Client6createEPPS0_PKNS0_6ConfigEPvS6_, client, config, user_data, client_memory);
    *client_memory.cast<Ptr<void>>().get(emuenv.mem) = get_client_vtable(emuenv.mem);
    *client = client_memory;
    STUBBED("Stubed");
    return 0;
}

// IPMI::Client::getMsg(unsigned int, void*, unsigned int*, unsigned int, unsigned int*)
EXPORT(int, _ZN4IPMI6Client6getMsgEjPvPjjS2_, IPMI::Client *self, unsigned int, void *, unsigned int *, unsigned int, unsigned int *) {
    return UNIMPLEMENTED();
}

// IPMI::Client::connect(void const*, unsigned int, int*)
EXPORT(int, _ZN4IPMI6Client7connectEPKvjPi, IPMI::Client *self, void const *params, SceSize params_size, SceInt32 *error) {
    TRACY_FUNC(_ZN4IPMI6Client7connectEPKvjPi, self, params, params_size, error);
    *error = 0;
    return UNIMPLEMENTED();
}

// IPMI::Client::destroy()
EXPORT(int, _ZN4IPMI6Client7destroyEv, IPMI::Client *self) {
    return UNIMPLEMENTED();
}

// IPMI::Client::tryGetMsg(unsigned int, void*, unsigned long*, unsigned long)
EXPORT(int, _ZN4IPMI6Client9tryGetMsgEjPvPmm, IPMI::Client *self, unsigned int, void *, unsigned long *, unsigned long) {
    return UNIMPLEMENTED();
}

// IPMI::Client::~Client()
EXPORT(int, _ZN4IPMI6ClientD1Ev, IPMI::Client *self) {
    return UNIMPLEMENTED();
}

EXPORT(int, SceIpmi_296D44D4) {
    return UNIMPLEMENTED();
}

EXPORT(int, SceIpmi_BC3A3031) {
    return UNIMPLEMENTED();
}

EXPORT(int, SceIpmi_2C6DB642) {
    return UNIMPLEMENTED();
}

EXPORT(int, SceIpmi_006EFA9D) {
    return UNIMPLEMENTED();
}
