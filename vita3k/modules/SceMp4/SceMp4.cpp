// Vita3K emulator project
// Copyright (C) 2018 Vita3K team
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

#include "SceMp4.h"

#include <io/functions.h>
#include <kernel/thread/thread_functions.h>
#include <util/lock_and_find.h>
#include <util/log.h>

#include <algorithm>
#include <filesystem>
#include <fileapi.h>

EXPORT(int, SceMp4OpenFile, Ptr<const char> file_name, SceAvPlayerFileManager *file_manager, void *param_3,
    void *memory_base, void *capacity, void *param_6, uint log_level) {

    LOG_DEBUG("file name: {}", file_name.get(host.mem));

    return 10;
}

EXPORT(int, SceMp4Init, uint32_t param_1, uint32_t param_2, uint64_t *length ) {
    LOG_DEBUG("param_1: {}, param_2: {}, param_3: {}", param_1, param_2, log_hex(*length));
    *length = 1;
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(SceMp4OpenFile)
BRIDGE_IMPL(SceMp4Init)

