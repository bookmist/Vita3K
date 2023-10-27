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

#include <camera/camera.h>
#include <camera/functions.h>
#include <camera/state.h>
#include <emuenv/state.h>
#include <mem/ptr.h>
#include <rtc/rtc.h>
#include <util/log.h>

class Camera::CameraImpl {};

Camera::Camera(){};
Camera::~Camera(){};

int Camera::get_attribute(CameraAttributes attribute) {
    switch (attribute) {
    case CameraAttributes::Saturation: return SCE_CAMERA_SATURATION_10;
    case CameraAttributes::Brightness: return 128; // 0-255
    case CameraAttributes::Contrast: return 128; // random default value  0-255
    case CameraAttributes::Sharpness: return SCE_CAMERA_SHARPNESS_100;
    case CameraAttributes::Reverse: return SCE_CAMERA_REVERSE_OFF;
    case CameraAttributes::Effect: return SCE_CAMERA_EFFECT_NORMAL;
    case CameraAttributes::EV: return 0;
    case CameraAttributes::Zoom: return 10; // 10-40
    case CameraAttributes::AntiFlicker: return SCE_CAMERA_ANTIFLICKER_AUTO;
    case CameraAttributes::ISO: return SCE_CAMERA_ISO_AUTO;
    case CameraAttributes::Gain: return SCE_CAMERA_GAIN_AUTO;
    case CameraAttributes::WhiteBalance: return SCE_CAMERA_WB_AUTO;
    case CameraAttributes::Backlight: return SCE_CAMERA_BACKLIGHT_OFF;
    case CameraAttributes::Nightmode: return SCE_CAMERA_NIGHTMODE_OFF;
    case CameraAttributes::ExposureCeiling: return 0;
    case CameraAttributes::AutoControlHold: return 0;
    // undescribed parameters
    case CameraAttributes::ImageQuality: return 0;
    case CameraAttributes::NoiseReduction: return 0;
    case CameraAttributes::SharpnessOff: return 0;
    }
    return 0;
}

int Camera::set_attribute(CameraAttributes attribute, int value) {
	return 0;
}

int Camera::open(SceCameraInfo *info) {
    this->is_opened = true;
    this->info = *info;
    return 0;
}

int Camera::start() {
    this->is_started = true;
    this->frame_idx = 0;
    return 0;
}

int Camera::read(EmuEnvState &emuenv, SceCameraRead *read, void *pIBase, void *pUBase, void *pVBase, SceSize sizeIBase, SceSize sizeUBase, SceSize sizeVBase) {
    read->frame = frame_idx++;
    read->status = SCE_CAMERA_STATUS_IS_ACTIVE;
    read->timestamp = rtc_get_ticks(this->base_ticks);
    switch (this->info.format) {
    case SCE_CAMERA_FORMAT_YUV420_PLANE:
    case SCE_CAMERA_FORMAT_YUV422_PLANE:
        memset(pIBase, 255, sizeIBase);
        memset(pUBase, 127, sizeUBase);
        memset(pVBase, 127, sizeVBase);
        break;
    case SCE_CAMERA_FORMAT_YUV422_PACKED:
    case SCE_CAMERA_FORMAT_ABGR:
    case SCE_CAMERA_FORMAT_ARGB:
        memset(pIBase, 255, sizeIBase);
        break;
    case SCE_CAMERA_FORMAT_RAW8:
        memset(pIBase, 255, sizeIBase);
        read->raw8_format = SCE_CAMERA_RAW8_FORMAT_BGGR; // just random value
    }

    return 0;
}

int Camera::close() {
    this->is_started = false;
    this->is_opened = false;
    return 0;
}

int Camera::stop() {
    this->is_started = false;
    return 0;
}
