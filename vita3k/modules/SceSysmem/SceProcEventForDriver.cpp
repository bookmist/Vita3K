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

#include "kernel/state.h"

#include <module/module.h>

EXPORT(int, ksceKernelInvokeProcEventHandler) {
    return UNIMPLEMENTED();
}

#define VITASDK_BUILD_ASSERT_EQ(a, b)

typedef struct SceProcEventInvokeParam1 {
    SceSize size; // SceProcEventInvokeParam1 struct size : 0x10
    int unk_0x04;
    int unk_0x08;
    int unk_0x0C;
} SceProcEventInvokeParam1;
VITASDK_BUILD_ASSERT_EQ(0x10, SceProcEventInvokeParam1);

typedef struct SceProcEventInvokeParam2 {
    SceSize size; // SceProcEventInvokeParam2 struct size : 0x14
    SceUID pid;
    int unk_0x08;
    int unk_0x0C;
    int unk_0x10;
} SceProcEventInvokeParam2;
VITASDK_BUILD_ASSERT_EQ(0x14, SceProcEventInvokeParam2);

typedef struct SceProcEventHandler {
    SceSize size; // SceProcEventHandler struct size : 0x1C
    Ptr<void> create; // int (* create)(SceUID pid, SceProcEventInvokeParam2 *a2, int a3);
    Ptr<void> exit; // int (* exit)(SceUID pid, SceProcEventInvokeParam1 *a2, int a3);                             // current process exit
    Ptr<void> kill; // int (* kill)(SceUID pid, SceProcEventInvokeParam1 *a2, int a3);                             // by SceShell
    Ptr<void> stop; // int (* stop)(SceUID pid, int event_type, SceProcEventInvokeParam1 *a3, int a4);
    Ptr<void> start; // int (* start)(SceUID pid, int event_type, SceProcEventInvokeParam1 *a3, int a4);
    Ptr<void> switch_process; // int (* switch_process)(int event_id, int event_type, SceProcEventInvokeParam2 *a3, int a4); // switch display frame?
} SceProcEventHandler;
VITASDK_BUILD_ASSERT_EQ(0x1C, SceProcEventHandler);

EXPORT(int, ksceKernelRegisterProcEventHandler, char *name, SceProcEventHandler *handler, Ptr<void> a3) {
    emuenv.kernel.get_thread(thread_id)->run_callback(handler->create.address(), { 1, 0, a3.address() });
    emuenv.kernel.get_thread(thread_id)->run_callback(handler->start.address(), { 1, 0, a3.address() });
    return UNIMPLEMENTED();
}

EXPORT(int, ksceKernelUnregisterProcEventHandler) {
    return UNIMPLEMENTED();
}
