// Vita3K emulator project
// Copyright (C) 2021 Vita3K team
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

EXPORT(int, sceMp4CloseFile, SceUID player_handle) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4EnableStream, SceUID player_handle, int32_t stream_no, bool state) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4GetNextUnit, SceUID player_handle, uint32_t param_2) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4GetNextUnit2Ref, SceUID player_handle, uint32_t *param_2) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4GetNextUnit3Ref, SceUID player_handle, uint32_t *param_2) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4GetNextUnitData, SceUID player_handle, void *param_2) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4GetStreamInfo, SceUID player_handle, int32_t stream_no, uint32_t *stream_info) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4JumpPTS) {
    return UNIMPLEMENTED();
}

struct SceAvPlayerFileManager {
    uint32_t user_data = 0;

    // Cast to SceAvPlayerOpenFile, SceAvPlayerCloseFile, SceAvPlayerReadFile and SceAvPlayerGetFileSize.
    Ptr<void> open_file;
    Ptr<void> close_file;
    Ptr<void> read_file;
    Ptr<void> file_size;
};

EXPORT(int, sceMp4OpenFile, char *file_name, SceAvPlayerFileManager *file_manager, int32_t param_3, int32_t memory_base, int32_t capacity, int32_t param_6, int32_t log_level) {
    STUBBED("Fake Handle");
    LOG_DEBUG("file_name: {}", file_name);
    return 13;
}

EXPORT(SceInt64, sceMp4PTSToTime, SceInt64 pts) {
    return UNIMPLEMENTED();
}

EXPORT(void, sceMp4ReleaseBuffer, SceUID player_handle, uint32_t *buffer) {
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
}

EXPORT(int, sceMp4Reset, SceUID player_handle) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceMp4StartFileStreaming, SceUID player_handle, uint32_t param_2, SceInt64 *result) {
    return UNIMPLEMENTED();
}

EXPORT(SceInt64, sceMp4TimeToPTS, SceInt64 Time) {
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceMp4CloseFile)
BRIDGE_IMPL(sceMp4EnableStream)
BRIDGE_IMPL(sceMp4GetNextUnit)
BRIDGE_IMPL(sceMp4GetNextUnit2Ref)
BRIDGE_IMPL(sceMp4GetNextUnit3Ref)
BRIDGE_IMPL(sceMp4GetNextUnitData)
BRIDGE_IMPL(sceMp4GetStreamInfo)
BRIDGE_IMPL(sceMp4JumpPTS)
BRIDGE_IMPL(sceMp4OpenFile)
BRIDGE_IMPL(sceMp4PTSToTime)
BRIDGE_IMPL(sceMp4ReleaseBuffer)
BRIDGE_IMPL(sceMp4Reset)
BRIDGE_IMPL(sceMp4StartFileStreaming)
BRIDGE_IMPL(sceMp4TimeToPTS)
