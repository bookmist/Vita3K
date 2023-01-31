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

#include <util/tracy.h>
TRACY_MODULE_NAME(SceCamera);

enum SceCameraErrorCode {
    SCE_CAMERA_ERROR_PARAM = 0x802E0000,
    SCE_CAMERA_ERROR_ALREADY_INIT = 0x802E0001,
    SCE_CAMERA_ERROR_NOT_INIT = 0x802E0002,
    SCE_CAMERA_ERROR_ALREADY_OPEN = 0x802E0003,
    SCE_CAMERA_ERROR_NOT_OPEN = 0x802E0004,
    SCE_CAMERA_ERROR_ALREADY_START = 0x802E0005,
    SCE_CAMERA_ERROR_NOT_START = 0x802E0006,
    SCE_CAMERA_ERROR_FORMAT_UNKNOWN = 0x802E0007,
    SCE_CAMERA_ERROR_RESOLUTION_UNKNOWN = 0x802E0008,
    SCE_CAMERA_ERROR_BAD_FRAMERATE = 0x802E0009,
    SCE_CAMERA_ERROR_TIMEOUT = 0x802E000A,
    SCE_CAMERA_ERROR_EXCLUSIVE = 0x802E000B,
    SCE_CAMERA_ERROR_ATTRIBUTE_UNKNOWN = 0x802E000C,
    SCE_CAMERA_ERROR_MAX_PROCESS = 0x802E000D,
    SCE_CAMERA_ERROR_NOT_ACTIVE = 0x802E000E,
    SCE_CAMERA_ERROR_ALREADY_READ = 0x802E000F,
    SCE_CAMERA_ERROR_NOT_MOUNTED = 0x802E0010,
    SCE_CAMERA_ERROR_DATA_RANGE_UNKNOWN = 0x802E0011,
    SCE_CAMERA_ERROR_OTHER_ALREADY_START = 0x802E0012,
    SCE_CAMERA_ERROR_FATAL = 0x802E00FF
};

enum SceCameraDevice {
    SCE_CAMERA_DEVICE_FRONT = 0, //!< Front camera
    SCE_CAMERA_DEVICE_BACK = 1 //!< Retro camera
};

enum SceCameraPriority {
    SCE_CAMERA_PRIORITY_SHARE = 0, //!< Share mode
    SCE_CAMERA_PRIORITY_EXCLUSIVE = 1 //!< Exclusive mode
};

enum SceCameraFormat {
    SCE_CAMERA_FORMAT_INVALID = 0, //!< Invalid format
    SCE_CAMERA_FORMAT_YUV422_PLANE = 1, //!< YUV422 planes
    SCE_CAMERA_FORMAT_YUV422_PACKED = 2, //!< YUV422 pixels packed
    SCE_CAMERA_FORMAT_YUV420_PLANE = 3, //!< YUV420 planes
    SCE_CAMERA_FORMAT_ARGB = 4, //!< ARGB pixels
    SCE_CAMERA_FORMAT_ABGR = 5, //!< ABGR pixels
    SCE_CAMERA_FORMAT_RAW8 = 6 //!< 8 bit raw data
};

enum SceCameraResolution {
    SCE_CAMERA_RESOLUTION_0_0 = 0, //!< Invalid resolution
    SCE_CAMERA_RESOLUTION_640_480 = 1, //!< VGA resolution
    SCE_CAMERA_RESOLUTION_320_240 = 2, //!< QVGA resolution
    SCE_CAMERA_RESOLUTION_160_120 = 3, //!< QQVGA resolution
    SCE_CAMERA_RESOLUTION_352_288 = 4, //!< CIF resolution
    SCE_CAMERA_RESOLUTION_176_144 = 5, //!< QCIF resolution
    SCE_CAMERA_RESOLUTION_480_272 = 6, //!< PSP resolution
    SCE_CAMERA_RESOLUTION_640_360 = 8 //!< NGP resolution
};

