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

#include "ScePower.h"

#include <util/tracy.h>
#include <util/types.h>

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_power.h>

#include <kernel/callback.h>
#include <kernel/state.h>
#include <util/lock_and_find.h>

#include <climits>

#define LOW_BATTERY_PERCENT 10

TRACY_MODULE_NAME(ScePower);

enum ScePowerErrorCode {
    SCE_POWER_ERROR_INVALID_VALUE = 0x802B0000,
    SCE_POWER_ERROR_ALREADY_REGISTERED = 0x802B0001,
    SCE_POWER_ERROR_CALLBACK_NOT_REGISTERED = 0x802B0002,
    SCE_POWER_ERROR_CANT_SUSPEND = 0x802B0003,
    SCE_POWER_ERROR_NO_BATTERY = 0x802B0100,
    SCE_POWER_ERROR_DETECTING = 0x802B0101
};

enum ScePowerBatteryRemainLevel {
    SCE_POWER_BATTERY_REMAIN_LEVEL_INVALID = 0,
    SCE_POWER_BATTERY_REMAIN_LEVEL_0_25_PERCENTS = 1,
    SCE_POWER_BATTERY_REMAIN_LEVEL_25_50_PERCENTS = 2,
    SCE_POWER_BATTERY_REMAIN_LEVEL_50_75_PERCENTS = 3,
    SCE_POWER_BATTERY_REMAIN_LEVEL_75_100_PERCENTS = 4
};

EXPORT(int, scePowerBatteryUpdateInfo) {
    TRACY_FUNC(scePowerBatteryUpdateInfo);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerCancelRequest) {
    TRACY_FUNC(scePowerCancelRequest);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetArmClockFrequency) {
    TRACY_FUNC(scePowerGetArmClockFrequency);
    return 444;
}

EXPORT(int, scePowerGetBatteryChargingStatus) {
    TRACY_FUNC(scePowerGetBatteryChargingStatus);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBatteryCycleCount) {
    TRACY_FUNC(scePowerGetBatteryCycleCount);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBatteryElec) {
    TRACY_FUNC(scePowerGetBatteryElec);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBatteryFullCapacity) {
    TRACY_FUNC(scePowerGetBatteryFullCapacity);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBatteryLifePercent) {
    TRACY_FUNC(scePowerGetBatteryLifePercent);
    int res;
    SDL_GetPowerInfo(NULL, &res);
    if (res == -1) {
        return 100;
    }
    return res;
}

EXPORT(int, scePowerGetBatteryLifeTime) {
    TRACY_FUNC(scePowerGetBatteryLifeTime);
    int res;
    SDL_GetPowerInfo(&res, NULL);
    if (res == -1) {
        return INT_MAX;
    }
    return res;
}

EXPORT(int, scePowerGetBatteryRemainCapacity) {
    TRACY_FUNC(scePowerGetBatteryRemainCapacity);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBatteryRemainLevel) {
    TRACY_FUNC(scePowerGetBatteryRemainLevel);
    int res;
    SDL_GetPowerInfo(NULL, &res);
    if (res >= 0) {
        if (res <= 25)
            return SCE_POWER_BATTERY_REMAIN_LEVEL_0_25_PERCENTS;
        else if (res <= 50)
            return SCE_POWER_BATTERY_REMAIN_LEVEL_25_50_PERCENTS;
        else if (res <= 75)
            return SCE_POWER_BATTERY_REMAIN_LEVEL_50_75_PERCENTS;
    }

    return SCE_POWER_BATTERY_REMAIN_LEVEL_75_100_PERCENTS;
}

EXPORT(int, scePowerGetBatteryRemainMaxLevel) {
    TRACY_FUNC(scePowerGetBatteryRemainMaxLevel);
    return SCE_POWER_BATTERY_REMAIN_LEVEL_75_100_PERCENTS;
}

EXPORT(int, scePowerGetBatterySOH) {
    TRACY_FUNC(scePowerGetBatterySOH);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBatteryTemp) {
    TRACY_FUNC(scePowerGetBatteryTemp);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBatteryVolt) {
    TRACY_FUNC(scePowerGetBatteryVolt);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetBusClockFrequency) {
    TRACY_FUNC(scePowerGetBusClockFrequency);
    return 222;
}

