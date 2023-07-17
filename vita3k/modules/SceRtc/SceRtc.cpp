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

#include "SceRtc.h"

#include <kernel/state.h>
#include <rtc/rtc.h>
#include <util/tracy.h>

#include <util/safe_time.h>

#include <chrono>
#include <ctime>

TRACY_MODULE_NAME(SceRtc);

EXPORT(int, _sceRtcConvertLocalTimeToUtc, const SceRtcTick *pLocalTime, SceRtcTick *pUtc) {
    TRACY_FUNC(_sceRtcConvertLocalTimeToUtc, pLocalTime, pUtc);
    if (pUtc == nullptr || pLocalTime == nullptr) {
        return RET_ERROR(SCE_RTC_ERROR_INVALID_POINTER);
    }
    std::time_t t = std::time(nullptr);

    tm local_tm = {};
    tm gmt_tm = {};

    SAFE_LOCALTIME(&t, &local_tm);
    SAFE_GMTIME(&t, &gmt_tm);

    std::time_t local = std::mktime(&local_tm);
    std::time_t gmt = std::mktime(&gmt_tm);
    pUtc->tick = pLocalTime->tick - (local - gmt) * VITA_CLOCKS_PER_SEC;

    return 0;
}

EXPORT(int, _sceRtcConvertUtcToLocalTime, const SceRtcTick *pUtc, SceRtcTick *pLocalTime) {
    TRACY_FUNC(_sceRtcConvertUtcToLocalTime, pUtc, pLocalTime);
    if (pUtc == nullptr || pLocalTime == nullptr) {
        return RET_ERROR(SCE_RTC_ERROR_INVALID_POINTER);
    }

    std::time_t t = std::time(nullptr);

    tm local_tm = {};
    tm gmt_tm = {};

    SAFE_LOCALTIME(&t, &local_tm);
    SAFE_GMTIME(&t, &gmt_tm);

    std::time_t local = std::mktime(&local_tm);
    std::time_t gmt = std::mktime(&gmt_tm);
    pLocalTime->tick = pUtc->tick + (local - gmt) * VITA_CLOCKS_PER_SEC;
    return 0;
}

EXPORT(int, _sceRtcFormatRFC2822) {
    TRACY_FUNC(_sceRtcFormatRFC2822);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceRtcFormatRFC2822LocalTime, char *pszDateTime, const SceRtcTick *utc) {
    TRACY_FUNC(_sceRtcFormatRFC2822LocalTime, pszDateTime, utc);
    // The following code is from PPSSPP
    // Copyright (c) 2012- PPSSPP Project.

    // Get timezone difference
    std::time_t epoch_plus_11h = 60 * 60 * 11;
    tm epoch_localtime = {};
    tm epoch_gmtime = {};
    SAFE_LOCALTIME(&epoch_plus_11h, &epoch_localtime);
    SAFE_GMTIME(&epoch_plus_11h, &epoch_gmtime);

    auto local_tz_hour = epoch_localtime.tm_hour;
    const auto local_tz_minute = epoch_localtime.tm_min;
    const auto gmt_tz_hour = epoch_gmtime.tm_hour;
    const auto gmt_tz_minute = epoch_gmtime.tm_min;
    const auto tz_minute_diff = local_tz_minute - gmt_tz_minute;
    if (tz_minute_diff != 0 && gmt_tz_hour > local_tz_hour)
        local_tz_hour++;
    const auto tz_hour_diff = local_tz_hour - gmt_tz_hour;

    if (utc) { // format utc in localtime
        SceDateTime date;
        memset(&date, 0, sizeof(date));
        tm gmt = {};
        __RtcTicksToPspTime(&date, utc->tick);
        __RtcPspTimeToTm(&gmt, &date);
        while (gmt.tm_year < 70)
            gmt.tm_year += 400;
        while (gmt.tm_year >= 470)
            gmt.tm_year -= 400;

        time_t time = rtc_timegm(&gmt);
        tm current_localtime = {};
        SAFE_LOCALTIME(&time, &current_localtime);

        char *end = pszDateTime + 32;
        pszDateTime += strftime(pszDateTime, end - pszDateTime, "%a, %d %b ", &current_localtime);
        pszDateTime += snprintf(pszDateTime, end - pszDateTime, "%04d", date.year);
        pszDateTime += strftime(pszDateTime, end - pszDateTime, " %H:%M:%S ", &current_localtime);

        if (local_tz_hour < gmt_tz_hour || current_localtime.tm_mday < gmt.tm_mday) {
            pszDateTime += snprintf(pszDateTime, end - pszDateTime, "-%02d%02d", abs(tz_hour_diff), abs(tz_minute_diff));
        } else {
            pszDateTime += snprintf(pszDateTime, end - pszDateTime, "+%02d%02d", abs(tz_hour_diff), abs(tz_minute_diff));
        }
    } else { // format current time
        time_t time = std::time(nullptr);
        tm local_time = {};
        tm gmt = {};

        SAFE_LOCALTIME(&time, &local_time);
        SAFE_GMTIME(&time, &gmt);

        char *end = pszDateTime + 32;
        pszDateTime += strftime(pszDateTime, end - pszDateTime, "%a, %d %b ", &local_time);
        pszDateTime += snprintf(pszDateTime, end - pszDateTime, "%04d", local_time.tm_year + 1900);
        pszDateTime += strftime(pszDateTime, end - pszDateTime, " %H:%M:%S ", &local_time);

        if (local_tz_hour < gmt_tz_hour || local_time.tm_mday < gmt.tm_mday) {
            pszDateTime += snprintf(pszDateTime, end - pszDateTime, "-%02d%02d", abs(tz_hour_diff), abs(tz_minute_diff));
        } else {
            pszDateTime += snprintf(pszDateTime, end - pszDateTime, "+%02d%02d", abs(tz_hour_diff), abs(tz_minute_diff));
        }
    }
    return 0;
}
EXPORT(int, _sceRtcFormatRFC3339) {
    TRACY_FUNC(_sceRtcFormatRFC3339);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceRtcFormatRFC3339LocalTime) {
    TRACY_FUNC(_sceRtcFormatRFC3339LocalTime);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceRtcGetCurrentAdNetworkTick, SceRtcTick *tick) {
    TRACY_FUNC(_sceRtcGetCurrentAdNetworkTick, tick);
    STUBBED("stubbed with _sceRtcGetCurrentNetworkTick");
    return CALL_EXPORT(_sceRtcGetCurrentNetworkTick, tick);
}

