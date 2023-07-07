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

#include "SceNetInternal.h"

#include "../SceNet/SceNet.h"
#include "net/types.h"

#include <v3kprintf.h>

#include <kernel/state.h>
#include <util/lock_and_find.h>
#include <util/tracy.h>
TRACY_MODULE_NAME(SceNetInternal);

EXPORT(int, sceNetInternalInetPton, int af, const char *src, void *dst) {
    TRACY_FUNC(sceNetInternalInetPton, af, src, dst);
    return CALL_EXPORT(sceNetInetPton, af, src, dst);
}

EXPORT(int, sceNetInternalIcmConnect, int sid, int flags) {
    TRACY_FUNC(sceNetInternalIcmConnect, sid, flags);
    // call sceNetSyscallIcmConnect(sid, flags)
    return UNIMPLEMENTED();
}

EXPORT(int, SceNetInternal_8157DE3E) {
    TRACY_FUNC(SceNetInternal_8157DE3E);
    STUBBED("always error");
    return RET_ERROR(SCE_NET_ERROR_EINVAL);
    // return UNIMPLEMENTED();
}

EXPORT(int, SceNetInternalPrintf, const char *fmt, module::vargs args) {
    TRACY_FUNC(SceNetInternalPrintf, fmt);
    STUBBED("printf");
    std::vector<char> buffer(KiB(1));

    const ThreadStatePtr thread = lock_and_find(thread_id, emuenv.kernel.threads, emuenv.kernel.mutex);

    if (!thread) {
        return SCE_KERNEL_ERROR_UNKNOWN_THREAD_ID;
    }

    const int result = utils::snprintf(buffer.data(), buffer.size(), fmt, *(thread->cpu), emuenv.mem, args);

    if (!result) {
        return SCE_KERNEL_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("{}", buffer.data());

    return SCE_KERNEL_OK;
}

EXPORT(int, SceNetInternalIoctl, int s, uint32_t com, void *data) {
    TRACY_FUNC(SceNetInternalIoctl, s, com, data);
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceNetInternalIcmConnect)
BRIDGE_IMPL(sceNetInternalInetPton)
BRIDGE_IMPL(SceNetInternal_8157DE3E)
BRIDGE_IMPL(SceNetInternalPrintf)
BRIDGE_IMPL(SceNetInternalIoctl)