enum SceCameraFrameRate {
    SCE_CAMERA_FRAMERATE_3_FPS = 3, //!< 3.75 fps
    SCE_CAMERA_FRAMERATE_5_FPS = 5, //!< 5 fps
    SCE_CAMERA_FRAMERATE_7_FPS = 7, //!< 7.5 fps
    SCE_CAMERA_FRAMERATE_10_FPS = 10, //!< 10 fps
    SCE_CAMERA_FRAMERATE_15_FPS = 15, //!< 15 fps
    SCE_CAMERA_FRAMERATE_20_FPS = 20, //!< 20 fps
    SCE_CAMERA_FRAMERATE_30_FPS = 30, //!< 30 fps
    SCE_CAMERA_FRAMERATE_60_FPS = 60, //!< 60 fps
    SCE_CAMERA_FRAMERATE_120_FPS = 120 //!< 120 fps (@note Resolution must be QVGA or lower)
};

enum SceCameraExposureCompensation {
    SCE_CAMERA_EV_NEGATIVE_20 = -20, //!< -2.0
    SCE_CAMERA_EV_NEGATIVE_17 = -17, //!< -1.7
    SCE_CAMERA_EV_NEGATIVE_15 = -15, //!< -1.5
    SCE_CAMERA_EV_NEGATIVE_13 = -13, //!< -1.3
    SCE_CAMERA_EV_NEGATIVE_10 = -10, //!< -1.0
    SCE_CAMERA_EV_NEGATIVE_7 = -7, //!< -0.7
    SCE_CAMERA_EV_NEGATIVE_5 = -5, //!< -0.5
    SCE_CAMERA_EV_NEGATIVE_3 = -3, //!< -0.3
    SCE_CAMERA_EV_POSITIVE_0 = 0, //!< +0.0
    SCE_CAMERA_EV_POSITIVE_3 = 3, //!< +0.3
    SCE_CAMERA_EV_POSITIVE_5 = 5, //!< +0.5
    SCE_CAMERA_EV_POSITIVE_7 = 7, //!< +0.7
    SCE_CAMERA_EV_POSITIVE_10 = 10, //!< +1.0
    SCE_CAMERA_EV_POSITIVE_13 = 13, //!< +1.3
    SCE_CAMERA_EV_POSITIVE_15 = 15, //!< +1.5
    SCE_CAMERA_EV_POSITIVE_17 = 17, //!< +1.7
    SCE_CAMERA_EV_POSITIVE_20 = 20 //!< +2.0
};

enum SceCameraEffect {
    SCE_CAMERA_EFFECT_NORMAL = 0,
    SCE_CAMERA_EFFECT_NEGATIVE = 1,
    SCE_CAMERA_EFFECT_BLACKWHITE = 2,
    SCE_CAMERA_EFFECT_SEPIA = 3,
    SCE_CAMERA_EFFECT_BLUE = 4,
    SCE_CAMERA_EFFECT_RED = 5,
    SCE_CAMERA_EFFECT_GREEN = 6
};

enum SceCameraReverse {
    SCE_CAMERA_REVERSE_OFF = 0, //!< Reverse mode off
    SCE_CAMERA_REVERSE_MIRROR = 1, //!< Mirror mode
    SCE_CAMERA_REVERSE_FLIP = 2, //!< Flip mode
    SCE_CAMERA_REVERSE_MIRROR_FLIP = (SCE_CAMERA_REVERSE_MIRROR | SCE_CAMERA_REVERSE_FLIP) //!< Mirror + Flip mode
};

enum SceCameraSaturation {
    SCE_CAMERA_SATURATION_0 = 0, //!< 0.0
    SCE_CAMERA_SATURATION_5 = 5, //!< 0.5
    SCE_CAMERA_SATURATION_10 = 10, //!< 1.0
    SCE_CAMERA_SATURATION_20 = 20, //!< 2.0
    SCE_CAMERA_SATURATION_30 = 30, //!< 3.0
    SCE_CAMERA_SATURATION_40 = 40 //!< 4.0
};

enum SceCameraSharpness {
    SCE_CAMERA_SHARPNESS_100 = 1, //!< 100%
    SCE_CAMERA_SHARPNESS_200 = 2, //!< 200%
    SCE_CAMERA_SHARPNESS_300 = 3, //!< 300%
    SCE_CAMERA_SHARPNESS_400 = 4 //!< 400%
};

