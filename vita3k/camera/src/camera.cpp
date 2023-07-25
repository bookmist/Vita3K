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
#include <opencv2/opencv.hpp>
#include <rtc/rtc.h>
#include <util/log.h>
#define export_name __FUNCTION__

// Put here function used for get camera states

Camera &CameraState::get_camera(SceCameraDevice devnum) {
    if (devnum == SCE_CAMERA_DEVICE_FRONT) {
        return this->front_camera;
    } else if (devnum == SCE_CAMERA_DEVICE_BACK) {
        return this->back_camera;
    } else {
        throw std::runtime_error("Invalid camera device");
    }
}

int CameraState::open(SceCameraDevice devnum, SceCameraInfo *pInfo) {
    Camera &camera = this->get_camera(devnum);
    return camera.open(pInfo);
}

int CameraState::is_active(SceCameraDevice devnum) {
    Camera &camera = this->get_camera(devnum);
    return camera.is_active();
};

struct res_rec {
    SceCameraResolution res;
    uint16_t width;
    uint16_t height;
};
const std::array<res_rec, 8> resolutions = { {
    { SCE_CAMERA_RESOLUTION_0_0, 0, 0 },
    { SCE_CAMERA_RESOLUTION_640_480, 640, 480 },
    { SCE_CAMERA_RESOLUTION_320_240, 320, 240 },
    { SCE_CAMERA_RESOLUTION_160_120, 160, 120 },
    { SCE_CAMERA_RESOLUTION_352_288, 352, 288 },
    { SCE_CAMERA_RESOLUTION_176_144, 176, 144 },
    { SCE_CAMERA_RESOLUTION_480_272, 480, 272 },
    { SCE_CAMERA_RESOLUTION_640_360, 640, 360 },
} };

int Camera::open(SceCameraInfo *info) {
    if (is_open) {
        return RET_ERROR(SCE_CAMERA_ERROR_ALREADY_OPEN);
    }
    this->is_open = true;
    for (auto res : resolutions) {
        if (res.res == info->resolution) {
            info->width = res.width;
            info->height = res.height;
            break;
        }
    }
    this->info = *info;
    return 0;
}

int Camera::start() {
    if (!is_open) {
        return RET_ERROR(SCE_CAMERA_ERROR_NOT_OPEN);
    }
    if (is_started) {
        return RET_ERROR(SCE_CAMERA_ERROR_ALREADY_START);
    }
    this->is_started = true;

    this->capture = std::make_unique<cv::VideoCapture>(0); // 0 represents the default webcam
    if (!capture->isOpened()) {
        // handle error
        return RET_ERROR(SCE_CAMERA_ERROR_FATAL);
    }
    this->frame_idx = 0;
    return 0;
}

std::string to_debug_str(SceCameraFormat type) {
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

int Camera::read(EmuEnvState &emuenv, SceCameraRead *read) {
    cv::Mat frame;

    capture->read(frame);
    if (frame.empty()) {
        read->status = SCE_CAMERA_STATUS_IS_ALREADY_READ; // hack
        return RET_ERROR(SCE_CAMERA_ERROR_FATAL);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / this->info.framerate));
    read->frame = frame_idx++;
    read->status = SCE_CAMERA_STATUS_IS_ACTIVE;
    read->timestamp = rtc_get_ticks(this->base_ticks);
    if (frame.rows != this->info.width || frame.cols != this->info.height)
        cv::resize(frame, frame, cv::Size(this->info.width, this->info.height));
    char *pIBase;
    char *pUBase;
    char *pVBase;
    SceSize sizeIBase;
    SceSize sizeUBase;
    SceSize sizeVBase;
    if (this->info.buffer == SCE_CAMERA_BUFFER_SETBYREAD) {
        pIBase = read->pIBase.cast<char>().get(emuenv.mem);
        pUBase = read->pUBase.cast<char>().get(emuenv.mem);
        pVBase = read->pVBase.cast<char>().get(emuenv.mem);
        sizeIBase = read->sizeIBase;
        sizeUBase = read->sizeUBase;
        sizeVBase = read->sizeVBase;
    } else {
        pIBase = this->info.pIBase.cast<char>().get(emuenv.mem);
        pUBase = this->info.pUBase.cast<char>().get(emuenv.mem);
        pVBase = this->info.pVBase.cast<char>().get(emuenv.mem);
        sizeIBase = this->info.sizeIBase;
        sizeUBase = this->info.sizeUBase;
        sizeVBase = this->info.sizeVBase;
    }
    if (this->info.format == SCE_CAMERA_FORMAT_YUV420_PLANE) {
        cv::Mat yuv;
        cv::cvtColor(frame, yuv, cv::COLOR_BGR2YUV_I420);
        memcpy(pIBase, yuv.data, sizeIBase);
        memcpy(pUBase, yuv.data + (this->info.width * this->info.height), sizeUBase);
        memcpy(pVBase, yuv.data + (this->info.width * this->info.height * 5 / 4), sizeVBase);
    } else if (this->info.format == SCE_CAMERA_FORMAT_YUV422_PLANE) {
        cv::Mat yuv;
        cv::cvtColor(frame, yuv, cv::COLOR_BGR2YUV_YV12);
        memcpy(pIBase, yuv.data, sizeIBase);
        memset(pUBase, 127, sizeUBase); // stub
        memset(pVBase, 127, sizeVBase);
        // memcpy(pUBase, yuv.data + (this->info.width * this->info.height), sizeUBase);
        // memcpy(pVBase, yuv.data + (this->info.width * this->info.height * 6 / 4), sizeVBase);
    } else if (this->info.format == SCE_CAMERA_FORMAT_ABGR) {
        cv::Mat yuv;
        cv::cvtColor(frame, yuv, cv::COLOR_BGR2RGBA);
        memcpy(pIBase, yuv.data, sizeIBase);
    } else {
        LOG_ERROR("format not supported {}", to_debug_str((SceCameraFormat)this->info.format));
    }
    return 0;
}

int Camera::is_active() {
    if (!is_open) {
        return RET_ERROR(SCE_CAMERA_ERROR_NOT_OPEN);
    }
    if (!is_started) {
        return RET_ERROR(SCE_CAMERA_ERROR_NOT_START);
    }
    return 1;
}

Camera::Camera() {
    this->capture = nullptr;
}

Camera::~Camera() {
    this->capture.reset();
}

int Camera::close() {
    if (!is_open) {
        return RET_ERROR(SCE_CAMERA_ERROR_NOT_OPEN);
    }
    this->is_started = false;
    this->is_open = false;
    return 0;
}

int Camera::stop() {
    if (!is_open) {
        return RET_ERROR(SCE_CAMERA_ERROR_NOT_OPEN);
    }
    if (!is_started) {
        return RET_ERROR(SCE_CAMERA_ERROR_NOT_START);
    }
    this->is_started = false;
    this->capture.reset();
    return 0;
}
