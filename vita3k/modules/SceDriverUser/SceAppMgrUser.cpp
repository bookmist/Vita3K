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

#include "SceAppMgrUser.h"

#include <emuenv/app_util.h>
#include <io/device.h>
#include <io/functions.h>
#include <io/io.h>
#include <packages/functions.h>

#include <modules/module_parent.h>
#include <util/find.h>

#include <util/tracy.h>
TRACY_MODULE_NAME(SceAppMgrUser);

struct SceAppMgrSaveDataDataDelete {
    int size;
    unsigned int slotId;
    Ptr<SceAppUtilSaveDataSlotParam> slotParam;
    uint8_t reserved[32];
    Ptr<SceAppUtilSaveDataDataSaveItem> files;
    int fileNum;
    SceAppUtilMountPoint *mountPoint;
};

struct SceAppMgrSaveDataData {
    int size;
    unsigned int slotId;
    Ptr<SceAppUtilSaveDataSlotParam> slotParam;
    uint8_t reserved[32];
    Ptr<SceAppUtilSaveDataDataSaveItem> files;
    int fileNum;
    SceAppUtilMountPoint *mountPoint;
    unsigned int *requiredSizeKiB;
};

EXPORT(SceInt32, _sceAppMgrGetAppState, SceAppMgrAppState *appState, SceUInt32 sizeofSceAppMgrAppState, SceUInt32 buildVersion) {
    TRACY_FUNC(_sceAppMgrGetAppState, appState, sizeofSceAppMgrAppState, buildVersion);
    return CALL_EXPORT(__sceAppMgrGetAppState, appState, sizeofSceAppMgrAppState, buildVersion);
}

