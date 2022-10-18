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

#include "SceMotion.h"

#include <util/tracy.h>
TRACY_MODULE_NAME(SceMotion);

EXPORT(int, sceMotionGetAngleThreshold) {
    TRACY_FUNC(sceMotionGetAngleThreshold);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetBasicOrientation, SceFVector3 *basicOrientation) {
    TRACY_FUNC(sceMotionGetBasicOrientation, basicOrientation);
    if (!basicOrientation)
        return RET_ERROR(SCE_MOTION_ERROR_NULL_PARAMETER);

    STUBBED("set default value");
    basicOrientation->x = 0;
    basicOrientation->y = 0;
    basicOrientation->z = 1;

    return 0;
}

EXPORT(int, sceMotionGetDeadband) {
    TRACY_FUNC(sceMotionGetDeadband);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetDeadbandExt) {
    TRACY_FUNC(sceMotionGetDeadbandExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetDeviceLocation) {
    TRACY_FUNC(sceMotionGetDeviceLocation);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetGyroBiasCorrection) {
    TRACY_FUNC(sceMotionGetGyroBiasCorrection);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetMagnetometerState) {
    TRACY_FUNC(sceMotionGetMagnetometerState);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetSensorState) {
    TRACY_FUNC(sceMotionGetSensorState);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetState) {
    TRACY_FUNC(sceMotionGetState);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetStateExt) {
    TRACY_FUNC(sceMotionGetStateExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetStateInternal) {
    TRACY_FUNC(sceMotionGetStateInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetTiltCorrection) {
    TRACY_FUNC(sceMotionGetTiltCorrection);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionGetTiltCorrectionExt) {
    TRACY_FUNC(sceMotionGetTiltCorrectionExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionInitLibraryExt) {
    TRACY_FUNC(sceMotionInitLibraryExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionMagnetometerOff) {
    TRACY_FUNC(sceMotionMagnetometerOff);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionMagnetometerOn) {
    TRACY_FUNC(sceMotionMagnetometerOn);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionReset) {
    TRACY_FUNC(sceMotionReset);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionResetExt) {
    TRACY_FUNC(sceMotionResetExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionRotateYaw) {
    TRACY_FUNC(sceMotionRotateYaw);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionSetAngleThreshold) {
    TRACY_FUNC(sceMotionSetAngleThreshold);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionSetDeadband) {
    TRACY_FUNC(sceMotionSetDeadband);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionSetDeadbandExt) {
    TRACY_FUNC(sceMotionSetDeadbandExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionSetGyroBiasCorrection) {
    TRACY_FUNC(sceMotionSetGyroBiasCorrection);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionSetTiltCorrection) {
    TRACY_FUNC(sceMotionSetTiltCorrection);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionSetTiltCorrectionExt) {
    TRACY_FUNC(sceMotionSetTiltCorrectionExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionStartSampling) {
    TRACY_FUNC(sceMotionStartSampling);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionStartSamplingExt) {
    TRACY_FUNC(sceMotionStartSamplingExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionStopSampling) {
    TRACY_FUNC(sceMotionStopSampling);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionStopSamplingExt) {
    TRACY_FUNC(sceMotionStopSamplingExt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionTermLibraryExt) {
    TRACY_FUNC(sceMotionTermLibraryExt);
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceMotionGetAngleThreshold)
BRIDGE_IMPL(sceMotionGetBasicOrientation)
BRIDGE_IMPL(sceMotionGetDeadband)
BRIDGE_IMPL(sceMotionGetDeadbandExt)
BRIDGE_IMPL(sceMotionGetDeviceLocation)
BRIDGE_IMPL(sceMotionGetGyroBiasCorrection)
BRIDGE_IMPL(sceMotionGetMagnetometerState)
BRIDGE_IMPL(sceMotionGetSensorState)
BRIDGE_IMPL(sceMotionGetState)
BRIDGE_IMPL(sceMotionGetStateExt)
BRIDGE_IMPL(sceMotionGetStateInternal)
BRIDGE_IMPL(sceMotionGetTiltCorrection)
BRIDGE_IMPL(sceMotionGetTiltCorrectionExt)
BRIDGE_IMPL(sceMotionInitLibraryExt)
BRIDGE_IMPL(sceMotionMagnetometerOff)
BRIDGE_IMPL(sceMotionMagnetometerOn)
BRIDGE_IMPL(sceMotionReset)
BRIDGE_IMPL(sceMotionResetExt)
BRIDGE_IMPL(sceMotionRotateYaw)
BRIDGE_IMPL(sceMotionSetAngleThreshold)
BRIDGE_IMPL(sceMotionSetDeadband)
BRIDGE_IMPL(sceMotionSetDeadbandExt)
BRIDGE_IMPL(sceMotionSetGyroBiasCorrection)
BRIDGE_IMPL(sceMotionSetTiltCorrection)
BRIDGE_IMPL(sceMotionSetTiltCorrectionExt)
BRIDGE_IMPL(sceMotionStartSampling)
BRIDGE_IMPL(sceMotionStartSamplingExt)
BRIDGE_IMPL(sceMotionStopSampling)
BRIDGE_IMPL(sceMotionStopSamplingExt)
BRIDGE_IMPL(sceMotionTermLibraryExt)
