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

#include <module/module.h>

#include <util/tracy.h>
TRACY_MODULE_NAME(SceAudioencUser);

EXPORT(int, sceAudioencClearContext) {
    return UNIMPLEMENTED();
}

typedef struct SceAudioencInfoCelp {
    SceUInt32 size;
    SceUInt32 excitationMode;
    SceUInt32 samplingRate;
    SceUInt32 bitRate;
} SceAudioencInfoCelp;

typedef union SceAudioencInfo {
    SceUInt32 size;
    SceAudioencInfoCelp celp;
} SceAudioencInfo;

typedef struct SceAudioencOptInfoCelp {
    SceUInt32 size;
    SceUInt8 header[32];
    SceUInt32 headerSize;
    SceUInt32 encoderVersion;
} SceAudioencOptInfoCelp;

typedef union SceAudioencOptInfo {
    SceUInt32 size;
    SceAudioencOptInfoCelp celp;
} SceAudioencOptInfo;

typedef struct SceAudioencCtrl {
    SceUInt32 size;
    SceInt32 handle;
    SceUInt8 *pInputPcm;
    SceUInt32 inputPcmSize;
    SceUInt32 maxPcmSize;
    void *pOutputEs;
    SceUInt32 outputEsSize;
    SceUInt32 maxEsSize;
    SceUInt32 wordLength;
    SceAudioencInfo *pInfo;
    SceAudioencOptInfo *pOptInfo;
} SceAudioencCtrl;

EXPORT(int, sceAudioencCreateEncoder, SceAudioencCtrl *pCtrl, SceUInt32 codecType) {
    TRACY_FUNC(sceAudioencCreateEncoder, pCtrl, codecType);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencCreateEncoderExternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencCreateEncoderResident) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencDeleteEncoder, SceAudioencCtrl *pCtrl) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencDeleteEncoderExternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencDeleteEncoderResident) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencEncode, SceAudioencCtrl *pCtrl) {
    TRACY_FUNC(sceAudioencEncode, pCtrl);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencEncodeNFrames) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencGetContextSize) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencGetInternalError) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencGetOptInfo) {
    return UNIMPLEMENTED();
}

struct SceAudioencInitStreamParam {
    SceUInt32 size;
    SceUInt32 totalStreams;
};

union SceAudioencInitParam {
    SceUInt32 size;
    SceAudioencInitStreamParam celp;
};

EXPORT(int, sceAudioencInitLibrary, SceUInt32 codecType, SceAudioencInitParam *pInitParam) {
    TRACY_FUNC(sceAudioencInitLibrary, codecType, pInitParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAudioencTermLibrary) {
    return UNIMPLEMENTED();
}