EXPORT(int, scePowerGetCaseTemp) {
    TRACY_FUNC(scePowerGetCaseTemp);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerGetGpuClockFrequency) {
    TRACY_FUNC(scePowerGetGpuClockFrequency);
    return 222;
}

EXPORT(int, scePowerGetGpuXbarClockFrequency) {
    TRACY_FUNC(scePowerGetGpuXbarClockFrequency);
    return 166;
}

EXPORT(int, scePowerGetUsingWireless) {
    TRACY_FUNC(scePowerGetUsingWireless);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerIsBatteryCharging) {
    TRACY_FUNC(scePowerIsBatteryCharging);
    SDL_PowerState info = SDL_GetPowerInfo(NULL, NULL);
    return (info == SDL_POWERSTATE_CHARGING);
}

EXPORT(int, scePowerIsBatteryExist) {
    TRACY_FUNC(scePowerIsBatteryExist);
    SDL_PowerState info = SDL_GetPowerInfo(NULL, NULL);
    return (info != SDL_POWERSTATE_NO_BATTERY);
}

EXPORT(int, scePowerIsLowBattery) {
    TRACY_FUNC(scePowerIsLowBattery);
    int res;
    SDL_GetPowerInfo(NULL, &res);
    if (res <= LOW_BATTERY_PERCENT) {
        return SCE_TRUE;
    }
    return SCE_FALSE;
}

EXPORT(int, scePowerIsLowBatteryInhibitUpdateDownload) {
    TRACY_FUNC(scePowerIsLowBatteryInhibitUpdateDownload);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerIsLowBatteryInhibitUpdateReboot) {
    TRACY_FUNC(scePowerIsLowBatteryInhibitUpdateReboot);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerIsPowerOnline) {
    TRACY_FUNC(scePowerIsPowerOnline);
    SDL_PowerState info = SDL_GetPowerInfo(NULL, NULL);
    return ((info != SDL_POWERSTATE_UNKNOWN) && (info != SDL_POWERSTATE_ON_BATTERY));
}

EXPORT(int, scePowerIsRequest) {
    TRACY_FUNC(scePowerIsRequest);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerIsSuspendRequired) {
    TRACY_FUNC(scePowerIsSuspendRequired);
    return UNIMPLEMENTED();
}

enum ScePowerCallbackType {
    SCE_POWER_CB_AFTER_SYSTEM_RESUME = 0x00000080, /* TODO: confirm */
    SCE_POWER_CB_BATTERY_ONLINE = 0x00000100,
    SCE_POWER_CB_THERMAL_SUSPEND = 0x00000200, /* TODO: confirm */
    SCE_POWER_CB_LOW_BATTERY_SUSPEND = 0x00000400, /* TODO: confirm */
    SCE_POWER_CB_LOW_BATTERY = 0x00000800,
    SCE_POWER_CB_POWER_ONLINE = 0x00001000,
    SCE_POWER_CB_SYSTEM_SUSPEND = 0x00010000,
    SCE_POWER_CB_SYSTEM_RESUMING = 0x00020000,
    SCE_POWER_CB_SYSTEM_RESUME = 0x00040000,
    SCE_POWER_CB_UNK_0x100000 = 0x00100000, /* Related to proc_event::display_switch */
    SCE_POWER_CB_APP_RESUME = 0x00200000,
    SCE_POWER_CB_APP_SUSPEND = 0x00400000,
    SCE_POWER_CB_APP_RESUMING = 0x00800000, /* TODO: confirm */
    SCE_POWER_CB_BUTTON_PS_START_PRESS = 0x04000000,
    SCE_POWER_CB_BUTTON_PS_POWER_PRESS = 0x08000000,
    SCE_POWER_CB_BUTTON_PS_HOLD = 0x10000000,
    SCE_POWER_CB_BUTTON_PS_PRESS = 0x20000000,
    SCE_POWER_CB_BUTTON_POWER_HOLD = 0x40000000,
    SCE_POWER_CB_BUTTON_POWER_PRESS = 0x80000000,
    SCE_POWER_CB_VALID_MASK_KERNEL = 0xFCF71F80,
    SCE_POWER_CB_VALID_MASK_SYSTEM = 0xFCF71F80,
    SCE_POWER_CB_VALID_MASK_NON_SYSTEM = 0x00361180
};

