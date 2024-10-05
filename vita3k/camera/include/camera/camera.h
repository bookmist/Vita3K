﻿// Vita3K emulator project
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

#pragma once

#include <emuenv/state.h>
#include <mem/ptr.h>
#include <util/types.h>

enum SceCameraErrorCode : uint32_t {
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
    SCE_CAMERA_DEVICE_UNKNOWN = -1,
    SCE_CAMERA_DEVICE_FRONT = 0, //!< Front camera
    SCE_CAMERA_DEVICE_BACK = 1, //!< Retro camera
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

/* Camera Status */
enum SceCameraStatus {
    SCE_CAMERA_STATUS_IS_ACTIVE = 0,
    SCE_CAMERA_STATUS_IS_NOT_ACTIVE = -1,
    SCE_CAMERA_STATUS_IS_FORCED_STOP = -2,
    SCE_CAMERA_STATUS_IS_FORCED_STOP_POWER_CONFIG_CHANGE = -3,
    SCE_CAMERA_STATUS_IS_ALREADY_READ = 1,
    SCE_CAMERA_STATUS_IS_NOT_STABLE = 2
};

/* Camera Read Mode */
enum SceCameraReadMode {
    SCE_CAMERA_READ_MODE_WAIT_NEXTFRAME_ON = 0,
    SCE_CAMERA_READ_MODE_WAIT_NEXTFRAME_OFF = 1
};

/* Camera Read Exposure Time Mode */
enum SceCameraReadGetExposureTime {
    SCE_CAMERA_READ_GET_EXPOSURE_TIME_OFF = 0,
    SCE_CAMERA_READ_GET_EXPOSURE_TIME_ON = 1
};

/* Camera Data Range */

enum SceCameraDataRange {
    SCE_CAMERA_DATA_RANGE_FULL = 0,
    SCE_CAMERA_DATA_RANGE_BT601 = 1
};

/* Camera Buffer */
enum SceCameraBuffer {
    SCE_CAMERA_BUFFER_SETBYOPEN = 0,
    SCE_CAMERA_BUFFER_SETBYREAD = 1
};

/* Camera Raw8 Format Pattern */
enum SceCameraRaw8Format {
    SCE_CAMERA_RAW8_FORMAT_UNKNOWN = 0,
    SCE_CAMERA_RAW8_FORMAT_BGGR = 1,
    SCE_CAMERA_RAW8_FORMAT_GRBG = 2,
    SCE_CAMERA_RAW8_FORMAT_RGGB = 3,
    SCE_CAMERA_RAW8_FORMAT_GBRG = 4
};

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
    uint32_t exposure_time;
    uint32_t exposure_time_gap;
    uint32_t raw8_format;
    uint32_t unk_01;
    SceSize sizeIBase;
    SceSize sizeUBase;
    SceSize sizeVBase;
    Ptr<void> pIBase;
    Ptr<void> pUBase;
    Ptr<void> pVBase;
};

#ifdef USE_OPENCV
namespace cv {
class VideoCapture;
}
#endif // USE_OPENCV

enum class CameraAttributes {
    Saturation,
    Brightness,
    Contrast,
    Sharpness,
    Reverse,
    Effect,
    EV,
    Zoom,
    AntiFlicker,
    ISO,
    Gain,
    WhiteBalance,
    Backlight,
    Nightmode,
    ExposureCeiling,
    AutoControlHold,
    ImageQuality,
    NoiseReduction,
    SharpnessOff
};

class Camera {
private:
    class CameraImpl;
    std::unique_ptr<CameraImpl> pImpl; // opaque type here
public:
    SceCameraInfo info;
    bool is_opened{};
    bool is_started{};
    uint64_t frame_idx{};
    uint64_t base_ticks{}; // emuenv.kernel.base_tick.tick for timestamp
    //
    int get_attribute(CameraAttributes attribute);
    int set_attribute(CameraAttributes attribute, int value);
    int open(SceCameraInfo *info);
    int close();
    int start();
    int stop();
    int read(EmuEnvState &emuenv, SceCameraRead *read, void *pIBase, void *pUBase, void *pVBase, SceSize sizeIBase, SceSize sizeUBase, SceSize sizeVBase);
    Camera();
    ~Camera();

    // disable copy
    Camera(const Camera &) = delete;
    Camera &operator=(Camera const &) = delete;
};
