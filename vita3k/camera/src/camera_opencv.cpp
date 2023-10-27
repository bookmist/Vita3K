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
#define export_name __FUNCTION__

#define USE_OPENCV 1

#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
#endif // USE_OPENCV

int Camera::open(SceCameraInfo *info) {
    this->is_open = true;
    this->info = *info;
    return 0;
}

int Camera::start() {
    this->is_started = true;
#ifdef USE_OPENCV
    this->capture = std::make_unique<cv::VideoCapture>(0); // 0 represents the default webcam
    if (!capture->isOpened()) {
        // handle error
        return RET_ERROR(SCE_CAMERA_ERROR_FATAL);
    }
#endif // USE_OPENCV
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

int Camera::read(EmuEnvState &emuenv, SceCameraRead *read, void *pIBase, void *pUBase, void *pVBase, SceSize sizeIBase, SceSize sizeUBase, SceSize sizeVBase) {
    read->frame = frame_idx++;
    read->status = SCE_CAMERA_STATUS_IS_ACTIVE;
    read->timestamp = rtc_get_ticks(this->base_ticks);
#ifdef USE_OPENCV
    cv::Mat frame;

    capture->read(frame);
    if (frame.empty()) {
        read->status = SCE_CAMERA_STATUS_IS_ALREADY_READ; // hack
        return RET_ERROR(SCE_CAMERA_ERROR_FATAL);
    }
    if (frame.rows != this->info.width || frame.cols != this->info.height)
        cv::resize(frame, frame, cv::Size(this->info.width, this->info.height));
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
#endif // USE_OPENCV
    return 0;
}

int Camera::close() {
    this->is_started = false;
    this->is_open = false;
    return 0;
}

int Camera::stop() {
    this->is_started = false;
#ifdef USE_OPENCV
    this->capture.reset();
#endif // USE_OPENCV
    return 0;
}
