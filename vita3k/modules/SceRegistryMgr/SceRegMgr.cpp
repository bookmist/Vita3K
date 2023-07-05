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

#include "SceRegMgr.h"

#include <util/tracy.h>
TRACY_MODULE_NAME(SceRegMgr);

EXPORT(int, sceRegMgrAddRegistryCallback) {
    TRACY_FUNC(sceRegMgrAddRegistryCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrDbBackup) {
    TRACY_FUNC(sceRegMgrDbBackup);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrDbRestore) {
    TRACY_FUNC(sceRegMgrDbRestore);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrGetInitVals) {
    TRACY_FUNC(sceRegMgrGetInitVals);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrGetKeyBin, const char *category, const char *name, void *buf, int size) {
    TRACY_FUNC(sceRegMgrGetKeyBin);
    LOG_DEBUG("category: {}, name: {}, size: {}", category, name, size);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrGetKeyInt, const char *category, const char *name, int *buf) {
    TRACY_FUNC(sceRegMgrGetKeyInt, category, name, buf);
    if ((std::string(category) == "/CONFIG/PSPEMU") && (std::string(name) == "emu_list_flag")) {
        STUBBED("Stubbed");
        *buf = 1;

        return 0;
    } else if (std::string(category) == "/CONFIG/SYSTEM") {
        if (std::string(name) == "initialize") {
            LOG_WARN("Using init done");
            *buf = 1;
            return 0;
        } else if (std::string(name) == "language") {
            LOG_WARN("using Cfg value of: {}/{}", category, name);
            *buf = emuenv.cfg.sys_lang;
            return 0;
        }
    } else if (std::string(category) == "/CONFIG/SHELL") {
        if (std::string(name) == "shell_auto_create_folder") {
            *buf = 0;
            LOG_WARN("using default value of: {}/{}", category, name);
            return 0;
        } else if (std::string(name) == "playlog_clock") {
            *buf = 0;
            LOG_WARN("using default value of: {}/{}", category, name);
            return 0;
        }
    } else if (std::string(category) == "/CONFIG/DATE") {
        if (std::string(name) == "time_zone") {
            *buf = 12;
            LOG_WARN("using default value of: {}/{}", category, name);
            return 0;
        } else if (std::string(name) == "date_format") {
            *buf = emuenv.cfg.sys_date_format;
            LOG_WARN("using cfg value of date_format");
            return 0;
        } else if (std::string(name) == "time_format") {
            *buf = emuenv.cfg.sys_time_format;
            LOG_WARN("using cfg value of time_format");
            return 0;
        } else if (std::string(name) == "auto_summer_time") {
            *buf = 1;
            LOG_WARN("using default value of: {}/{}", category, name);
            return 0;
        } else if (std::string(name) == "summer_time") {
            *buf = 0;
            LOG_WARN("using default value of: {}/{}", category, name);
            return 0;
        }
    } else if (std::string(category) == "/CONFIG/ACCESSIBILITY") {
        if (std::string(name) == "invert_color") {
            *buf = 0;
            LOG_WARN("using default value of: {}/{}", category, name);
            return 0;
        } else if (std::string(name) == "keyremap_enable") {
            *buf = 0;
            LOG_WARN("using default value of: {}/{}", category, name);
            return 0;
        }
    }

    LOG_WARN("Unknow value of category: {}, name: {}", category, name);

    return 0;
}

EXPORT(int, sceRegMgrGetKeyStr, const char *category, const char *name, char *buf, const int size) {
    TRACY_FUNC(sceRegMgrGetKeyStr);
    LOG_DEBUG("category: {}, name: {}, size: {}", category, name, size);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrGetKeys, const char *category, char *buf, const int elements_number) {
    TRACY_FUNC(sceRegMgrGetKeys, category, buf, elements_number);
    LOG_DEBUG("category: {}, elements number: {}", category, elements_number);
    return UNIMPLEMENTED();
}

struct KeyInfo {
    int size; // sizeof(this)
    Ptr<const char> key;
    int type; // probably type - cba to RE
    int key_size; // probably size - cba to RE
};

EXPORT(int, sceRegMgrGetKeysInfo, const char *category, KeyInfo *keys, int num_keys) {
    TRACY_FUNC(sceRegMgrGetKeysInfo);
    for (int i = 0; i < num_keys; i++) {
        LOG_DEBUG("key: {}/{}", category, keys[i].key.get(emuenv.mem));
        // std::string target = category + '/' + keys[i].key;
        // keys[i].type = get_type_of_key(category, keys[i].key); // code it yourself or hardcode 0
        // keys[i].key_size = get_size_of_key(category, keys[i].key); // code it yourself or hardcode 4
    }
    return 0;
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrGetRegVersion) {
    TRACY_FUNC(sceRegMgrGetRegVersion);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrIsBlueScreen) {
    TRACY_FUNC(sceRegMgrIsBlueScreen);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrRegisterCallback) {
    TRACY_FUNC(sceRegMgrRegisterCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrRegisterDrvErrCallback) {
    TRACY_FUNC(sceRegMgrRegisterDrvErrCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrResetRegistryLv) {
    TRACY_FUNC(sceRegMgrResetRegistryLv);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSetKeyBin) {
    TRACY_FUNC(sceRegMgrSetKeyBin);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSetKeyInt, const char *category, const char *name, int buf) {
    TRACY_FUNC(sceRegMgrSetKeyInt, category, name, buf);
    LOG_DEBUG("category: {}, name: {}, buf: {}", category, name, buf);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSetKeyStr, const char *category, const char *name, char *buf, int size) {
    TRACY_FUNC(sceRegMgrSetKeyStr);
    LOG_DEBUG("category: {}, name: {}, buf: {}, size: {}", category, name, buf, size);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrSetKeys) {
    TRACY_FUNC(sceRegMgrSetKeys);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrStartCallback) {
    TRACY_FUNC(sceRegMgrStartCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrStopCallback) {
    TRACY_FUNC(sceRegMgrStopCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrUnregisterCallback) {
    TRACY_FUNC(sceRegMgrUnregisterCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrUnregisterDrvErrCallback) {
    TRACY_FUNC(sceRegMgrUnregisterDrvErrCallback);
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceRegMgrAddRegistryCallback)
BRIDGE_IMPL(sceRegMgrDbBackup)
BRIDGE_IMPL(sceRegMgrDbRestore)
BRIDGE_IMPL(sceRegMgrGetInitVals)
BRIDGE_IMPL(sceRegMgrGetKeyBin)
BRIDGE_IMPL(sceRegMgrGetKeyInt)
BRIDGE_IMPL(sceRegMgrGetKeyStr)
BRIDGE_IMPL(sceRegMgrGetKeys)
BRIDGE_IMPL(sceRegMgrGetKeysInfo)
BRIDGE_IMPL(sceRegMgrGetRegVersion)
BRIDGE_IMPL(sceRegMgrIsBlueScreen)
BRIDGE_IMPL(sceRegMgrRegisterCallback)
BRIDGE_IMPL(sceRegMgrRegisterDrvErrCallback)
BRIDGE_IMPL(sceRegMgrResetRegistryLv)
BRIDGE_IMPL(sceRegMgrSetKeyBin)
BRIDGE_IMPL(sceRegMgrSetKeyInt)
BRIDGE_IMPL(sceRegMgrSetKeyStr)
BRIDGE_IMPL(sceRegMgrSetKeys)
BRIDGE_IMPL(sceRegMgrStartCallback)
BRIDGE_IMPL(sceRegMgrStopCallback)
BRIDGE_IMPL(sceRegMgrUnregisterCallback)
BRIDGE_IMPL(sceRegMgrUnregisterDrvErrCallback)
