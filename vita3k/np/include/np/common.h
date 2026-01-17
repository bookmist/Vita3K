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

#pragma once

#include <mem/ptr.h>
#include <util/types.h>

#include <cstdint>
#include <cstring>

namespace np {

#define SCE_NP_ONLINEID_MIN_LENGTH 3
#define SCE_NP_ONLINEID_MAX_LENGTH 16

struct SceNpOnlineId {
    char data[SCE_NP_ONLINEID_MAX_LENGTH];
    char term;
    char dummy[3];
};
static_assert(sizeof(SceNpOnlineId) == 0x14, "SceNpOnlineId is an invalid size");

struct SceNpIdOptParam {
    SceUChar8 unknown[4];
    char platformType[4];
};
static_assert(sizeof(SceNpIdOptParam) == 0x8, "SceNpIdOptParam is an invalid size");

struct SceNpId {
    SceNpOnlineId handle;
    SceNpIdOptParam opt;
    SceInt8 isIdValid;
    SceUChar8 reserved[7];
};
static_assert(sizeof(SceNpId) == 0x24, "SceNpId is an invalid size");

struct CommunicationID {
    char data[9];
    char term;
    std::uint8_t num;
    char dummy;
};

inline bool operator==(const CommunicationID &lhs, const CommunicationID &rhs) {
    return (strncmp(lhs.data, rhs.data, 9) == 0) && (lhs.num == rhs.num);
}

struct CommunicationConfig {
    Ptr<CommunicationID> comm_id;
    Ptr<void> unk0;
    Ptr<void> unk1;
};

constexpr auto SCE_NP_TROPHY_INVALID_TROPHY_ID = -1;
} // namespace np

enum SceNpTrophyErrorCode : uint32_t {
    SCE_NP_TROPHY_ERROR_NONE = 0,
    SCE_NP_TROPHY_ERROR_UNKNOWN = 0x80551600,
    SCE_NP_TROPHY_ERROR_NOT_INITIALIZED = 0x80551601,
    SCE_NP_TROPHY_ERROR_ALREADY_INITIALIZED = 0x80551602,
    SCE_NP_TROPHY_ERROR_NO_MEMORY = 0x80551603,
    SCE_NP_TROPHY_ERROR_INVALID_ARGUMENT = 0x80551604,
    SCE_NP_TROPHY_ERROR_INSUFFICIENT_BUFFER = 0x80551605,
    SCE_NP_TROPHY_ERROR_EXCEEDS_MAX = 0x80551606,
    SCE_NP_TROPHY_ERROR_ABORT = 0x80551607,
    SCE_NP_TROPHY_ERROR_INVALID_HANDLE = 0x80551608,
    SCE_NP_TROPHY_ERROR_INVALID_CONTEXT = 0x80551609,
    SCE_NP_TROPHY_ERROR_INVALID_NPCOMMID = 0x8055160a,
    SCE_NP_TROPHY_ERROR_INVALID_NPCOMMSIGN = 0x8055160b,
    SCE_NP_TROPHY_ERROR_NPCOMMSIGN_VERIFICATION_FAILURE = 0x8055160c,
    SCE_NP_TROPHY_ERROR_INVALID_GROUP_ID = 0x8055160d,
    SCE_NP_TROPHY_ERROR_INVALID_TROPHY_ID = 0x8055160e,
    SCE_NP_TROPHY_ERROR_TROPHY_ALREADY_UNLOCKED = 0x8055160f,
    SCE_NP_TROPHY_ERROR_PLATINUM_CANNOT_UNLOCK = 0x80551610,
    SCE_NP_TROPHY_ERROR_ACCOUNTID_NOT_MATCH = 0x80551611,
    SCE_NP_TROPHY_ERROR_SETUP_REQUIRED = 0x80551612,
    SCE_NP_TROPHY_ERROR_ALREADY_SETUP = 0x80551613,
    SCE_NP_TROPHY_ERROR_BROKEN_DATA = 0x80551614,
    SCE_NP_TROPHY_ERROR_INSUFFICIENT_EM_SPACE = 0x80551615,
    SCE_NP_TROPHY_ERROR_CONTEXT_ALREADY_EXISTS = 0x80551616,
    SCE_NP_TROPHY_ERROR_TRP_FILE_VERIFICATION_FAILURE = 0x80551617,
    SCE_NP_TROPHY_ERROR_ICON_FILE_NOT_FOUND = 0x80551618,
    SCE_NP_TROPHY_ERROR_TRP_FILE_NOT_FOUND = 0x80551619,
    SCE_NP_TROPHY_ERROR_INVALID_TRP_FILE_FORMAT = 0x8055161a,
    SCE_NP_TROPHY_ERROR_UNSUPPORTED_TRP_FILE = 0x8055161b,
    SCE_NP_TROPHY_ERROR_INVALID_TROPHY_CONF_FORMAT = 0x8055161c,
    SCE_NP_TROPHY_ERROR_UNSUPPORTED_TROPHY_CONF = 0x8055161d,
    SCE_NP_TROPHY_ERROR_TROPHY_NOT_UNLOCKED = 0x8055161e,
    SCE_NP_TROPHY_ERROR_UNLOCK_DENIED = 0x8055161f,
    SCE_NP_TROPHY_ERROR_INSUFFICIENT_MC_SPACE = 0x80551620,
    SCE_NP_TROPHY_ERROR_DEBUG_FAILURE = 0x80551621
};
