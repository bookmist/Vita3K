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

#include "SceMotionDev.h"

#include <util/tracy.h>

TRACY_MODULE_NAME(SceMotionDev);

EXPORT(int, sceMotionDevGetAccCalibData, void *data) {
    TRACY_FUNC(sceMotionDevGetAccCalibData, data);
    memset(data, 0, 96);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetAccCalibData2) {
    TRACY_FUNC(sceMotionDevGetAccCalibData2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetCalibrationData, SceUInt32 block_id, void *data, SceUInt32 data_num) {
    TRACY_FUNC(sceMotionDevGetCalibrationData, block_id, data, data_num);
    if ((1 < block_id) || !data) {
        return 0x80360001;
    }
    memset(data, 0, data_num * 24);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetCalibrationHeader, SceUInt32 block_id, void *calib_header) {
    TRACY_FUNC(sceMotionDevGetCalibrationHeader, block_id, calib_header);
    if ((1 < block_id) || !calib_header) {
        return 0x80360001;
    }
    memset(calib_header, 0, 3 * 4);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetControllerType, SceUInt32 block_id, uint32_t *data) {
    TRACY_FUNC(sceMotionDevGetControllerType, block_id, data);
    *data = 1;
    return 0;
}

EXPORT(int, sceMotionDevGetCurrentMagnCalibData, void *data) {
    TRACY_FUNC(sceMotionDevGetCurrentMagnCalibData, data);
    memset(data, 0, 52);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetCurrentMagnStabilityLevel, SceUInt32 *level) {
    TRACY_FUNC(sceMotionDevGetCurrentMagnStabilityLevel, level);
    STUBBED("SCE_MOTION_MAGNETIC_FIELD_VERYSTABLE");
    *level = 2; // SCE_MOTION_MAGNETIC_FIELD_VERYSTABLE
    return 0;
}

EXPORT(int, sceMotionDevGetDeviceInfo, void *device_info) {
    TRACY_FUNC(sceMotionDevGetDeviceInfo, device_info);
    memset(device_info, 0, 44);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetDeviceLocation, void *location) {
    TRACY_FUNC(sceMotionDevGetDeviceLocation, location);
    memset(location, 0, 48);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetDs3CalibData) {
    TRACY_FUNC(sceMotionDevGetDs3CalibData);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetEvaInfo, void *data) {
    TRACY_FUNC(sceMotionDevGetEvaInfo, data);
    memset(data, 0, 72);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetFactoryMagnCalibData, void *data) {
    TRACY_FUNC(sceMotionDevGetFactoryMagnCalibData, data);
    memset(data, 0, 52);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetGyroBias, void *bias) {
    TRACY_FUNC(sceMotionDevGetGyroBias, bias);
    memset(bias, 0, 16);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetGyroBias2) {
    TRACY_FUNC(sceMotionDevGetGyroBias2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetGyroCalibData, void *data) {
    TRACY_FUNC(sceMotionDevGetGyroCalibData, data);
    memset(data, 0, 108);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetGyroCalibData2) {
    TRACY_FUNC(sceMotionDevGetGyroCalibData2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevGetMeasMode, void *mode_info) {
    TRACY_FUNC(sceMotionDevGetMeasMode, mode_info);
    memset(mode_info, 0, 8);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevIsReady) {
    TRACY_FUNC(sceMotionDevIsReady);
    STUBBED("SCE_TRUE");
    return SCE_TRUE; // UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevMagnSamplingStart) {
    TRACY_FUNC(sceMotionDevMagnSamplingStart);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevMagnSamplingStop) {
    TRACY_FUNC(sceMotionDevMagnSamplingStop);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevRead) {
    TRACY_FUNC(sceMotionDevRead);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevRead2) {
    TRACY_FUNC(sceMotionDevRead2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevRead3) {
    TRACY_FUNC(sceMotionDevRead3);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevReadForMagnCalib) {
    TRACY_FUNC(sceMotionDevReadForMagnCalib);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevSamplingStart) {
    TRACY_FUNC(sceMotionDevSamplingStart);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevSamplingStart2) {
    TRACY_FUNC(sceMotionDevSamplingStart2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevSamplingStop) {
    TRACY_FUNC(sceMotionDevSamplingStop);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevSamplingStop2) {
    TRACY_FUNC(sceMotionDevSamplingStop2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevSetGyroFeedBack) {
    TRACY_FUNC(sceMotionDevSetGyroFeedBack);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevSetSamplingMode) {
    TRACY_FUNC(sceMotionDevSetSamplingMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevUpdateMagnCalibData) {
    TRACY_FUNC(sceMotionDevUpdateMagnCalibData);
    return UNIMPLEMENTED();
}

EXPORT(int, sceMotionDevUpdateMagnStabilityLevel) {
    TRACY_FUNC(sceMotionDevUpdateMagnStabilityLevel);
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceMotionDevGetAccCalibData)
BRIDGE_IMPL(sceMotionDevGetAccCalibData2)
BRIDGE_IMPL(sceMotionDevGetCalibrationData)
BRIDGE_IMPL(sceMotionDevGetCalibrationHeader)
BRIDGE_IMPL(sceMotionDevGetControllerType)
BRIDGE_IMPL(sceMotionDevGetCurrentMagnCalibData)
BRIDGE_IMPL(sceMotionDevGetCurrentMagnStabilityLevel)
BRIDGE_IMPL(sceMotionDevGetDeviceInfo)
BRIDGE_IMPL(sceMotionDevGetDeviceLocation)
BRIDGE_IMPL(sceMotionDevGetDs3CalibData)
BRIDGE_IMPL(sceMotionDevGetEvaInfo)
BRIDGE_IMPL(sceMotionDevGetFactoryMagnCalibData)
BRIDGE_IMPL(sceMotionDevGetGyroBias)
BRIDGE_IMPL(sceMotionDevGetGyroBias2)
BRIDGE_IMPL(sceMotionDevGetGyroCalibData)
BRIDGE_IMPL(sceMotionDevGetGyroCalibData2)
BRIDGE_IMPL(sceMotionDevGetMeasMode)
BRIDGE_IMPL(sceMotionDevIsReady)
BRIDGE_IMPL(sceMotionDevMagnSamplingStart)
BRIDGE_IMPL(sceMotionDevMagnSamplingStop)
BRIDGE_IMPL(sceMotionDevRead)
BRIDGE_IMPL(sceMotionDevRead2)
BRIDGE_IMPL(sceMotionDevRead3)
BRIDGE_IMPL(sceMotionDevReadForMagnCalib)
BRIDGE_IMPL(sceMotionDevSamplingStart)
BRIDGE_IMPL(sceMotionDevSamplingStart2)
BRIDGE_IMPL(sceMotionDevSamplingStop)
BRIDGE_IMPL(sceMotionDevSamplingStop2)
BRIDGE_IMPL(sceMotionDevSetGyroFeedBack)
BRIDGE_IMPL(sceMotionDevSetSamplingMode)
BRIDGE_IMPL(sceMotionDevUpdateMagnCalibData)
BRIDGE_IMPL(sceMotionDevUpdateMagnStabilityLevel)