EXPORT(int, _sceRtcGetCurrentClock, SceDateTime *datePtr, int iTimeZone) {
    TRACY_FUNC(_sceRtcGetCurrentClock, datePtr, iTimeZone);
    if (datePtr == nullptr) {
        return RET_ERROR(SCE_RTC_ERROR_INVALID_POINTER);
    }

    uint64_t tick = rtc_get_ticks(emuenv.kernel.base_tick.tick) + iTimeZone * 60 * 60 * VITA_CLOCKS_PER_SEC;
    __RtcTicksToPspTime(datePtr, tick);

    return 0;
}

EXPORT(int, _sceRtcGetCurrentClockLocalTime, SceDateTime *datePtr) {
    TRACY_FUNC(_sceRtcGetCurrentClockLocalTime, datePtr);
    if (datePtr == nullptr) {
        return RET_ERROR(SCE_RTC_ERROR_INVALID_POINTER);
    }

    std::time_t t = std::time(nullptr);

    tm local_tm = {};
    tm gmt_tm = {};

    SAFE_LOCALTIME(&t, &local_tm);
    SAFE_GMTIME(&t, &gmt_tm);

    std::time_t local = std::mktime(&local_tm);
    std::time_t gmt = std::mktime(&gmt_tm);
    uint64_t tick = rtc_get_ticks(emuenv.kernel.base_tick.tick) + (local - gmt) * VITA_CLOCKS_PER_SEC;
    __RtcTicksToPspTime(datePtr, tick);
    return 0;
}

EXPORT(int, _sceRtcGetCurrentDebugNetworkTick) {
    TRACY_FUNC(_sceRtcGetCurrentDebugNetworkTick);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceRtcGetCurrentGpsTick) {
    TRACY_FUNC(_sceRtcGetCurrentGpsTick);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceRtcGetCurrentNetworkTick, SceRtcTick *tick) {
    TRACY_FUNC(_sceRtcGetCurrentNetworkTick, tick);
    if (tick == nullptr) {
        return RET_ERROR(SCE_RTC_ERROR_INVALID_POINTER);
    }

    tick->tick = rtc_get_ticks(emuenv.kernel.base_tick.tick);

    return 0;
}

EXPORT(int, _sceRtcGetCurrentRetainedNetworkTick) {
    TRACY_FUNC(_sceRtcGetCurrentRetainedNetworkTick);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceRtcGetCurrentTick, SceRtcTick *tick) {
    TRACY_FUNC(_sceRtcGetCurrentTick, tick);
    if (tick == nullptr) {
        return RET_ERROR(SCE_RTC_ERROR_INVALID_POINTER);
    }

    tick->tick = rtc_get_ticks(emuenv.kernel.base_tick.tick);

    return 0;
}

EXPORT(int, _sceRtcGetLastAdjustedTick) {
    TRACY_FUNC(_sceRtcGetLastAdjustedTick);
    return UNIMPLEMENTED();
}

EXPORT(int, _sceRtcGetLastReincarnatedTick) {
    TRACY_FUNC(_sceRtcGetLastReincarnatedTick);
    return UNIMPLEMENTED();
}

EXPORT(SceULong64, sceRtcGetAccumulativeTime) {
    TRACY_FUNC(sceRtcGetAccumulativeTime);
    STUBBED("sceRtcGetAccumulativeTime");
    return rtc_get_ticks(emuenv.kernel.base_tick.tick);
}

BRIDGE_IMPL(_sceRtcConvertLocalTimeToUtc)
BRIDGE_IMPL(_sceRtcConvertUtcToLocalTime)
BRIDGE_IMPL(_sceRtcFormatRFC2822)
BRIDGE_IMPL(_sceRtcFormatRFC2822LocalTime)
BRIDGE_IMPL(_sceRtcFormatRFC3339)
BRIDGE_IMPL(_sceRtcFormatRFC3339LocalTime)
BRIDGE_IMPL(_sceRtcGetCurrentAdNetworkTick)
BRIDGE_IMPL(_sceRtcGetCurrentClock)
BRIDGE_IMPL(_sceRtcGetCurrentClockLocalTime)
BRIDGE_IMPL(_sceRtcGetCurrentDebugNetworkTick)
BRIDGE_IMPL(_sceRtcGetCurrentGpsTick)
BRIDGE_IMPL(_sceRtcGetCurrentNetworkTick)
BRIDGE_IMPL(_sceRtcGetCurrentRetainedNetworkTick)
BRIDGE_IMPL(_sceRtcGetCurrentTick)
BRIDGE_IMPL(_sceRtcGetLastAdjustedTick)
BRIDGE_IMPL(_sceRtcGetLastReincarnatedTick)
BRIDGE_IMPL(sceRtcGetAccumulativeTime)
