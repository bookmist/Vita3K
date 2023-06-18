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

#pragma once

#include "camera.h"

#include <SDL_gamecontroller.h>

struct CameraState {
    // Put variable used by app here
    SceCameraDevice active_camera{ SCE_CAMERA_DEVICE_UNKNOWN };
    Camera front_camera;
    Camera back_camera;
    Camera &get_camera(SceCameraDevice devnum);
    int open(SceCameraDevice devnum, SceCameraInfo *pInfo);
    int close(SceCameraDevice devnum);
    int start(SceCameraDevice devnum);
    int stop(SceCameraDevice devnum);
    int read(SceCameraDevice devnum, SceCameraRead *pRead);
    int is_active(SceCameraDevice devnum);
};