inline std::string to_debug_str(ScePowerCallbackType type) {
    switch (type) {
    case ScePowerCallbackType::SCE_POWER_CB_AFTER_SYSTEM_RESUME: return "SCE_POWER_CB_AFTER_SYSTEM_RESUME";
    case ScePowerCallbackType::SCE_POWER_CB_BATTERY_ONLINE: return "SCE_POWER_CB_BATTERY_ONLINE";
    case ScePowerCallbackType::SCE_POWER_CB_THERMAL_SUSPEND: return "SCE_POWER_CB_THERMAL_SUSPEND";
    case ScePowerCallbackType::SCE_POWER_CB_LOW_BATTERY_SUSPEND: return "SCE_POWER_CB_LOW_BATTERY_SUSPEND";
    case ScePowerCallbackType::SCE_POWER_CB_LOW_BATTERY: return "SCE_POWER_CB_LOW_BATTERY";
    case ScePowerCallbackType::SCE_POWER_CB_POWER_ONLINE: return "SCE_POWER_CB_POWER_ONLINE";
    case ScePowerCallbackType::SCE_POWER_CB_SYSTEM_SUSPEND: return "SCE_POWER_CB_SYSTEM_SUSPEND";
    case ScePowerCallbackType::SCE_POWER_CB_SYSTEM_RESUMING: return "SCE_POWER_CB_SYSTEM_RESUMING";
    case ScePowerCallbackType::SCE_POWER_CB_SYSTEM_RESUME: return "SCE_POWER_CB_SYSTEM_RESUME";
    case ScePowerCallbackType::SCE_POWER_CB_UNK_0x100000: return "SCE_POWER_CB_UNK_0x100000";
    case ScePowerCallbackType::SCE_POWER_CB_APP_RESUME: return "SCE_POWER_CB_APP_RESUME";
    case ScePowerCallbackType::SCE_POWER_CB_APP_SUSPEND: return "SCE_POWER_CB_APP_SUSPEND";
    case ScePowerCallbackType::SCE_POWER_CB_APP_RESUMING: return "SCE_POWER_CB_APP_RESUMING";
    case ScePowerCallbackType::SCE_POWER_CB_BUTTON_PS_START_PRESS: return "SCE_POWER_CB_BUTTON_PS_START_PRESS";
    case ScePowerCallbackType::SCE_POWER_CB_BUTTON_PS_POWER_PRESS: return "SCE_POWER_CB_BUTTON_PS_POWER_PRESS";
    case ScePowerCallbackType::SCE_POWER_CB_BUTTON_PS_HOLD: return "SCE_POWER_CB_BUTTON_PS_HOLD";
    case ScePowerCallbackType::SCE_POWER_CB_BUTTON_PS_PRESS: return "SCE_POWER_CB_BUTTON_PS_PRESS";
    case ScePowerCallbackType::SCE_POWER_CB_BUTTON_POWER_HOLD: return "SCE_POWER_CB_BUTTON_POWER_HOLD";
    case ScePowerCallbackType::SCE_POWER_CB_BUTTON_POWER_PRESS: return "SCE_POWER_CB_BUTTON_POWER_PRESS";
    case ScePowerCallbackType::SCE_POWER_CB_VALID_MASK_SYSTEM: return "SCE_POWER_CB_VALID_MASK_SYSTEM";
    case ScePowerCallbackType::SCE_POWER_CB_VALID_MASK_NON_SYSTEM: return "SCE_POWER_CB_VALID_MASK_NON_SYSTEM";
    default: return std::to_string(static_cast<typename std::underlying_type<ScePowerCallbackType>::type>(type));
    }
}

int power_thread_id = 0;
std::map<SceUID, CallbackPtr> power_callbacks{};
void send_power_callback(ScePowerCallbackType type) {
    // LOG_TRACE("power event send:{}", to_debug_str(type));
    for (auto &cb : power_callbacks) {
        cb.second->direct_notify(type);
    }
}