EXPORT(int, sceAppMgrAcidDirSet) {
    TRACY_FUNC(sceAppMgrAcidDirSet);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrAcquireSoundOutExclusive3) {
    TRACY_FUNC(sceAppMgrAcquireSoundOutExclusive3);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrAddContAddMount) {
    TRACY_FUNC(sceAppMgrAddContAddMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrAddContMount) {
    TRACY_FUNC(sceAppMgrAddContMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrAppDataMount, int mountId, char *mountPoint) {
    TRACY_FUNC(sceAppMgrAppDataMount, mountId, mountPoint);
    STUBBED("Using strcpy");
    switch (mountId) {
    case 0x64: // photo0:
        strcpy(mountPoint, "ux0:picture");
        break;
    case 0x65: // psnfriend
        fmt::format_to(mountPoint, "ur0:user/{}/psnfriend{}", emuenv.io.user_id, '\0');
        break;
    case 0x66: // psnmsg
        fmt::format_to(mountPoint, "ur0:user/{}/psnmsg{}", emuenv.io.user_id, '\0');
        break;
    case 0x67: // near
        fmt::format_to(mountPoint, "ur0:user/{}/near{}", emuenv.io.user_id, '\0');
        break;
    case 0x69: // music0:
        strcpy(mountPoint, "ux0:music");
        break;
    case 0x6C: // calendar
        strcpy(mountPoint, "ux0:calendar");
        break;
    case 0x6D: // video0:
        strcpy(mountPoint, "ux0:video");
        break;
    default:
        LOG_WARN("Unknown mountId {}", log_hex(mountId));
        break;
    }

    return 0;
}

EXPORT(int, sceAppMgrAppDataMountById, int id, const char *titleid, char *mount_point) {
    TRACY_FUNC(sceAppMgrAppDataMountById);
    LOG_DEBUG("id: {}, title id: {}", log_hex(id), titleid);
    switch (id) {
    case 0x6E: // appmeta/titleid
        emuenv.io.device_paths.appmeta0 = fmt::format("appmeta/{}", titleid);
        strcpy(mount_point, "appmeta0:");
        break;
    default:
        LOG_WARN("Unknown id {}", log_hex(id));
        break;
    }

    return 0;
}

EXPORT(int, sceAppMgrAppMount) {
    TRACY_FUNC(sceAppMgrAppMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrAppParamGetInt, int pid, int param, int *value) {
    TRACY_FUNC(sceAppMgrAppParamGetInt);
    LOG_DEBUG("pid: {}, param: {}", pid, log_hex(param));
    switch (param) {
    case 0x10:
        *value = 1;
        break;
    default:
        LOG_WARN("Unknown param {}", log_hex(param));
        break;
    }
    return STUBBED("");
}

EXPORT(SceInt32, sceAppMgrAppParamGetString, int pid, int param, char *string, int length) {
    TRACY_FUNC(sceAppMgrAppParamGetString, pid, param, string, length);
    return CALL_EXPORT(_sceAppMgrAppParamGetString, pid, param, string, length);
}

EXPORT(int, sceAppMgrAppParamSetString) {
    TRACY_FUNC(sceAppMgrAppParamSetString);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrAppUmount) {
    TRACY_FUNC(sceAppMgrAppUmount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrBgdlGetQueueStatus) {
    TRACY_FUNC(sceAppMgrBgdlGetQueueStatus);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrCaptureFrameBufDMACByAppId) {
    TRACY_FUNC(sceAppMgrCaptureFrameBufDMACByAppId);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrCaptureFrameBufIFTUByAppId) {
    TRACY_FUNC(sceAppMgrCaptureFrameBufIFTUByAppId);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrCheckRifGD) {
    TRACY_FUNC(sceAppMgrCheckRifGD);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrContentInstallPeriodStart) {
    TRACY_FUNC(sceAppMgrContentInstallPeriodStart);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrContentInstallPeriodStop) {
    TRACY_FUNC(sceAppMgrContentInstallPeriodStop);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrConvertVs0UserDrivePath, char *source_path, char *dest_path, int dest_len) {
    TRACY_FUNC(sceAppMgrConvertVs0UserDrivePath, source_path, dest_path, dest_len);
    STUBBED("Using strncpy");
    strncpy(dest_path, source_path, dest_len);
    return 0;
}

EXPORT(int, sceAppMgrDeclareShellProcess2, const char *name) {
    TRACY_FUNC(sceAppMgrDeclareShellProcess2);
    LOG_DEBUG("name: {}", name);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrDestroyAppByName, const char *name) {
    TRACY_FUNC(sceAppMgrDestroyAppByName, name);
    LOG_DEBUG("name:{}", to_debug_str(emuenv.mem, name));
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrDrmClose) {
    TRACY_FUNC(sceAppMgrDrmClose);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrDrmOpen) {
    TRACY_FUNC(sceAppMgrDrmOpen);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrForceUmount) {
    TRACY_FUNC(sceAppMgrForceUmount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGameDataMount, const char *app_path, const char *patch_path, const char *rif_path, char *mount_point) {
    TRACY_FUNC(sceAppMgrGameDataMount, app_path, patch_path, rif_path, mount_point);
    if (app_path && strlen(app_path) > 0) {
        emuenv.io.device_paths.gamedata0 = app_path;
        strcpy(mount_point, "gamedata0:");
    }

    if (patch_path || rif_path) {
        LOG_WARN("Patch and rif mounting is not yet implemented");
    }

    return 0;
}

EXPORT(int, sceAppMgrGetAppInfo) {
    TRACY_FUNC(sceAppMgrGetAppInfo);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetAppMgrState) {
    TRACY_FUNC(sceAppMgrGetAppMgrState);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetAppParam) {
    TRACY_FUNC(sceAppMgrGetAppParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetAppParam2) {
    TRACY_FUNC(sceAppMgrGetAppParam2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetBootParam) {
    TRACY_FUNC(sceAppMgrGetBootParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetBudgetInfo) {
    TRACY_FUNC(sceAppMgrGetBudgetInfo);
    STUBBED("not system mode");
    return -1;
}

EXPORT(int, sceAppMgrGetCoredumpStateForShell) {
    TRACY_FUNC(sceAppMgrGetCoredumpStateForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetCurrentBgmState) {
    TRACY_FUNC(sceAppMgrGetCurrentBgmState);
    return UNIMPLEMENTED();
}

struct BgmState {
    uint32_t unk_0;
    uint32_t unk2;
    int unk3;
    int unk4;
    int unk5;
    int unk6;
    int unk7;
    int unk8;
};

EXPORT(int, sceAppMgrGetCurrentBgmState2, BgmState *state) {
    TRACY_FUNC(sceAppMgrGetCurrentBgmState2);
    memset(state, 0, sizeof(BgmState));
    state->unk_0 = 130;
    //state->unk2 = 130;
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetDevInfo, const char *dev, uint64_t *max_size, uint64_t *free_size) {
    TRACY_FUNC(sceAppMgrGetDevInfo, dev, max_size, free_size);
    if (dev == nullptr) {
        return RET_ERROR(SCE_ERROR_ERRNO_EINVAL);
    }

    auto device = device::get_device(dev);
    if (device == VitaIoDevice::_INVALID) {
        LOG_ERROR("Cannot find device for path: {}", dev);
        return RET_ERROR(SCE_ERROR_ERRNO_ENOENT);
    }

    fs::path dev_path = device._to_string();
    fs::path path = emuenv.pref_path / dev_path;
    fs::space_info space = fs::space(path);

    // TODO: Use free or available?
    // free = free space available on the whole partition
    // available = free space available to a non-privileged process
    // Using available in case the drive is nearly full and the game tries to write since available will always be smaller
    *free_size = space.available;
    *max_size = space.capacity;

    return 0;
}

EXPORT(int, sceAppMgrGetFgAppInfo) {
    TRACY_FUNC(sceAppMgrGetFgAppInfo);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetIdByName) {
    TRACY_FUNC(sceAppMgrGetIdByName);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetMediaTypeFromDrive) {
    TRACY_FUNC(sceAppMgrGetMediaTypeFromDrive);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetMediaTypeFromDriveByPid) {
    TRACY_FUNC(sceAppMgrGetMediaTypeFromDriveByPid);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetMountProcessNum) {
    TRACY_FUNC(sceAppMgrGetMountProcessNum);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetNameById) {
    TRACY_FUNC(sceAppMgrGetNameById);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetPfsDrive) {
    TRACY_FUNC(sceAppMgrGetPfsDrive);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetPidListForShell) {
    TRACY_FUNC(sceAppMgrGetPidListForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetRawPath, char *path, char *resolved_path, int resolved_path_size) {
    TRACY_FUNC(sceAppMgrGetRawPath, resolved_path, path, resolved_path_size);
    STUBBED("Using strncpy");
    strncpy(resolved_path, path, resolved_path_size);
    return 0;
}

EXPORT(int, sceAppMgrGetRawPathOfApp0ByAppIdForShell) {
    TRACY_FUNC(sceAppMgrGetRawPathOfApp0ByAppIdForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetRawPathOfApp0ByPidForShell) {
    TRACY_FUNC(sceAppMgrGetRawPathOfApp0ByPidForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetRecommendedScreenOrientation) {
    TRACY_FUNC(sceAppMgrGetRecommendedScreenOrientation);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetRunningAppIdListForShell) {
    TRACY_FUNC(sceAppMgrGetRunningAppIdListForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetSaveDataInfo) {
    TRACY_FUNC(sceAppMgrGetSaveDataInfo);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetSaveDataInfoForSpecialExport) {
    TRACY_FUNC(sceAppMgrGetSaveDataInfoForSpecialExport);
    return UNIMPLEMENTED();
}

struct SceAppMgrAppStatus { // size is 0x80
    SceUInt32 unk_0; // 0x0
    SceUInt32 launchMode; // 0x4
    SceUInt32 bgm_priority_or_status; // 0x8
    char appName[32]; // 0xC
    SceUInt32 unk_2c; // 0x2C
    SceUID appId; // 0x30 - Application ID
    SceUID processId; // 0x34 - Process ID
    SceUInt32 status_related_1; // 0x38
    SceUInt32 status_related_2; // 0x3C
    SceUInt32 unk_40;
    SceUInt32 unk_44;
    SceUInt32 unk_48;
    SceUInt32 unk_4c;
    char name_02[32];
    char unk_70;
    uint8_t unk_71;
    char unk_72;
    char unk_73;
    uint32_t unk_74;
    uint32_t unk_78;
    uint32_t unk_7c;
};

EXPORT(int, sceAppMgrGetStatusByAppId, int id, SceAppMgrAppStatus *appStatus) {
    TRACY_FUNC(sceAppMgrGetStatusByAppId);
    LOG_DEBUG("id: {}", id);
    if (appStatus) {
        memset(appStatus, 0, sizeof(SceAppMgrAppStatus));
        appStatus->unk_71 = 2;
        appStatus->unk_74 = 1;
        appStatus->status_related_1 = 2;

        appStatus->launchMode = 2;
        appStatus->unk_0 = 2;
        appStatus->unk_2c = 2;
        appStatus->bgm_priority_or_status = 1;
        strncpy(appStatus->appName, emuenv.io.title_id.c_str(), sizeof(appStatus->appName));
        appStatus->status_related_2 = 2;
        appStatus->unk_40 = 2;
        appStatus->unk_44 = 2;
        appStatus->unk_48 = 2;
        appStatus->unk_4c = 2;
        strncpy(appStatus->name_02, emuenv.io.title_id.c_str(), sizeof(appStatus->name_02));
        appStatus->unk_70 = 1;
        appStatus->unk_78 = 2;
        appStatus->unk_7c = 2;
    }

    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetStatusById) {
    TRACY_FUNC(sceAppMgrGetStatusById);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetStatusByName, const char *appName, SceAppMgrAppStatus *appStatus) {
    TRACY_FUNC(sceAppMgrGetStatusByName);
    LOG_DEBUG_IF(appName, "name: {}", appName);
    // LOG_DEBUG("Called");
    if (appStatus) {
        memset(appStatus, 0, sizeof(SceAppMgrAppStatus));
        appStatus->unk_71 = 2;
        appStatus->unk_74 = 1;
        appStatus->status_related_1 = 2;

        appStatus->launchMode = 2;
        appStatus->unk_0 = 2;
        appStatus->unk_2c = 2;
        appStatus->bgm_priority_or_status = 1;
        strncpy(appStatus->appName, emuenv.io.title_id.c_str(), sizeof(appStatus->appName));
        appStatus->status_related_2 = 2;
        appStatus->unk_40 = 2;
        appStatus->unk_44 = 2;
        appStatus->unk_48 = 2;
        appStatus->unk_4c = 2;
        strncpy(appStatus->name_02, emuenv.io.title_id.c_str(), sizeof(appStatus->name_02));
        appStatus->unk_70 = 1;
        appStatus->unk_78 = 2;
        appStatus->unk_7c = 2;
    }

    return 0;
}

EXPORT(int, sceAppMgrGetSystemDataFilePlayReady) {
    TRACY_FUNC(sceAppMgrGetSystemDataFilePlayReady);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetUserDirPath, int partition_id, char *userDirPath, SceSize path_maxlen) {
    TRACY_FUNC(sceAppMgrGetUserDirPath, partition_id, userDirPath, path_maxlen);
    STUBBED("Using strncpy");
    const auto partition = partition_id == 1 ? "ur0:" : "ux0:";
    fmt::format_to_n(userDirPath, path_maxlen, "{}user/{}{}", partition, emuenv.io.user_id, '\0');
    return 0;
}

EXPORT(int, sceAppMgrGetUserDirPathById) {
    TRACY_FUNC(sceAppMgrGetUserDirPathById);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetVs0UserDataDrive) {
    TRACY_FUNC(sceAppMgrGetVs0UserDataDrive);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrGetVs0UserModuleDrive) {
    TRACY_FUNC(sceAppMgrGetVs0UserModuleDrive);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrInitSafeMemoryById) {
    TRACY_FUNC(sceAppMgrInitSafeMemoryById);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrInstallDirMount) {
    TRACY_FUNC(sceAppMgrInstallDirMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrIsCameraActive) {
    TRACY_FUNC(sceAppMgrIsCameraActive);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLaunchAppByName, int flags, const char *name, const char *param) {
    TRACY_FUNC(sceAppMgrLaunchAppByName);
    LOG_DEBUG("flags: {}, name: {}, param: {}", log_hex(flags), name, param ? param : "null");

    return -1; // load_app_by_path(emuenv, "vs0:app/" + std::string(name) + "/eboot.bin", name, param);
}

EXPORT(int, sceAppMgrLaunchAppByName2) {
    TRACY_FUNC(sceAppMgrLaunchAppByName2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLaunchAppByName2ForShell) {
    TRACY_FUNC(sceAppMgrLaunchAppByName2ForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLaunchAppByName2ndStage) {
    TRACY_FUNC(sceAppMgrLaunchAppByName2ndStage);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLaunchAppByNameForShell) {
    TRACY_FUNC(sceAppMgrLaunchAppByNameForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLaunchAppByPath4, const char *path, const char *titleid, int a3, Ptr<char> const app_param[], int a5, void *opt) {
    TRACY_FUNC(sceAppMgrLaunchAppByPath4);
    LOG_DEBUG("path: {}, titleid: {}, a3: {}, app param: {}, a5: {}", path, titleid, a3, app_param ? app_param->get(emuenv.mem) : "null", a5);
    emuenv.io.app_path = titleid;
    return CALL_EXPORT(_sceAppMgrLoadExec, path, app_param, nullptr);
    //return load_app_by_path(emuenv, path, titleid, app_param);
}

EXPORT(int, sceAppMgrLaunchAppByUri, const int uri) {
    TRACY_FUNC(sceAppMgrLaunchAppByUri);
    LOG_DEBUG("uri: {}", uri);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLaunchAppByUri2) {
    TRACY_FUNC(sceAppMgrLaunchAppByUri2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLaunchVideoStreamingApp) {
    TRACY_FUNC(sceAppMgrLaunchVideoStreamingApp);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceAppMgrLoadExec, const char *appPath, Ptr<char> const argv[], const SceAppMgrLoadExecOptParam *optParam) {
    TRACY_FUNC(sceAppMgrLoadExec, appPath, argv, optParam);
    return CALL_EXPORT(_sceAppMgrLoadExec, appPath, argv, optParam);
}

EXPORT(int, sceAppMgrLoadSaveDataSystemFile) {
    TRACY_FUNC(sceAppMgrLoadSaveDataSystemFile);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLoopBackFormat) {
    TRACY_FUNC(sceAppMgrLoopBackFormat);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrLoopBackMount, const char *mountpoint, char *mounted) {
    TRACY_FUNC(sceAppMgrLoopBackMount);
    // LOG_DEBUG("mountpmoint: {}, mounted: {}", mountpoint, mounted);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrMmsMount, int id, char *mount_point) {
    TRACY_FUNC(sceAppMgrMmsMount, id, mount_point);
    switch (id) {
    case 0x190:
        strcpy(mount_point, "ux0:mms/photo/");
        break;
    case 0x191:
        strcpy(mount_point, "ux0:mms/music");
        break;
    case 0x192:
        strcpy(mount_point, "ux0:mms/video");
        break;
    default:
        LOG_WARN("Unknown id: {}", log_hex(id));
        break;
    }
    return STUBBED("using strcpy");
}

EXPORT(int, sceAppMgrOverwriteLaunchParamForShell) {
    TRACY_FUNC(sceAppMgrOverwriteLaunchParamForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrPeekLaunchParamForShell) {
    TRACY_FUNC(sceAppMgrPeekLaunchParamForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrPhotoMount) {
    TRACY_FUNC(sceAppMgrPhotoMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrPhotoUmount) {
    TRACY_FUNC(sceAppMgrPhotoUmount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrPspSaveDataGetParams) {
    TRACY_FUNC(sceAppMgrPspSaveDataGetParams);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrPspSaveDataRead) {
    TRACY_FUNC(sceAppMgrPspSaveDataRead);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrPspSaveDataRootMount) {
    TRACY_FUNC(sceAppMgrPspSaveDataRootMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrReceiveEvent, SceAppMgrEvent *mgrEvent) {
    TRACY_FUNC(sceAppMgrReceiveEvent, mgrEvent);
    mgrEvent->mgrEvent = SCE_APPMGR_EVENT_ON_RESUME;
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrReceiveEventNum, SceUInt32 *eventNum) {
    TRACY_FUNC(sceAppMgrReceiveEventNum, eventNum);

    // Vita3K does not yet manage events    
    *eventNum = 0;

    return STUBBED("Set eventNum to 0");
}

struct NotifForShell {
    SceUInt32 size;
    char msg[32];
};

EXPORT(int, sceAppMgrReceiveNotificationRequestForShell, NotifForShell *notif) {
    TRACY_FUNC(sceAppMgrReceiveNotificationRequestForShell);
    // LOG_DEBUG("size: {}", notif->size);
    // memset(notif, 0, sizeof(NotifForShell));
    // strcpy(notif->msg, "Vita3K forever <3");
    return 0x80802024;
}

EXPORT(int, sceAppMgrReceiveShellEvent, SceAppMgrEvent *mgrEvent) {
    TRACY_FUNC(sceAppMgrReceiveShellEvent, mgrEvent);
    // mgrEvent->mgrEvent = SCE_APPMGR_EVENT_ON_RESUME;
    LOG_DEBUG("call shell event");
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrReceiveSystemEvent, SceAppMgrSystemEvent *systemEvent) {
    TRACY_FUNC(sceAppMgrReceiveSystemEvent, systemEvent);
    systemEvent->systemEvent = SCE_APPMGR_SYSTEMEVENT_ON_RESUME;
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataAddMount) {
    TRACY_FUNC(sceAppMgrSaveDataAddMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataDataRemove, Ptr<SceAppMgrSaveDataDataDelete> data) {
    TRACY_FUNC(sceAppMgrSaveDataDataRemove, data);
    for (int i = 0; i < data.get(emuenv.mem)->fileNum; i++) {
        const auto file = fs::path(construct_savedata0_path(data.get(emuenv.mem)->files.get(emuenv.mem)[i].dataPath.get(emuenv.mem)));
        if (fs::is_regular_file(file)) {
            remove_file(emuenv.io, file.string().c_str(), emuenv.pref_path, export_name);
        } else
            remove_dir(emuenv.io, file.string().c_str(), emuenv.pref_path, export_name);
    }

    return 0;
}

EXPORT(int, sceAppMgrSaveDataDataRemove2) {
    TRACY_FUNC(sceAppMgrSaveDataDataRemove2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataDataSave, Ptr<SceAppMgrSaveDataData> data) {
    TRACY_FUNC(sceAppMgrSaveDataDataSave, data);
    SceUID fd;

    for (int i = 0; i < data.get(emuenv.mem)->fileNum; i++) {
        auto files = data.get(emuenv.mem)->files.get(emuenv.mem);

        const auto file_path = construct_savedata0_path(files[i].dataPath.get(emuenv.mem));
        switch (files[i].mode) {
        case SCE_APPUTIL_SAVEDATA_DATA_SAVE_MODE_DIRECTORY:
            create_dir(emuenv.io, file_path.c_str(), 0777, emuenv.pref_path, export_name);
            break;
        case SCE_APPUTIL_SAVEDATA_DATA_SAVE_MODE_FILE_TRUNCATE:
            if (files[i].buf) {
                fd = open_file(emuenv.io, file_path.c_str(), SCE_O_WRONLY | SCE_O_CREAT, emuenv.pref_path, export_name);
                seek_file(fd, static_cast<int>(files[i].offset), SCE_SEEK_SET, emuenv.io, export_name);
                write_file(fd, files[i].buf.get(emuenv.mem), files[i].bufSize, emuenv.io, export_name);
                close_file(emuenv.io, fd, export_name);
            }
            fd = open_file(emuenv.io, file_path.c_str(), SCE_O_WRONLY | SCE_O_APPEND | SCE_O_TRUNC, emuenv.pref_path, export_name);
            truncate_file(fd, files[i].bufSize + files[i].offset, emuenv.io, export_name);
            close_file(emuenv.io, fd, export_name);
            break;
        case SCE_APPUTIL_SAVEDATA_DATA_SAVE_MODE_FILE:
        default:
            fd = open_file(emuenv.io, file_path.c_str(), SCE_O_WRONLY | SCE_O_CREAT, emuenv.pref_path, export_name);
            seek_file(fd, static_cast<int>(files[i].offset), SCE_SEEK_SET, emuenv.io, export_name);
            if (files[i].buf.get(emuenv.mem)) {
                write_file(fd, files[i].buf.get(emuenv.mem), files[i].bufSize, emuenv.io, export_name);
            }
            close_file(emuenv.io, fd, export_name);
            break;
        }
    }
    return 0;
}

EXPORT(int, sceAppMgrSaveDataDataSave2) {
    TRACY_FUNC(sceAppMgrSaveDataDataSave2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataGetQuota) {
    TRACY_FUNC(sceAppMgrSaveDataGetQuota);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataMount) {
    TRACY_FUNC(sceAppMgrSaveDataMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotCreate) {
    TRACY_FUNC(sceAppMgrSaveDataSlotCreate);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotDelete) {
    TRACY_FUNC(sceAppMgrSaveDataSlotDelete);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotFileClose) {
    TRACY_FUNC(sceAppMgrSaveDataSlotFileClose);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotFileGetParam) {
    TRACY_FUNC(sceAppMgrSaveDataSlotFileGetParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotFileOpen) {
    TRACY_FUNC(sceAppMgrSaveDataSlotFileOpen);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotGetParam) {
    TRACY_FUNC(sceAppMgrSaveDataSlotGetParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotGetStatus) {
    TRACY_FUNC(sceAppMgrSaveDataSlotGetStatus);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotInit) {
    TRACY_FUNC(sceAppMgrSaveDataSlotInit);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotSetParam) {
    TRACY_FUNC(sceAppMgrSaveDataSlotSetParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataSlotSetStatus) {
    TRACY_FUNC(sceAppMgrSaveDataSlotSetStatus);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSaveDataUmount) {
    TRACY_FUNC(sceAppMgrSaveDataUmount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSendNotificationRequest, NotifForShell *notif) {
    TRACY_FUNC(sceAppMgrSendNotificationRequest);
    // if (notif)
    //     LOG_DEBUG("Notif: {}", notif->msg);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSendParam) {
    TRACY_FUNC(sceAppMgrSendParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSendSystemEvent) {
    TRACY_FUNC(sceAppMgrSendSystemEvent);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSendSystemEvent2) {
    TRACY_FUNC(sceAppMgrSendSystemEvent2);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetBackRenderPortOwner) {
    TRACY_FUNC(sceAppMgrSetBackRenderPortOwner);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetBgmProxyApp) {
    TRACY_FUNC(sceAppMgrSetBgmProxyApp);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetNetworkDisconnectionWarningDialogState) {
    TRACY_FUNC(sceAppMgrSetNetworkDisconnectionWarningDialogState);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetPowerSaveMode) {
    TRACY_FUNC(sceAppMgrSetPowerSaveMode);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetRecommendedScreenOrientationForShell) {
    TRACY_FUNC(sceAppMgrSetRecommendedScreenOrientationForShell);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetShellScreenOrientation) {
    TRACY_FUNC(sceAppMgrSetShellScreenOrientation);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetSystemDataFile) {
    TRACY_FUNC(sceAppMgrSetSystemDataFile);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSetSystemDataFilePlayReady) {
    TRACY_FUNC(sceAppMgrSetSystemDataFilePlayReady);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSystemParamDateTimeGetConf) {
    TRACY_FUNC(sceAppMgrSystemParamDateTimeGetConf);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSystemParamGetInt) {
    TRACY_FUNC(sceAppMgrSystemParamGetInt);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrSystemParamGetString) {
    TRACY_FUNC(sceAppMgrSystemParamGetString);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrThemeDataMount) {
    TRACY_FUNC(sceAppMgrThemeDataMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrTrophyMount) {
    TRACY_FUNC(sceAppMgrTrophyMount);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrTrophyMountById) {
    TRACY_FUNC(sceAppMgrTrophyMountById);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrUmount, const char *mount_point) {
    TRACY_FUNC(sceAppMgrUmount, mount_point);
    if (std::string(mount_point) == "gamedata0:") {
        emuenv.io.device_paths.gamedata0.clear();

        return 0;
    } else if (std::string(mount_point) == "appmeta0:") {
        emuenv.io.device_paths.appmeta0.clear();

        return 0;
    }

    LOG_DEBUG("Unknown mount point: {}", mount_point);

    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrUmountByPid) {
    TRACY_FUNC(sceAppMgrUmountByPid);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrUpdateSaveDataParam) {
    TRACY_FUNC(sceAppMgrUpdateSaveDataParam);
    return UNIMPLEMENTED();
}

EXPORT(int, sceAppMgrWorkDirMount, int mountId, char *mountPoint) {
    TRACY_FUNC(sceAppMgrWorkDirMount, mountId, mountPoint);
    STUBBED("using strcpy");
    switch (mountId) {
    case 0xC8:
        strcpy(mountPoint, "ur0:temp/sqlite/");
        break;
    case 0xC9:
        strcpy(mountPoint, "ur0:temp/attach/");
        break;
    case 0xCA:
        strcpy(mountPoint, "ux0:pspemu/");
        break;
    case 0xCC:
        strcpy(mountPoint, "ur0:temp/checkout/");
        break;
    case 0xCE:
        strcpy(mountPoint, "ur0:temp/webbrowser/");
        break;
    default:
        LOG_WARN("Unknown mount id: {}", log_hex(mountId));
        break;
    }

    return 0;
}

EXPORT(int, sceAppMgrWorkDirMountById, const int mountId, char *mount_point) {
    TRACY_FUNC(sceAppMgrWorkDirMountById);
    LOG_DEBUG("mountId: {}", log_hex(mountId));
    switch (mountId) {
    case 0xCD:
        strcpy(mount_point, "ux0:cache");
        break;
    default:
        LOG_WARN("Unknown mount id: {}", log_hex(mountId));
        break;
    }
    return 0;
}

BRIDGE_IMPL(_sceAppMgrGetAppState)
BRIDGE_IMPL(sceAppMgrAcidDirSet)
BRIDGE_IMPL(sceAppMgrAcquireSoundOutExclusive3)
BRIDGE_IMPL(sceAppMgrAddContAddMount)
BRIDGE_IMPL(sceAppMgrAddContMount)
BRIDGE_IMPL(sceAppMgrAppDataMount)
BRIDGE_IMPL(sceAppMgrAppDataMountById)
BRIDGE_IMPL(sceAppMgrAppMount)
BRIDGE_IMPL(sceAppMgrAppParamGetInt)
BRIDGE_IMPL(sceAppMgrAppParamGetString)
BRIDGE_IMPL(sceAppMgrAppParamSetString)
BRIDGE_IMPL(sceAppMgrAppUmount)
BRIDGE_IMPL(sceAppMgrBgdlGetQueueStatus)
BRIDGE_IMPL(sceAppMgrCaptureFrameBufDMACByAppId)
BRIDGE_IMPL(sceAppMgrCaptureFrameBufIFTUByAppId)
BRIDGE_IMPL(sceAppMgrCheckRifGD)
BRIDGE_IMPL(sceAppMgrContentInstallPeriodStart)
BRIDGE_IMPL(sceAppMgrContentInstallPeriodStop)
BRIDGE_IMPL(sceAppMgrConvertVs0UserDrivePath)
BRIDGE_IMPL(sceAppMgrDeclareShellProcess2)
BRIDGE_IMPL(sceAppMgrDestroyAppByName)
BRIDGE_IMPL(sceAppMgrDrmClose)
BRIDGE_IMPL(sceAppMgrDrmOpen)
BRIDGE_IMPL(sceAppMgrForceUmount)
BRIDGE_IMPL(sceAppMgrGameDataMount)
BRIDGE_IMPL(sceAppMgrGetAppInfo)
BRIDGE_IMPL(sceAppMgrGetAppMgrState)
BRIDGE_IMPL(sceAppMgrGetAppParam)
BRIDGE_IMPL(sceAppMgrGetAppParam2)
BRIDGE_IMPL(sceAppMgrGetBootParam)
BRIDGE_IMPL(sceAppMgrGetBudgetInfo)
BRIDGE_IMPL(sceAppMgrGetCoredumpStateForShell)
BRIDGE_IMPL(sceAppMgrGetCurrentBgmState)
BRIDGE_IMPL(sceAppMgrGetCurrentBgmState2)
BRIDGE_IMPL(sceAppMgrGetDevInfo)
BRIDGE_IMPL(sceAppMgrGetFgAppInfo)
BRIDGE_IMPL(sceAppMgrGetIdByName)
BRIDGE_IMPL(sceAppMgrGetMediaTypeFromDrive)
BRIDGE_IMPL(sceAppMgrGetMediaTypeFromDriveByPid)
BRIDGE_IMPL(sceAppMgrGetMountProcessNum)
BRIDGE_IMPL(sceAppMgrGetNameById)
BRIDGE_IMPL(sceAppMgrGetPfsDrive)
BRIDGE_IMPL(sceAppMgrGetPidListForShell)
BRIDGE_IMPL(sceAppMgrGetRawPath)
BRIDGE_IMPL(sceAppMgrGetRawPathOfApp0ByAppIdForShell)
BRIDGE_IMPL(sceAppMgrGetRawPathOfApp0ByPidForShell)
BRIDGE_IMPL(sceAppMgrGetRecommendedScreenOrientation)
BRIDGE_IMPL(sceAppMgrGetRunningAppIdListForShell)
BRIDGE_IMPL(sceAppMgrGetSaveDataInfo)
BRIDGE_IMPL(sceAppMgrGetSaveDataInfoForSpecialExport)
BRIDGE_IMPL(sceAppMgrGetStatusByAppId)
BRIDGE_IMPL(sceAppMgrGetStatusById)
BRIDGE_IMPL(sceAppMgrGetStatusByName)
BRIDGE_IMPL(sceAppMgrGetSystemDataFilePlayReady)
BRIDGE_IMPL(sceAppMgrGetUserDirPath)
BRIDGE_IMPL(sceAppMgrGetUserDirPathById)
BRIDGE_IMPL(sceAppMgrGetVs0UserDataDrive)
BRIDGE_IMPL(sceAppMgrGetVs0UserModuleDrive)
BRIDGE_IMPL(sceAppMgrInitSafeMemoryById)
BRIDGE_IMPL(sceAppMgrInstallDirMount)
BRIDGE_IMPL(sceAppMgrIsCameraActive)
BRIDGE_IMPL(sceAppMgrLaunchAppByName)
BRIDGE_IMPL(sceAppMgrLaunchAppByName2)
BRIDGE_IMPL(sceAppMgrLaunchAppByName2ForShell)
BRIDGE_IMPL(sceAppMgrLaunchAppByName2ndStage)
BRIDGE_IMPL(sceAppMgrLaunchAppByNameForShell)
BRIDGE_IMPL(sceAppMgrLaunchAppByPath4)
BRIDGE_IMPL(sceAppMgrLaunchAppByUri)
BRIDGE_IMPL(sceAppMgrLaunchAppByUri2)
BRIDGE_IMPL(sceAppMgrLaunchVideoStreamingApp)
BRIDGE_IMPL(sceAppMgrLoadExec)
BRIDGE_IMPL(sceAppMgrLoadSaveDataSystemFile)
BRIDGE_IMPL(sceAppMgrLoopBackFormat)
BRIDGE_IMPL(sceAppMgrLoopBackMount)
BRIDGE_IMPL(sceAppMgrMmsMount)
BRIDGE_IMPL(sceAppMgrOverwriteLaunchParamForShell)
BRIDGE_IMPL(sceAppMgrPeekLaunchParamForShell)
BRIDGE_IMPL(sceAppMgrPhotoMount)
BRIDGE_IMPL(sceAppMgrPhotoUmount)
BRIDGE_IMPL(sceAppMgrPspSaveDataGetParams)
BRIDGE_IMPL(sceAppMgrPspSaveDataRead)
BRIDGE_IMPL(sceAppMgrPspSaveDataRootMount)
BRIDGE_IMPL(sceAppMgrReceiveEvent)
BRIDGE_IMPL(sceAppMgrReceiveEventNum)
BRIDGE_IMPL(sceAppMgrReceiveNotificationRequestForShell)
BRIDGE_IMPL(sceAppMgrReceiveShellEvent)
BRIDGE_IMPL(sceAppMgrReceiveSystemEvent)
BRIDGE_IMPL(sceAppMgrSaveDataAddMount)
BRIDGE_IMPL(sceAppMgrSaveDataDataRemove)
BRIDGE_IMPL(sceAppMgrSaveDataDataRemove2)
BRIDGE_IMPL(sceAppMgrSaveDataDataSave)
BRIDGE_IMPL(sceAppMgrSaveDataDataSave2)
BRIDGE_IMPL(sceAppMgrSaveDataGetQuota)
BRIDGE_IMPL(sceAppMgrSaveDataMount)
BRIDGE_IMPL(sceAppMgrSaveDataSlotCreate)
BRIDGE_IMPL(sceAppMgrSaveDataSlotDelete)
BRIDGE_IMPL(sceAppMgrSaveDataSlotFileClose)
BRIDGE_IMPL(sceAppMgrSaveDataSlotFileGetParam)
BRIDGE_IMPL(sceAppMgrSaveDataSlotFileOpen)
BRIDGE_IMPL(sceAppMgrSaveDataSlotGetParam)
BRIDGE_IMPL(sceAppMgrSaveDataSlotGetStatus)
BRIDGE_IMPL(sceAppMgrSaveDataSlotInit)
BRIDGE_IMPL(sceAppMgrSaveDataSlotSetParam)
BRIDGE_IMPL(sceAppMgrSaveDataSlotSetStatus)
BRIDGE_IMPL(sceAppMgrSaveDataUmount)
BRIDGE_IMPL(sceAppMgrSendNotificationRequest)
BRIDGE_IMPL(sceAppMgrSendParam)
BRIDGE_IMPL(sceAppMgrSendSystemEvent)
BRIDGE_IMPL(sceAppMgrSendSystemEvent2)
BRIDGE_IMPL(sceAppMgrSetBackRenderPortOwner)
BRIDGE_IMPL(sceAppMgrSetBgmProxyApp)
BRIDGE_IMPL(sceAppMgrSetNetworkDisconnectionWarningDialogState)
BRIDGE_IMPL(sceAppMgrSetPowerSaveMode)
BRIDGE_IMPL(sceAppMgrSetRecommendedScreenOrientationForShell)
BRIDGE_IMPL(sceAppMgrSetShellScreenOrientation)
BRIDGE_IMPL(sceAppMgrSetSystemDataFile)
BRIDGE_IMPL(sceAppMgrSetSystemDataFilePlayReady)
BRIDGE_IMPL(sceAppMgrSystemParamDateTimeGetConf)
BRIDGE_IMPL(sceAppMgrSystemParamGetInt)
BRIDGE_IMPL(sceAppMgrSystemParamGetString)
BRIDGE_IMPL(sceAppMgrThemeDataMount)
BRIDGE_IMPL(sceAppMgrTrophyMount)
BRIDGE_IMPL(sceAppMgrTrophyMountById)
BRIDGE_IMPL(sceAppMgrUmount)
BRIDGE_IMPL(sceAppMgrUmountByPid)
BRIDGE_IMPL(sceAppMgrUpdateSaveDataParam)
BRIDGE_IMPL(sceAppMgrWorkDirMount)
BRIDGE_IMPL(sceAppMgrWorkDirMountById)