enum SceCameraAntiFlicker {
    SCE_CAMERA_ANTIFLICKER_AUTO = 1, //!< Automatic mode
    SCE_CAMERA_ANTIFLICKER_50HZ = 2, //!< 50 Hz mode
    SCE_CAMERA_ANTIFLICKER_60HZ = 3 //!< 50 Hz mode
};

enum SceCameraISO {
    SCE_CAMERA_ISO_AUTO = 1, //!< Automatic mode
    SCE_CAMERA_ISO_100 = 100, //!< ISO100/21?
    SCE_CAMERA_ISO_200 = 200, //!< ISO200/24?
    SCE_CAMERA_ISO_400 = 400 //!< ISO400/27?
};

enum SceCameraGain {
    SCE_CAMERA_GAIN_AUTO = 0,
    SCE_CAMERA_GAIN_1 = 1,
    SCE_CAMERA_GAIN_2 = 2,
    SCE_CAMERA_GAIN_3 = 3,
    SCE_CAMERA_GAIN_4 = 4,
    SCE_CAMERA_GAIN_5 = 5,
    SCE_CAMERA_GAIN_6 = 6,
    SCE_CAMERA_GAIN_7 = 7,
    SCE_CAMERA_GAIN_8 = 8,
    SCE_CAMERA_GAIN_9 = 9,
    SCE_CAMERA_GAIN_10 = 10,
    SCE_CAMERA_GAIN_11 = 11,
    SCE_CAMERA_GAIN_12 = 12,
    SCE_CAMERA_GAIN_13 = 13,
    SCE_CAMERA_GAIN_14 = 14,
    SCE_CAMERA_GAIN_15 = 15,
    SCE_CAMERA_GAIN_16 = 16
};

enum SceCameraWhiteBalance {
    SCE_CAMERA_WB_AUTO = 0, //!< Automatic mode
    SCE_CAMERA_WB_DAY = 1, //!< Daylight mode
    SCE_CAMERA_WB_CWF = 2, //!< Cool White Fluorescent mode
    SCE_CAMERA_WB_SLSA = 4 //!< Standard Light Source A mode
};

enum SceCameraBacklight {
    SCE_CAMERA_BACKLIGHT_OFF = 0, //!< Disabled
    SCE_CAMERA_BACKLIGHT_ON = 1 //!< Enabled
};

enum SceCameraNightmode {
    SCE_CAMERA_NIGHTMODE_OFF = 0, //!< Disabled
    SCE_CAMERA_NIGHTMODE_LESS10 = 1, //!< 10 lux or below
    SCE_CAMERA_NIGHTMODE_LESS100 = 2, //!< 100 lux or below
    SCE_CAMERA_NIGHTMODE_OVER100 = 3 //!< 100 lux or over
};

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

struct SceCameraInfo {
    SceSize size; //!< sizeof(SceCameraInfo)
    uint16_t priority; //!< Process priority (one of ::SceCameraPriority)
    uint16_t format; //!< Output format (One or more ::SceCameraFormat)
    uint16_t resolution; //!< Resolution (one of ::SceCameraResolution)
    uint16_t framerate; //!< Framerate (one of ::SceCameraFrameRate)
    uint16_t width;
    uint16_t height;
    uint16_t range;
    uint16_t pad; //!< Structure padding
    SceSize sizeIBase;
    SceSize sizeUBase;
    SceSize sizeVBase;
    Ptr<void> pIBase;
    Ptr<void> pUBase;
    Ptr<void> pVBase;
    uint16_t pitch;
    uint16_t buffer;
};

struct SceCameraRead {
    SceSize size; //!< sizeof(SceCameraRead)
    int mode;
    int pad;
    int status;
    uint64_t frame;
    uint64_t timestamp;
    SceSize sizeIBase;
    SceSize sizeUBase;
    SceSize sizeVBase;
    Ptr<void> pIBase;
    Ptr<void> pUBase;
    Ptr<void> pVBase;
};

