/* plugin.c -- Config and plugin loading
 *
 * Copyright (C) 2017 Yifan Lu
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "taihen/plugin.h"

#include "io/device.h"
#include "io/functions.h"
#include "kernel/cpu_protocol.h"
#include "kernel/types.h"
#include "taihen/error.h"
#include "taihen/taihen.h"

#include <emuenv/state.h>
#include <io/vfs.h>
#include <module/module.h>
#include <taihen/taihen-parser/parser.h>
#include <util/log.h>

int plugin_init(void) {
    /*
    g_config_lock = ksceKernelCreateMutex("tai_config_lock", SCE_KERNEL_MUTEX_ATTR_RECURSIVE, 0, NULL);
    LOG("ksceKernelCreateMutex(tai_config_lock): 0x%08X", g_config_lock);
    if (g_config_lock < 0) {
        return g_config_lock;
    }*/
    return TAI_SUCCESS;
}

void plugin_deinit(void) {
    LOG("Cleaning up plugin subsystem.");
    // ksceKernelDeleteMutex(g_config_lock);
}

/**
 * @brief      Load tai config file
 *
 *             Frees any existing config, then allocates memory and loads config
 *             to it. First ux0:tai/config.txt will be looked. Followed by
 *             ur0:tai/config.txt
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_BLOCKING if attempted to call during plugin load
 */
int plugin_load_config(EmuEnvState &emuenv) {
    /** Path to the taiHEN configuration file */
    auto constexpr TAIHEN_CONFIG_FILE = "tai/config.txt";
    auto constexpr TAIHEN_CONFIG_DEVICE = VitaIoDevice::ux0;
    /** Fallback if the configuration file is not found. */
    auto constexpr TAIHEN_RECOVERY_CONFIG_DEVICE = VitaIoDevice::ur0;

    int ret;
    std::string config;

    auto module_data = get_module_data(emuenv);
    if (!module_data->g_config_lock.try_lock()) {
        return TAI_ERROR_BLOCKING;
    }
    std::lock_guard<std::mutex> lock(module_data->g_config_lock, std::adopt_lock);
    vfs::FileBuffer config_buffer;
    bool res = vfs::read_file(TAIHEN_CONFIG_DEVICE, config_buffer, emuenv.pref_path, TAIHEN_CONFIG_FILE);
    if (!res) {
        LOG_ERROR("failed to open config {}:{}", TAIHEN_CONFIG_DEVICE, TAIHEN_CONFIG_FILE);
        LOG_INFO("opening recovery config {}:{}", TAIHEN_RECOVERY_CONFIG_DEVICE, TAIHEN_CONFIG_FILE);
        res = vfs::read_file(TAIHEN_RECOVERY_CONFIG_DEVICE, config_buffer, emuenv.pref_path, TAIHEN_CONFIG_FILE);
        if (!res) {
            LOG_ERROR("Failed to open recovery config  ur0:tai/config.txt");
            return TAI_ERROR_NOT_FOUND;
        }
    }
    if (!config_buffer.empty()) {
        config = std::string(reinterpret_cast<char *>(config_buffer.data()), config_buffer.size());
    }

    if ((ret = taihen_config_validate(module_data->g_config.data())) != 0) {
        LOG("config parsing failed: {:X}", ret);
        module_data->g_config.clear();
        return ret;
    }

    module_data->g_config = config;
    return TAI_SUCCESS;
}

/**
 * @brief      Frees tai config file
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_BLOCKING if attempted to call during plugin load
 */
int plugin_free_config(EmuEnvState &emuenv) {
    auto module_data = get_module_data(emuenv);

    if (!module_data->g_config_lock.try_lock()) {
        return TAI_ERROR_BLOCKING;
    }
    std::lock_guard<std::mutex> lock(module_data->g_config_lock, std::adopt_lock);
    module_data->g_config.clear();
    return TAI_SUCCESS;
}

/**
 * @brief      Callback to config parser to load a plugin
 *
 * @param[in]  path   The path to load
 * @param[in]  param  Pointer to the PID to load plugin to.
 */
static void plugin_load(const char *path, void *param) {
    EmuEnvState &emuenv = *(EmuEnvState *)param;
    int ret;
    int result;

    LOG("loading module {}", path);
    int thread_id = 0;
    // ret = ksceKernelLoadStartModuleForPid(pid, path, 0, NULL, 0, NULL, &result);
    // ret = CALL_EXPORT(_sceKernelLoadStartModule, path, 0, nullptr, 0, nullptr, &result);
    auto module_data = get_module_data(emuenv);
    module_data->plugins_to_load.emplace_back(path);
    LOG("load result: %x", ret);
}

/**
 * @brief      Parses the taiHEN config and loads all plugins for a titleid to a
 *             process
 *
 * @param[in]  pid      The pid to load to
 * @param[in]  titleid  The title to read from the config
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_SYSTEM if the config file is invalid
 */
int plugin_load_all(EmuEnvState &emuenv, SceUID pid, const char *titleid) {
    int ret;
    auto module_data = get_module_data(emuenv);
    module_data->g_delayed_load_config = 0;
    module_data->g_delayed_load_kernel_plugins = 0;
    {
        std::lock_guard<std::mutex> lock(module_data->g_config_lock);
        if (!module_data->g_config.empty()) {
            module_data->plugins_to_load.clear();
            taihen_config_parse(module_data->g_config.c_str(), titleid, plugin_load, &emuenv);
            ret = TAI_SUCCESS;
        } else {
            LOG("config not loaded");
            ret = TAI_ERROR_SYSTEM;
        }
    }
    if (module_data->g_delayed_load_config) {
        plugin_load_config();
    }
    if (module_data->g_delayed_load_kernel_plugins) {
        plugin_load_all(KERNEL_PID, "KERNEL");
    }
    return ret;
}

/**
 * @brief      (Re)loads the config _after_ `plugin_load_all` completes.
 *
 *             The use case here is if a plugin load changes the config.txt and
 *             needs taiHEN to reload it. This cannot be done normally because
 *             the config.txt is being parsed still, so this schedules it to be
 *             done afterwards.
 *
 *             This function does nothing if called outside of a start handler.
 *
 * @param[in]  load_kernel  Load all kernel plugins as well.
 *
 * @return     Zero
 */
int plugin_delayed_load_config(EmuEnvState &emuenv, int load_kernel) {
    auto module_data = get_module_data(emuenv);
    module_data->g_delayed_load_config = 1;
    if (load_kernel) {
        module_data->g_delayed_load_kernel_plugins = 1;
    }
    return TAI_SUCCESS;
}
