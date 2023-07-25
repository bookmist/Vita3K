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

#include "SceCamera.h"

#include "camera/state.h"

#include <camera/camera.h>
#include <kernel/state.h>

#include <util/tracy.h>
TRACY_MODULE_NAME(SceCamera);

template <>
std::string to_debug_str<SceCameraDevice>(const MemState &mem, SceCameraDevice type) {
    switch (type) {
    case SCE_CAMERA_DEVICE_FRONT: return "SCE_CAMERA_DEVICE_FRONT";
    case SCE_CAMERA_DEVICE_BACK: return "SCE_CAMERA_DEVICE_BACK";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraPriority>(const MemState &mem, SceCameraPriority type) {
    switch (type) {
    case SCE_CAMERA_PRIORITY_SHARE: return "SCE_CAMERA_PRIORITY_SHARE";
    case SCE_CAMERA_PRIORITY_EXCLUSIVE: return "SCE_CAMERA_PRIORITY_EXCLUSIVE";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraFormat>(const MemState &mem, SceCameraFormat type) {
    switch (type) {
    case SCE_CAMERA_FORMAT_INVALID: return "SCE_CAMERA_FORMAT_INVALID";
    case SCE_CAMERA_FORMAT_YUV422_PLANE: return "SCE_CAMERA_FORMAT_YUV422_PLANE";
    case SCE_CAMERA_FORMAT_YUV422_PACKED: return "SCE_CAMERA_FORMAT_YUV422_PACKED";
    case SCE_CAMERA_FORMAT_YUV420_PLANE: return "SCE_CAMERA_FORMAT_YUV420_PLANE";
    case SCE_CAMERA_FORMAT_ARGB: return "SCE_CAMERA_FORMAT_ARGB";
    case SCE_CAMERA_FORMAT_ABGR: return "SCE_CAMERA_FORMAT_ABGR";
    case SCE_CAMERA_FORMAT_RAW8: return "SCE_CAMERA_FORMAT_RAW8";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraResolution>(const MemState &mem, SceCameraResolution type) {
    switch (type) {
    case SCE_CAMERA_RESOLUTION_0_0: return "SCE_CAMERA_RESOLUTION_0_0";
    case SCE_CAMERA_RESOLUTION_640_480: return "SCE_CAMERA_RESOLUTION_640_480";
    case SCE_CAMERA_RESOLUTION_320_240: return "SCE_CAMERA_RESOLUTION_320_240";
    case SCE_CAMERA_RESOLUTION_160_120: return "SCE_CAMERA_RESOLUTION_160_120";
    case SCE_CAMERA_RESOLUTION_352_288: return "SCE_CAMERA_RESOLUTION_352_288";
    case SCE_CAMERA_RESOLUTION_176_144: return "SCE_CAMERA_RESOLUTION_176_144";
    case SCE_CAMERA_RESOLUTION_480_272: return "SCE_CAMERA_RESOLUTION_480_272";
    case SCE_CAMERA_RESOLUTION_640_360: return "SCE_CAMERA_RESOLUTION_640_360";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraFrameRate>(const MemState &mem, SceCameraFrameRate type) {
    switch (type) {
    case SCE_CAMERA_FRAMERATE_3_FPS: return "SCE_CAMERA_FRAMERATE_3_FPS";
    case SCE_CAMERA_FRAMERATE_5_FPS: return "SCE_CAMERA_FRAMERATE_5_FPS";
    case SCE_CAMERA_FRAMERATE_7_FPS: return "SCE_CAMERA_FRAMERATE_7_FPS";
    case SCE_CAMERA_FRAMERATE_10_FPS: return "SCE_CAMERA_FRAMERATE_10_FPS";
    case SCE_CAMERA_FRAMERATE_15_FPS: return "SCE_CAMERA_FRAMERATE_15_FPS";
    case SCE_CAMERA_FRAMERATE_20_FPS: return "SCE_CAMERA_FRAMERATE_20_FPS";
    case SCE_CAMERA_FRAMERATE_30_FPS: return "SCE_CAMERA_FRAMERATE_30_FPS";
    case SCE_CAMERA_FRAMERATE_60_FPS: return "SCE_CAMERA_FRAMERATE_60_FPS";
    case SCE_CAMERA_FRAMERATE_120_FPS: return "SCE_CAMERA_FRAMERATE_120_FPS";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraExposureCompensation>(const MemState &mem, SceCameraExposureCompensation type) {
    switch (type) {
    case SCE_CAMERA_EV_NEGATIVE_20: return "SCE_CAMERA_EV_NEGATIVE_20";
    case SCE_CAMERA_EV_NEGATIVE_17: return "SCE_CAMERA_EV_NEGATIVE_17";
    case SCE_CAMERA_EV_NEGATIVE_15: return "SCE_CAMERA_EV_NEGATIVE_15";
    case SCE_CAMERA_EV_NEGATIVE_13: return "SCE_CAMERA_EV_NEGATIVE_13";
    case SCE_CAMERA_EV_NEGATIVE_10: return "SCE_CAMERA_EV_NEGATIVE_10";
    case SCE_CAMERA_EV_NEGATIVE_7: return "SCE_CAMERA_EV_NEGATIVE_7";
    case SCE_CAMERA_EV_NEGATIVE_5: return "SCE_CAMERA_EV_NEGATIVE_5";
    case SCE_CAMERA_EV_NEGATIVE_3: return "SCE_CAMERA_EV_NEGATIVE_3";
    case SCE_CAMERA_EV_POSITIVE_0: return "SCE_CAMERA_EV_POSITIVE_0";
    case SCE_CAMERA_EV_POSITIVE_3: return "SCE_CAMERA_EV_POSITIVE_3";
    case SCE_CAMERA_EV_POSITIVE_5: return "SCE_CAMERA_EV_POSITIVE_5";
    case SCE_CAMERA_EV_POSITIVE_7: return "SCE_CAMERA_EV_POSITIVE_7";
    case SCE_CAMERA_EV_POSITIVE_10: return "SCE_CAMERA_EV_POSITIVE_10";
    case SCE_CAMERA_EV_POSITIVE_13: return "SCE_CAMERA_EV_POSITIVE_13";
    case SCE_CAMERA_EV_POSITIVE_15: return "SCE_CAMERA_EV_POSITIVE_15";
    case SCE_CAMERA_EV_POSITIVE_17: return "SCE_CAMERA_EV_POSITIVE_17";
    case SCE_CAMERA_EV_POSITIVE_20: return "SCE_CAMERA_EV_POSITIVE_20";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraEffect>(const MemState &mem, SceCameraEffect type) {
    switch (type) {
    case SCE_CAMERA_EFFECT_NORMAL: return "SCE_CAMERA_EFFECT_NORMAL";
    case SCE_CAMERA_EFFECT_NEGATIVE: return "SCE_CAMERA_EFFECT_NEGATIVE";
    case SCE_CAMERA_EFFECT_BLACKWHITE: return "SCE_CAMERA_EFFECT_BLACKWHITE";
    case SCE_CAMERA_EFFECT_SEPIA: return "SCE_CAMERA_EFFECT_SEPIA";
    case SCE_CAMERA_EFFECT_BLUE: return "SCE_CAMERA_EFFECT_BLUE";
    case SCE_CAMERA_EFFECT_RED: return "SCE_CAMERA_EFFECT_RED";
    case SCE_CAMERA_EFFECT_GREEN: return "SCE_CAMERA_EFFECT_GREEN";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraReverse>(const MemState &mem, SceCameraReverse type) {
    switch (type) {
    case SCE_CAMERA_REVERSE_OFF: return "SCE_CAMERA_REVERSE_OFF";
    case SCE_CAMERA_REVERSE_MIRROR: return "SCE_CAMERA_REVERSE_MIRROR";
    case SCE_CAMERA_REVERSE_FLIP: return "SCE_CAMERA_REVERSE_FLIP";
    case SCE_CAMERA_REVERSE_MIRROR_FLIP: return "SCE_CAMERA_REVERSE_MIRROR_FLIP";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraSaturation>(const MemState &mem, SceCameraSaturation type) {
    switch (type) {
    case SCE_CAMERA_SATURATION_0: return "SCE_CAMERA_SATURATION_0";
    case SCE_CAMERA_SATURATION_5: return "SCE_CAMERA_SATURATION_5";
    case SCE_CAMERA_SATURATION_10: return "SCE_CAMERA_SATURATION_10";
    case SCE_CAMERA_SATURATION_20: return "SCE_CAMERA_SATURATION_20";
    case SCE_CAMERA_SATURATION_30: return "SCE_CAMERA_SATURATION_30";
    case SCE_CAMERA_SATURATION_40: return "SCE_CAMERA_SATURATION_40";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraSharpness>(const MemState &mem, SceCameraSharpness type) {
    switch (type) {
    case SCE_CAMERA_SHARPNESS_100: return "SCE_CAMERA_SHARPNESS_100";
    case SCE_CAMERA_SHARPNESS_200: return "SCE_CAMERA_SHARPNESS_200";
    case SCE_CAMERA_SHARPNESS_300: return "SCE_CAMERA_SHARPNESS_300";
    case SCE_CAMERA_SHARPNESS_400: return "SCE_CAMERA_SHARPNESS_400";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraAntiFlicker>(const MemState &mem, SceCameraAntiFlicker type) {
    switch (type) {
    case SCE_CAMERA_ANTIFLICKER_AUTO: return "SCE_CAMERA_ANTIFLICKER_AUTO";
    case SCE_CAMERA_ANTIFLICKER_50HZ: return "SCE_CAMERA_ANTIFLICKER_50HZ";
    case SCE_CAMERA_ANTIFLICKER_60HZ: return "SCE_CAMERA_ANTIFLICKER_60HZ";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraISO>(const MemState &mem, SceCameraISO type) {
    switch (type) {
    case SCE_CAMERA_ISO_AUTO: return "SCE_CAMERA_ISO_AUTO";
    case SCE_CAMERA_ISO_100: return "SCE_CAMERA_ISO_100";
    case SCE_CAMERA_ISO_200: return "SCE_CAMERA_ISO_200";
    case SCE_CAMERA_ISO_400: return "SCE_CAMERA_ISO_400";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraGain>(const MemState &mem, SceCameraGain type) {
    switch (type) {
    case SCE_CAMERA_GAIN_AUTO: return "SCE_CAMERA_GAIN_AUTO";
    case SCE_CAMERA_GAIN_1: return "SCE_CAMERA_GAIN_1";
    case SCE_CAMERA_GAIN_2: return "SCE_CAMERA_GAIN_2";
    case SCE_CAMERA_GAIN_3: return "SCE_CAMERA_GAIN_3";
    case SCE_CAMERA_GAIN_4: return "SCE_CAMERA_GAIN_4";
    case SCE_CAMERA_GAIN_5: return "SCE_CAMERA_GAIN_5";
    case SCE_CAMERA_GAIN_6: return "SCE_CAMERA_GAIN_6";
    case SCE_CAMERA_GAIN_7: return "SCE_CAMERA_GAIN_7";
    case SCE_CAMERA_GAIN_8: return "SCE_CAMERA_GAIN_8";
    case SCE_CAMERA_GAIN_9: return "SCE_CAMERA_GAIN_9";
    case SCE_CAMERA_GAIN_10: return "SCE_CAMERA_GAIN_10";
    case SCE_CAMERA_GAIN_11: return "SCE_CAMERA_GAIN_11";
    case SCE_CAMERA_GAIN_12: return "SCE_CAMERA_GAIN_12";
    case SCE_CAMERA_GAIN_13: return "SCE_CAMERA_GAIN_13";
    case SCE_CAMERA_GAIN_14: return "SCE_CAMERA_GAIN_14";
    case SCE_CAMERA_GAIN_15: return "SCE_CAMERA_GAIN_15";
    case SCE_CAMERA_GAIN_16: return "SCE_CAMERA_GAIN_16";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraWhiteBalance>(const MemState &mem, SceCameraWhiteBalance type) {
    switch (type) {
    case SCE_CAMERA_WB_AUTO: return "SCE_CAMERA_WB_AUTO";
    case SCE_CAMERA_WB_DAY: return "SCE_CAMERA_WB_DAY";
    case SCE_CAMERA_WB_CWF: return "SCE_CAMERA_WB_CWF";
    case SCE_CAMERA_WB_SLSA: return "SCE_CAMERA_WB_SLSA";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraBacklight>(const MemState &mem, SceCameraBacklight type) {
    switch (type) {
    case SCE_CAMERA_BACKLIGHT_OFF: return "SCE_CAMERA_BACKLIGHT_OFF";
    case SCE_CAMERA_BACKLIGHT_ON: return "SCE_CAMERA_BACKLIGHT_ON";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceCameraNightmode>(const MemState &mem, SceCameraNightmode type) {
    switch (type) {
    case SCE_CAMERA_NIGHTMODE_OFF: return "SCE_CAMERA_NIGHTMODE_OFF";
    case SCE_CAMERA_NIGHTMODE_LESS10: return "SCE_CAMERA_NIGHTMODE_LESS10";
    case SCE_CAMERA_NIGHTMODE_LESS100: return "SCE_CAMERA_NIGHTMODE_LESS100";
    case SCE_CAMERA_NIGHTMODE_OVER100: return "SCE_CAMERA_NIGHTMODE_OVER100";
    }
    return std::to_string(type);
}

template <>
inline std::string to_debug_str<SceCameraBuffer>(const MemState &mem, SceCameraBuffer type) {
    switch (type) {
    case SceCameraBuffer::SCE_CAMERA_BUFFER_SETBYOPEN: return "SCE_CAMERA_BUFFER_SETBYOPEN";
    case SceCameraBuffer::SCE_CAMERA_BUFFER_SETBYREAD: return "SCE_CAMERA_BUFFER_SETBYREAD";
    }
    return std::to_string(type);
}

template <>
inline std::string to_debug_str<SceCameraReadMode>(const MemState &mem, SceCameraReadMode type) {
    switch (type) {
    case SCE_CAMERA_READ_MODE_WAIT_NEXTFRAME_ON: return "SCE_CAMERA_READ_MODE_WAIT_NEXTFRAME_ON";
    case SCE_CAMERA_READ_MODE_WAIT_NEXTFRAME_OFF: return "SCE_CAMERA_READ_MODE_WAIT_NEXTFRAME_OFF";
    }
    return std::to_string(type);
}

template <>
inline std::string to_debug_str<SceCameraReadGetExposureTime>(const MemState &mem, SceCameraReadGetExposureTime type) {
    switch (type) {
    case SCE_CAMERA_READ_GET_EXPOSURE_TIME_OFF: return "SCE_CAMERA_READ_GET_EXPOSURE_TIME_OFF";
    case SCE_CAMERA_READ_GET_EXPOSURE_TIME_ON: return "SCE_CAMERA_READ_GET_EXPOSURE_TIME_ON";
    }
    return std::to_string(type);
}

template <>
inline std::string to_debug_str<SceCameraStatus>(const MemState &mem, SceCameraStatus type) {
    switch (type) {
    case SCE_CAMERA_STATUS_IS_ACTIVE: return "SCE_CAMERA_STATUS_IS_ACTIVE";
    case SCE_CAMERA_STATUS_IS_NOT_ACTIVE: return "SCE_CAMERA_STATUS_IS_NOT_ACTIVE";
    case SCE_CAMERA_STATUS_IS_FORCED_STOP: return "SCE_CAMERA_STATUS_IS_FORCED_STOP";
    case SCE_CAMERA_STATUS_IS_FORCED_STOP_POWER_CONFIG_CHANGE: return "SCE_CAMERA_STATUS_IS_FORCED_STOP_POWER_CONFIG_CHANGE";
    case SCE_CAMERA_STATUS_IS_ALREADY_READ: return "SCE_CAMERA_STATUS_IS_ALREADY_READ";
    case SCE_CAMERA_STATUS_IS_NOT_STABLE: return "SCE_CAMERA_STATUS_IS_NOT_STABLE";
    }
    return std::to_string(type);
}

template <>
inline std::string to_debug_str<SceCameraDataRange>(const MemState &mem, SceCameraDataRange type) {
    switch (type) {
    case SCE_CAMERA_DATA_RANGE_FULL: return "SCE_CAMERA_DATA_RANGE_FULL";
    case SCE_CAMERA_DATA_RANGE_BT601: return "SCE_CAMERA_DATA_RANGE_BT601";
    }
    return std::to_string(type);
}

EXPORT(int, sceCameraOpen, SceCameraDevice devnum, SceCameraInfo *pInfo) {
    TRACY_FUNC(sceCameraOpen, devnum, pInfo);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    UNIMPLEMENTED();
    emuenv.camera.front_camera.base_ticks = emuenv.kernel.base_tick.tick;
    emuenv.camera.back_camera.base_ticks = emuenv.kernel.base_tick.tick;
    auto res = emuenv.camera.get_camera(devnum).open(pInfo);
    LOG_TRACE("devnum:{}", to_debug_str(emuenv.mem, devnum));
    LOG_TRACE("size:{}", to_debug_str(emuenv.mem, pInfo->size));
    LOG_TRACE("priority:{}", to_debug_str(emuenv.mem, (SceCameraPriority)pInfo->priority));
    LOG_TRACE("format:{}", to_debug_str(emuenv.mem, (SceCameraFormat)pInfo->format));
    LOG_TRACE("resolution:{}", to_debug_str(emuenv.mem, (SceCameraResolution)pInfo->resolution));
    LOG_TRACE("framerate:{}", to_debug_str(emuenv.mem, (SceCameraFrameRate)pInfo->framerate));
    LOG_TRACE("width:{}", to_debug_str(emuenv.mem, pInfo->width));
    LOG_TRACE("height:{}", to_debug_str(emuenv.mem, pInfo->height));
    LOG_TRACE("range:{}", to_debug_str(emuenv.mem, (SceCameraDataRange)pInfo->range));
    LOG_TRACE("pad:{}", to_debug_str(emuenv.mem, pInfo->pad));
    LOG_TRACE("sizeIBase:{}", to_debug_str(emuenv.mem, pInfo->sizeIBase));
    LOG_TRACE("sizeUBase:{}", to_debug_str(emuenv.mem, pInfo->sizeUBase));
    LOG_TRACE("sizeVBase:{}", to_debug_str(emuenv.mem, pInfo->sizeVBase));
    LOG_TRACE("pIBase:{}", to_debug_str(emuenv.mem, pInfo->pIBase));
    LOG_TRACE("pUBase:{}", to_debug_str(emuenv.mem, pInfo->pUBase));
    LOG_TRACE("pVBase:{}", to_debug_str(emuenv.mem, pInfo->pVBase));
    LOG_TRACE("pitch:{}", to_debug_str(emuenv.mem, pInfo->pitch));
    LOG_TRACE("buffer:{}", to_debug_str(emuenv.mem, (SceCameraBuffer)pInfo->buffer));
    return res;
}

EXPORT(int, sceCameraClose, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraClose, devnum);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    UNIMPLEMENTED();
    return emuenv.camera.get_camera(devnum).close();
    // return UNIMPLEMENTED();
}

EXPORT(int, sceCameraStart, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraStart, devnum);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    UNIMPLEMENTED();
    return emuenv.camera.get_camera(devnum).start();
}

EXPORT(int, sceCameraStop, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraStop, devnum);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    UNIMPLEMENTED();
    return emuenv.camera.get_camera(devnum).stop();
}

EXPORT(int, sceCameraRead, SceCameraDevice devnum, SceCameraRead *pRead) {
    TRACY_FUNC(sceCameraRead, devnum, pRead);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    UNIMPLEMENTED();
    int res = emuenv.camera.get_camera(devnum).is_active();
    if (res >= 0)
        res = emuenv.camera.get_camera(devnum).read(emuenv, pRead);
    /*
    LOG_TRACE("size:{}", to_debug_str(emuenv.mem, pRead->size));
    LOG_TRACE("mode:{}", to_debug_str(emuenv.mem, (SceCameraReadMode)pRead->mode));
    LOG_TRACE("pad:{}", to_debug_str(emuenv.mem, (SceCameraReadGetExposureTime)pRead->pad));
    LOG_TRACE("status:{}", to_debug_str(emuenv.mem, (SceCameraStatus)pRead->status));
    LOG_TRACE("frame:{}", to_debug_str(emuenv.mem, pRead->frame));
    LOG_TRACE("timestamp:{}", to_debug_str(emuenv.mem, pRead->timestamp));
    LOG_TRACE("exposure_time:{}", to_debug_str(emuenv.mem, pRead->exposure_time));
    LOG_TRACE("exposure_time_gap:{}", to_debug_str(emuenv.mem, pRead->exposure_time_gap));
    LOG_TRACE("raw8_format:{}", to_debug_str(emuenv.mem, pRead->raw8_format));
    LOG_TRACE("sizeIBase:{}", to_debug_str(emuenv.mem, pRead->sizeIBase));
    LOG_TRACE("sizeUBase:{}", to_debug_str(emuenv.mem, pRead->sizeUBase));
    LOG_TRACE("sizeVBase:{}", to_debug_str(emuenv.mem, pRead->sizeVBase));
    LOG_TRACE("pIBase:{}", to_debug_str(emuenv.mem, pRead->pIBase));
    LOG_TRACE("pUBase:{}", to_debug_str(emuenv.mem, pRead->pUBase));
    LOG_TRACE("pVBase:{}", to_debug_str(emuenv.mem, pRead->pVBase));
    /**/
    return res;
}

EXPORT(int, sceCameraIsActive, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraIsActive, devnum);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    UNIMPLEMENTED();
    return emuenv.camera.is_active(devnum);
}

EXPORT(int, sceCameraGetSaturation, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetSaturation, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetSaturation, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetSaturation, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetBrightness, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetBrightness, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetBrightness, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetBrightness, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetContrast, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetContrast, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetContrast, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetContrast, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetSharpness, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetSharpness, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetSharpness, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetSharpness, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetReverse, SceCameraDevice devnum, SceCameraReverse *pMode) {
    TRACY_FUNC(sceCameraGetReverse, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetReverse, SceCameraDevice devnum, SceCameraReverse mode) {
    TRACY_FUNC(sceCameraSetReverse, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetEffect, SceCameraDevice devnum, SceCameraEffect *pMode) {
    TRACY_FUNC(sceCameraGetEffect, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetEffect, SceCameraDevice devnum, SceCameraEffect mode) {
    TRACY_FUNC(sceCameraSetEffect, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetEV, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetEV, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetEV, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetEV, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetZoom, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetZoom, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetZoom, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetZoom, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetAntiFlicker, SceCameraDevice devnum, SceCameraAntiFlicker *pMode) {
    TRACY_FUNC(sceCameraGetAntiFlicker, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetAntiFlicker, SceCameraDevice devnum, SceCameraAntiFlicker mode) {
    TRACY_FUNC(sceCameraSetAntiFlicker, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetISO, SceCameraDevice devnum, SceCameraISO *pMode) {
    TRACY_FUNC(sceCameraGetISO, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetISO, SceCameraDevice devnum, SceCameraISO mode) {
    TRACY_FUNC(sceCameraSetISO, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetGain, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetGain, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetGain, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetGain, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetWhiteBalance, SceCameraDevice devnum, SceCameraWhiteBalance *pMode) {
    TRACY_FUNC(sceCameraGetWhiteBalance, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetWhiteBalance, SceCameraDevice devnum, SceCameraWhiteBalance mode) {
    TRACY_FUNC(sceCameraSetWhiteBalance, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetBacklight, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetBacklight, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetBacklight, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetBacklight, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetNightmode, SceCameraDevice devnum, SceCameraNightmode *pMode) {
    TRACY_FUNC(sceCameraGetNightmode, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetNightmode, SceCameraDevice devnum, SceCameraNightmode mode) {
    TRACY_FUNC(sceCameraSetNightmode, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetExposureCeiling, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetExposureCeiling, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetExposureCeiling, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetExposureCeiling, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetAutoControlHold, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetAutoControlHold, devnum, pMode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetAutoControlHold, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetAutoControlHold, devnum, mode);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetDeviceLocation, SceCameraDevice devnum, SceFVector3 *pLocation) {
    TRACY_FUNC(sceCameraGetDeviceLocation, devnum, pLocation);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetImageQuality, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetImageQuality, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraSetImageQuality, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetImageQuality, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraGetNoiseReduction, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetNoiseReduction, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraSetNoiseReduction, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetNoiseReduction, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraGetSharpnessOff, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetSharpnessOff, devnum, pLevel);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraSetSharpnessOff, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetSharpnessOff, devnum, level);
    if (emuenv.cfg.pstv_mode)
        return SCE_CAMERA_ERROR_NOT_MOUNTED;
    if (devnum != SCE_CAMERA_DEVICE_BACK && devnum != SCE_CAMERA_DEVICE_FRONT)
        return SCE_CAMERA_ERROR_PARAM;
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceCameraClose)
BRIDGE_IMPL(sceCameraGetAntiFlicker)
BRIDGE_IMPL(sceCameraGetAutoControlHold)
BRIDGE_IMPL(sceCameraGetBacklight)
BRIDGE_IMPL(sceCameraGetBrightness)
BRIDGE_IMPL(sceCameraGetContrast)
BRIDGE_IMPL(sceCameraGetDeviceLocation)
BRIDGE_IMPL(sceCameraGetEV)
BRIDGE_IMPL(sceCameraGetEffect)
BRIDGE_IMPL(sceCameraGetExposureCeiling)
BRIDGE_IMPL(sceCameraGetGain)
BRIDGE_IMPL(sceCameraGetISO)
BRIDGE_IMPL(sceCameraGetImageQuality)
BRIDGE_IMPL(sceCameraGetNightmode)
BRIDGE_IMPL(sceCameraGetNoiseReduction)
BRIDGE_IMPL(sceCameraGetReverse)
BRIDGE_IMPL(sceCameraGetSaturation)
BRIDGE_IMPL(sceCameraGetSharpness)
BRIDGE_IMPL(sceCameraGetSharpnessOff)
BRIDGE_IMPL(sceCameraGetWhiteBalance)
BRIDGE_IMPL(sceCameraGetZoom)
BRIDGE_IMPL(sceCameraIsActive)
BRIDGE_IMPL(sceCameraOpen)
BRIDGE_IMPL(sceCameraRead)
BRIDGE_IMPL(sceCameraSetAntiFlicker)
BRIDGE_IMPL(sceCameraSetAutoControlHold)
BRIDGE_IMPL(sceCameraSetBacklight)
BRIDGE_IMPL(sceCameraSetBrightness)
BRIDGE_IMPL(sceCameraSetContrast)
BRIDGE_IMPL(sceCameraSetEV)
BRIDGE_IMPL(sceCameraSetEffect)
BRIDGE_IMPL(sceCameraSetExposureCeiling)
BRIDGE_IMPL(sceCameraSetGain)
BRIDGE_IMPL(sceCameraSetISO)
BRIDGE_IMPL(sceCameraSetImageQuality)
BRIDGE_IMPL(sceCameraSetNightmode)
BRIDGE_IMPL(sceCameraSetNoiseReduction)
BRIDGE_IMPL(sceCameraSetReverse)
BRIDGE_IMPL(sceCameraSetSaturation)
BRIDGE_IMPL(sceCameraSetSharpness)
BRIDGE_IMPL(sceCameraSetSharpnessOff)
BRIDGE_IMPL(sceCameraSetWhiteBalance)
BRIDGE_IMPL(sceCameraSetZoom)
BRIDGE_IMPL(sceCameraStart)
BRIDGE_IMPL(sceCameraStop)