EXPORT(int, sceCameraOpen, SceCameraDevice devnum, SceCameraInfo *pInfo) {
    TRACY_FUNC(sceCameraOpen, devnum, pInfo);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraClose, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraClose, devnum);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraStart, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraStart, devnum);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraStop, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraStop, devnum);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraRead, SceCameraDevice devnum, SceCameraRead *pRead) {
    TRACY_FUNC(sceCameraRead, devnum, pRead);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraIsActive, SceCameraDevice devnum) {
    TRACY_FUNC(sceCameraIsActive, devnum);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetSaturation, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetSaturation, devnum, pLevel);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetSaturation, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetSaturation, devnum, level);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetBrightness, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetBrightness, devnum, pLevel);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetBrightness, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetBrightness, devnum, level);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetContrast, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetContrast, devnum, pLevel);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetContrast, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetContrast, devnum, level);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetSharpness, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetSharpness, devnum, pLevel);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetSharpness, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetSharpness, devnum, level);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetReverse, SceCameraDevice devnum, SceCameraReverse *pMode) {
    TRACY_FUNC(sceCameraGetReverse, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetReverse, SceCameraDevice devnum, SceCameraReverse mode) {
    TRACY_FUNC(sceCameraSetReverse, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetEffect, SceCameraDevice devnum, SceCameraEffect *pMode) {
    TRACY_FUNC(sceCameraGetEffect, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetEffect, SceCameraDevice devnum, SceCameraEffect mode) {
    TRACY_FUNC(sceCameraSetEffect, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetEV, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetEV, devnum, pLevel);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetEV, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetEV, devnum, level);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetZoom, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetZoom, devnum, pLevel);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetZoom, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetZoom, devnum, level);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetAntiFlicker, SceCameraDevice devnum, SceCameraAntiFlicker *pMode) {
    TRACY_FUNC(sceCameraGetAntiFlicker, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetAntiFlicker, SceCameraDevice devnum, SceCameraAntiFlicker mode) {
    TRACY_FUNC(sceCameraSetAntiFlicker, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetISO, SceCameraDevice devnum, SceCameraISO *pMode) {
    TRACY_FUNC(sceCameraGetISO, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetISO, SceCameraDevice devnum, SceCameraISO mode) {
    TRACY_FUNC(sceCameraSetISO, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetGain, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetGain, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetGain, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetGain, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetWhiteBalance, SceCameraDevice devnum, SceCameraWhiteBalance *pMode) {
    TRACY_FUNC(sceCameraGetWhiteBalance, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetWhiteBalance, SceCameraDevice devnum, SceCameraWhiteBalance mode) {
    TRACY_FUNC(sceCameraSetWhiteBalance, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetBacklight, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetBacklight, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetBacklight, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetBacklight, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetNightmode, SceCameraDevice devnum, SceCameraNightmode *pMode) {
    TRACY_FUNC(sceCameraGetNightmode, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetNightmode, SceCameraDevice devnum, SceCameraNightmode mode) {
    TRACY_FUNC(sceCameraSetNightmode, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetExposureCeiling, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetExposureCeiling, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetExposureCeiling, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetExposureCeiling, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetAutoControlHold, SceCameraDevice devnum, int *pMode) {
    TRACY_FUNC(sceCameraGetAutoControlHold, devnum, pMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraSetAutoControlHold, SceCameraDevice devnum, int mode) {
    TRACY_FUNC(sceCameraSetAutoControlHold, devnum, mode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetDeviceLocation, SceCameraDevice devnum, SceFVector3 *pLocation) {
    TRACY_FUNC(sceCameraGetDeviceLocation, devnum, pLocation);
    return UNIMPLEMENTED();
}

EXPORT(int, sceCameraGetImageQuality, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetImageQuality, devnum, pLevel);
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraSetImageQuality, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetImageQuality, devnum, level);
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraGetNoiseReduction, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetNoiseReduction, devnum, pLevel);
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraSetNoiseReduction, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetNoiseReduction, devnum, level);
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraGetSharpnessOff, SceCameraDevice devnum, int *pLevel) {
    TRACY_FUNC(sceCameraGetSharpnessOff, devnum, pLevel);
    return UNIMPLEMENTED();
}
EXPORT(int, sceCameraSetSharpnessOff, SceCameraDevice devnum, int level) {
    TRACY_FUNC(sceCameraSetSharpnessOff, devnum, level);
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