static int SDLCALL thread_function(EmuEnvState &emuenv) {
    bool currentKeyState = false; // �tat actuel de la touche
    bool previousKeyState = false; // �tat pr�c�dent de la touche
    bool holdEventTriggered = false; // Indicateur de d�clenchement de l'�v�nement de maintien enfonc�
    bool pressEventTriggered = false; // Indicateur de d�clenchement de l'�v�nement de pression
    std::chrono::steady_clock::time_point pressStartTime; // Heure de d�but de l'appui du bouton PS

    while (true) {
        const uint8_t *keys = SDL_GetKeyboardState(nullptr);

        previousKeyState = currentKeyState;
        currentKeyState = keys[emuenv.cfg.keyboard_button_psbutton_shell];
        if (currentKeyState && !previousKeyState) {
            // La touche vient d'�tre enfonc�e pour la premi�re fois
            pressStartTime = std::chrono::steady_clock::now(); // Enregistre l'heure de d�but de l'appui
            pressEventTriggered = false; // R�initialise l'indicateur d'�v�nement de pression
            holdEventTriggered = false; // R�initialise l'indicateur d'�v�nement de maintien enfonc�
        } else if (currentKeyState) {
            // Toujours enfonc�
            std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> pressDuration = currentTime - pressStartTime; // Calcule la dur�e d'appui en secondes

            if (pressDuration.count() >= 2.0 && !holdEventTriggered) {
                // Appui maintenu pendant plus de 2 secondes (SCE_POWER_CB_BUTTON_PS_HOLD)
                for (auto &cb : power_callbacks) {
                    cb.second->direct_notify(SCE_POWER_CB_BUTTON_PS_HOLD);
                }
                holdEventTriggered = true; // D�finit l'indicateur d'�v�nement de maintien enfonc� sur vrai
            }
        } else if (!currentKeyState && previousKeyState) {
            // Touche rel�ch�e apr�s une pression courte (SCE_POWER_CB_BUTTON_PS_PRESS)
            if (!holdEventTriggered && !pressEventTriggered) {
                for (auto &cb : power_callbacks) {
                    cb.second->direct_notify(SCE_POWER_CB_BUTTON_PS_PRESS);
                }
                pressEventTriggered = true; // D�finit l'indicateur d'�v�nement de pression sur vrai
            }
        }

        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

EXPORT(int, scePowerRegisterCallback, SceUID cbid) {
    TRACY_FUNC(scePowerRegisterCallback, cbid);
    LOG_TRACE("cbid:{}", cbid);
    const auto cb = lock_and_find(cbid, emuenv.kernel.callbacks, emuenv.kernel.mutex);
    if (!cb)
        return RET_ERROR(-1);
    power_callbacks[cbid] = cb;
    LOG_TRACE("name:{}, owner_thread_id:{}", cb->get_name(), cb->get_owner_thread_id());
    if (power_thread_id == 0) {
        power_thread_id = 1;
        // SDL_CreateThread(&thread_function, "SceGxmDisplayQueue",(void*)emuenv*);
        auto power_thread = std::thread(thread_function, std::ref(emuenv));
        power_thread.detach();
    }
    return 0;
}

EXPORT(int, scePowerRequestColdReset) {
    TRACY_FUNC(scePowerRequestColdReset);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerRequestDisplayOff) {
    TRACY_FUNC(scePowerRequestDisplayOff);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerRequestDisplayOn) {
    TRACY_FUNC(scePowerRequestDisplayOn);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerRequestStandby) {
    TRACY_FUNC(scePowerRequestStandby);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerRequestSuspend) {
    TRACY_FUNC(scePowerRequestSuspend);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerSetArmClockFrequency, int freq) {
    TRACY_FUNC(scePowerSetArmClockFrequency, freq);
    if (freq < 0) {
        return SCE_POWER_ERROR_INVALID_VALUE;
    }
    return 0;
}

EXPORT(int, scePowerSetBusClockFrequency, int freq) {
    TRACY_FUNC(scePowerSetBusClockFrequency, freq);
    if (freq < 0) {
        return SCE_POWER_ERROR_INVALID_VALUE;
    }
    return 0;
}

EXPORT(int, scePowerSetConfigurationMode, int mode) {
    TRACY_FUNC(scePowerSetConfigurationMode, mode);
    if (mode != 0x80 && mode != 0x800 && mode != 0x10880) {
        return SCE_POWER_ERROR_INVALID_VALUE;
    }
    return 0;
}

EXPORT(int, scePowerSetGpuClockFrequency, int freq) {
    TRACY_FUNC(scePowerSetGpuClockFrequency, freq);
    if (freq < 0) {
        return SCE_POWER_ERROR_INVALID_VALUE;
    }
    return 0;
}

EXPORT(int, scePowerSetGpuXbarClockFrequency, int freq) {
    TRACY_FUNC(scePowerSetGpuXbarClockFrequency, freq);
    if (freq < 0) {
        return SCE_POWER_ERROR_INVALID_VALUE;
    }
    return 0;
}

EXPORT(int, scePowerSetIdleTimerCount) {
    TRACY_FUNC(scePowerSetIdleTimerCount);
    return UNIMPLEMENTED();
}

EXPORT(int, scePowerSetUsingWireless, SceBool enabled) {
    TRACY_FUNC(scePowerSetUsingWireless, enabled);
    if (enabled != SCE_TRUE && enabled != SCE_FALSE) {
        return SCE_POWER_ERROR_INVALID_VALUE;
    }
    return 0;
}

EXPORT(int, scePowerUnregisterCallback, SceUID cbid) {
    TRACY_FUNC(scePowerUnregisterCallback, cbid);
    LOG_TRACE("cbid:{}", cbid);
    auto cb = emuenv.kernel.callbacks[cbid];
    LOG_TRACE("name:{}", cb->get_name());

    if (power_callbacks.find(cbid) == power_callbacks.end())
        return RET_ERROR(-1);

    power_callbacks.erase(cbid);

    return UNIMPLEMENTED();
}

BRIDGE_IMPL(scePowerBatteryUpdateInfo)
BRIDGE_IMPL(scePowerCancelRequest)
BRIDGE_IMPL(scePowerGetArmClockFrequency)
BRIDGE_IMPL(scePowerGetBatteryChargingStatus)
BRIDGE_IMPL(scePowerGetBatteryCycleCount)
BRIDGE_IMPL(scePowerGetBatteryElec)
BRIDGE_IMPL(scePowerGetBatteryFullCapacity)
BRIDGE_IMPL(scePowerGetBatteryLifePercent)
BRIDGE_IMPL(scePowerGetBatteryLifeTime)
BRIDGE_IMPL(scePowerGetBatteryRemainCapacity)
BRIDGE_IMPL(scePowerGetBatteryRemainLevel)
BRIDGE_IMPL(scePowerGetBatteryRemainMaxLevel)
BRIDGE_IMPL(scePowerGetBatterySOH)
BRIDGE_IMPL(scePowerGetBatteryTemp)
BRIDGE_IMPL(scePowerGetBatteryVolt)
BRIDGE_IMPL(scePowerGetBusClockFrequency)
BRIDGE_IMPL(scePowerGetCaseTemp)
BRIDGE_IMPL(scePowerGetGpuClockFrequency)
BRIDGE_IMPL(scePowerGetGpuXbarClockFrequency)
BRIDGE_IMPL(scePowerGetUsingWireless)
BRIDGE_IMPL(scePowerIsBatteryCharging)
BRIDGE_IMPL(scePowerIsBatteryExist)
BRIDGE_IMPL(scePowerIsLowBattery)
BRIDGE_IMPL(scePowerIsLowBatteryInhibitUpdateDownload)
BRIDGE_IMPL(scePowerIsLowBatteryInhibitUpdateReboot)
BRIDGE_IMPL(scePowerIsPowerOnline)
BRIDGE_IMPL(scePowerIsRequest)
BRIDGE_IMPL(scePowerIsSuspendRequired)
BRIDGE_IMPL(scePowerRegisterCallback)
BRIDGE_IMPL(scePowerRequestColdReset)
BRIDGE_IMPL(scePowerRequestDisplayOff)
BRIDGE_IMPL(scePowerRequestDisplayOn)
BRIDGE_IMPL(scePowerRequestStandby)
BRIDGE_IMPL(scePowerRequestSuspend)
BRIDGE_IMPL(scePowerSetArmClockFrequency)
BRIDGE_IMPL(scePowerSetBusClockFrequency)
BRIDGE_IMPL(scePowerSetConfigurationMode)
BRIDGE_IMPL(scePowerSetGpuClockFrequency)
BRIDGE_IMPL(scePowerSetGpuXbarClockFrequency)
BRIDGE_IMPL(scePowerSetIdleTimerCount)
BRIDGE_IMPL(scePowerSetUsingWireless)
BRIDGE_IMPL(scePowerUnregisterCallback)
