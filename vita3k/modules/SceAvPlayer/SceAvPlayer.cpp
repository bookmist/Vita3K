// Vita3K emulator project
// Copyright (C) 2018 Vita3K team
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

#include "SceAvPlayer.h"

#include <io/functions.h>
#include <kernel/thread/thread_functions.h>
#include <util/lock_and_find.h>
#include <util/log.h>

#include <algorithm>

// Defines stop/pause behaviour. If true, GetVideo/AudioData will return false when stopped.
constexpr bool REJECT_DATA_ON_PAUSE = true;

// Uses a catchup video style if lag causes the video to go behind.
constexpr bool CATCHUP_VIDEO_PLAYBACK = true;

typedef Ptr<void> (*SceAvPlayerAllocator)(void *arguments, uint32_t alignment, uint32_t size);
typedef void (*SceAvPlayerDeallocator)(void *arguments, void *memory);

typedef int (*SceAvPlayerOpenFile)(void *arguments, const char *filename);
typedef int (*SceAvPlayerCloseFile)(void *arguments);
typedef int (*SceAvPlayerReadFile)(void *arguments, uint8_t *buffer, uint64_t offset, uint32_t size);
typedef uint64_t (*SceAvPlayerGetFileSize)(void *arguments);

typedef void (*SceAvPlayerEventCallback)(void *arguments, int32_t event_id, int32_t source_id, void *event_data);

enum class DebugLevel {
    NONE,
    INFO,
    WARNINGS,
    ALL,
};
/*
struct SceAvPlayerMemoryAllocator {
    Ptr<void> user_data;

    // All of these should be cast to SceAvPlayerAllocator or SceAvPlayerDeallocator types.
    Ptr<void> general_allocator;
    Ptr<void> general_deallocator;
    Ptr<void> texture_allocator;
    Ptr<void> texture_deallocator;
};

struct SceAvPlayerFileManager {
    Ptr<void> user_data;

    // Cast to SceAvPlayerOpenFile, SceAvPlayerCloseFile, SceAvPlayerReadFile and SceAvPlayerGetFileSize.
    Ptr<void> open_file;
    Ptr<void> close_file;
    Ptr<void> read_file;
    Ptr<void> file_size;
};

struct SceAvPlayerEventManager {
    Ptr<void> user_data;

    // Cast to SceAvPlayerEventCallback.
    Ptr<void> event_callback;
};
*/
struct SceAvPlayerInfo {
    SceAvPlayerMemoryAllocator memory_allocator;
    SceAvPlayerFileManager file_manager;
    SceAvPlayerEventManager event_manager;
    DebugLevel debug_level;
    uint32_t base_priority;
    int32_t frame_buffer_count;
    int32_t auto_start;
    uint32_t unknown0;
    uint32_t unknown1;
};

struct SceAvPlayerAudio {
    uint16_t channels;
    uint16_t unknown;
    uint32_t sample_rate;
    uint32_t size;
    char language[4];
};

struct SceAvPlayerVideo {
    uint32_t width;
    uint32_t height;
    float aspect_ratio;
    char language[4];
};

struct SceAvPlayerTextPosition {
    uint16_t top;
    uint16_t left;
    uint16_t bottom;
    uint16_t right;
};

struct SceAvPlayerTimedText {
    char language[4];
    uint16_t text_size;
    uint16_t font_size;
    SceAvPlayerTextPosition position;
};

union SceAvPlayerStreamDetails {
    SceAvPlayerAudio audio;
    SceAvPlayerVideo video;
    SceAvPlayerTimedText text;
};

struct SceAvPlayerFrameInfo {
    Ptr<uint8_t> data;
    uint32_t unknown;
    uint64_t timestamp;
    SceAvPlayerStreamDetails stream_details;
};

enum class MediaType {
    VIDEO,
    AUDIO,
};

struct SceAvPlayerStreamInfo {
    MediaType stream_type;
    uint32_t unknown;
    SceAvPlayerStreamDetails stream_details;
};

enum SceAvPlayerErrorCode {
    SCE_AVPLAYER_ERROR_ILLEGAL_ADDR = 0x806a0001,
    SCE_AVPLAYER_ERROR_INVALID_ARGUMENT = 0x806a0002,
    SCE_AVPLAYER_ERROR_NOT_ENOUGH_MEMORY = 0x806a0003,
    SCE_AVPLAYER_ERROR_INVALID_EVENT = 0x806a0004,
    SCE_AVPLAYER_ERROR_SOME_STREAM_PROBLEM = 0x806a00a1,
};

enum SceAvPlayerState {
    SCE_AVPLAYER_STATE_UNKNOWN = 0,
    SCE_AVPLAYER_STATE_STOP = 1,
    SCE_AVPLAYER_STATE_READY = 2, //not used, processed internally
    SCE_AVPLAYER_STATE_PLAY = 3,
    SCE_AVPLAYER_STATE_PAUSE = 4,
    SCE_AVPLAYER_STATE_BUFFERING = 5,
    SCE_AVPLAYER_STATE_SPECIAL_32 = 32
};

static inline uint64_t current_time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

static Ptr<uint8_t> get_buffer(const PlayerPtr &player, MediaType media_type,
    MemState &mem, uint32_t size, bool new_frame = true) {
    uint32_t &buffer_size = media_type == MediaType::VIDEO ? player->video_buffer_size : player->audio_buffer_size;
    uint32_t &ring_index = media_type == MediaType::VIDEO ? player->video_buffer_ring_index : player->audio_buffer_ring_index;
    auto &buffers = media_type == MediaType::VIDEO ? player->video_buffer : player->audio_buffer;

    if (buffer_size < size) {
        buffer_size = size;
        for (uint32_t a = 0; a < PlayerInfoState::RING_BUFFER_COUNT; a++) {
            if (buffers[a])
                free(mem, buffers[a]);
            std::string alloc_name = fmt::format("AvPlayer {} Media Ring {}",
                media_type == MediaType::VIDEO ? "Video" : "Audio", a);

            buffers[a] = alloc(mem, size, alloc_name.c_str());
        }
    }

    if (new_frame)
        ring_index++;
    Ptr<uint8_t> buffer = buffers[ring_index % PlayerInfoState::RING_BUFFER_COUNT];
    return buffer;
}
//callback_thread/*
struct AvPlayerCallbackThreadData_t {
    KernelState *kernel = nullptr;
    MemState *mem = nullptr;
    SceUID thid = SCE_KERNEL_ERROR_ILLEGAL_THREAD_ID;
};

AvPlayerCallbackThreadData_t AvPlayerCallbackThreadData;
#include <modules/module_parent.h>
void init(HostState &host, SceUID base_thread_id) {
    if (AvPlayerCallbackThreadData.thid != SCE_KERNEL_ERROR_ILLEGAL_THREAD_ID) {
        return;
    }
    const char *export_name = "AvPlayerCallbackThread_init";
    const ThreadStatePtr main_thread = util::find(base_thread_id, host.kernel.threads);

    const auto stack_size = SCE_KERNEL_STACK_SIZE_USER_DEFAULT; // TODO: Verify this is the correct stack size

    auto inject = create_cpu_dep_inject(host);
    auto new_thread_id = create_thread(Ptr<void>(read_pc(*main_thread->cpu)), host.kernel, host.mem, "AvPlayerCallbackThread", SCE_KERNEL_DEFAULT_PRIORITY_USER, stack_size, inject, nullptr);

    if (new_thread_id > 0x80000000) {
        LOG_ERROR("Callback thread creation error");
        return;
    }
    AvPlayerCallbackThreadData.mem = &host.mem;
    AvPlayerCallbackThreadData.kernel = &host.kernel;
    AvPlayerCallbackThreadData.thid = new_thread_id;
    const ThreadStatePtr av_player_callback_thread = util::find(new_thread_id, host.kernel.threads);

    const std::function<void(SDL_Thread *)> delete_thread = [av_player_callback_thread](SDL_Thread *running_thread) {
        {
            const std::lock_guard<std::mutex> lock(av_player_callback_thread->mutex);
            av_player_callback_thread->to_do = ThreadToDo::exit;
        }
        av_player_callback_thread->something_to_do.notify_all(); // TODO Should this be notify_one()?
    };
}

struct CPUState_z {
    SceUID thread_id;
    MemState *mem = nullptr;
    CallSVC call_svc;
    ResolveNIDName resolve_nid_name;
    size_t disasm;
    void *disasm2;
    void *get_watch_memory_addr;
    void *uc;
    void *memory_read_hook = 0;
    void *memory_write_hook = 0;
    void *code_hook = 0;
    bool returning = false;
    std::stack<Address> lr_stack;

    bool did_break = false;
    bool did_inject = false;

    std::vector<ModuleRegion> module_regions;
    std::stack<StackFrame> stack_frames;
};

int run(HostState &host, SceUID thread_id, Address callback_address, uint arglen, uint32_t args[]) {
    if (AvPlayerCallbackThreadData.thid == SCE_KERNEL_ERROR_ILLEGAL_THREAD_ID) {
        LOG_ERROR("Callback thread error");
        return 0;
    }
    const ThreadStatePtr thread = util::find(AvPlayerCallbackThreadData.thid, AvPlayerCallbackThreadData.kernel->threads);
    std::unique_lock<std::mutex> lock(thread->mutex);
    stop(*thread->cpu);
    if (arglen > 0) {
        write_reg(*thread->cpu, 0, args[0]);
    }
    if (arglen > 1) {
        write_reg(*thread->cpu, 1, args[1]);
    }
    if (arglen > 2) {
        write_reg(*thread->cpu, 2, args[2]);
    } /*
    if (arglen > 3) {
        //remaining arguments shold be puched into stack
        for (int i = 3; i < arglen; i++) {
            CPUState_z *state = (CPUState_z *)(uintptr_t)(thread->cpu.get());
            state->stack_frames.push({ static_cast<uint32_t>(args[i]), read_sp(*thread->cpu)});
        }
    }
    */
    write_pc(*thread->cpu, callback_address);
    lock.unlock();
    run_thread(*thread, true);
    return read_reg(*thread->cpu, 0);
}
uint run_event_callback(HostState &host, SceUID thread_id, const PlayerPtr player_info, int32_t event_id, int32_t source_id, void *event_data) {
    uint32_t argp_arr[4] = { 0, 0, 0, 0 };
    argp_arr[0] = player_info->event_manager.user_data.address();
    argp_arr[1] = 2; //SCE_AVPLAYER_STATE_READY
    return run(host, thread_id, player_info->event_manager.event_callback.address(), 4, argp_arr);
} /**/
//end of callback_thread

EXPORT(int, sceAvPlayerAddSource, SceUID player_handle, const char *path) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);

    player_info->player.queue(expand_path(host.io, path, host.pref_path));
    run_event_callback(host, thread_id, player_info, SCE_AVPLAYER_STATE_BUFFERING, 0, 0);
    return 0;
}

EXPORT(int, sceAvPlayerClose, SceUID player_handle) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    run_event_callback(host, thread_id, player_info, SCE_AVPLAYER_STATE_STOP, 0, 0);
    host.kernel.players.erase(player_handle);
    return 0;
}

EXPORT(uint64_t, sceAvPlayerCurrentTime, SceUID player_handle) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);

    return player_info->player.last_timestamp;
}

EXPORT(int, sceAvPlayerDisableStream) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAvPlayerStreamCount, SceUID player_handle) {
    STUBBED("ALWAYS RETURN 2 (VIDEO AND AUDIO)");
    return 2;
}

EXPORT(int, sceAvPlayerEnableStream, SceUID player_handle, uint32_t stream_no) {
    if (player_handle == 0) {
        return SCE_AVPLAYER_ERROR_ILLEGAL_ADDR;
    }
    if (stream_no > (CALL_EXPORT(sceAvPlayerStreamCount, player_handle))) {
        return SCE_AVPLAYER_ERROR_INVALID_ARGUMENT;
    }
    LOG_INFO("Stream_no {}", stream_no);
    return UNIMPLEMENTED();
}

EXPORT(bool, sceAvPlayerGetAudioData, SceUID player_handle, SceAvPlayerFrameInfo *frame_info) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);

    Ptr<uint8_t> buffer;

    if (player_info->paused) {
        if (REJECT_DATA_ON_PAUSE) {
            return false;
        } else {
            // This is probably incorrect and will make weird noises :P
            buffer = get_buffer(player_info, MediaType::AUDIO, host.mem,
                player_info->player.last_sample_count * sizeof(int16_t) * player_info->player.last_channels, true);
        }
    } else {
        std::vector<int16_t> data = player_info->player.receive_audio();

        if (data.empty())
            return false;

        buffer = get_buffer(player_info, MediaType::AUDIO, host.mem, data.size() * sizeof(int16_t), false);
        std::memcpy(buffer.get(host.mem), data.data(), data.size() * sizeof(int16_t));
    }

    frame_info->timestamp = player_info->player.last_timestamp;
    frame_info->stream_details.audio.channels = player_info->player.last_channels;
    frame_info->stream_details.audio.sample_rate = player_info->player.last_sample_rate;
    frame_info->stream_details.audio.size = player_info->player.last_channels * player_info->player.last_sample_count * sizeof(int16_t);
    frame_info->data = buffer;

    strcpy(frame_info->stream_details.audio.language, "ENG");
    return true;
}

EXPORT(uint32_t, sceAvPlayerGetStreamInfo, SceUID player_handle, uint stream_no, Ptr<SceAvPlayerStreamInfo> stream_info) {
    if (!stream_info) {
        return SCE_AVPLAYER_ERROR_ILLEGAL_ADDR;
    }
    if (player_handle == 0) {
        return SCE_AVPLAYER_ERROR_ILLEGAL_ADDR;
    }
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    auto StreamInfo = stream_info.get(host.mem);
    if (stream_no == 0) { //suspect always two streams: audio and video //first is video
        DecoderSize size = player_info->player.get_size();
        StreamInfo->stream_type = MediaType::VIDEO;
        StreamInfo->stream_details.video.width = size.width;
        StreamInfo->stream_details.video.height = size.height;
        StreamInfo->stream_details.video.aspect_ratio = static_cast<float>(size.width) / static_cast<float>(size.height);
        strcpy(StreamInfo->stream_details.video.language, "ENG");
    } else if (stream_no == 1) { // audio
        player_info->player.receive_audio();
        StreamInfo->stream_type = MediaType::AUDIO;
        StreamInfo->stream_details.audio.channels = player_info->player.last_channels;
        StreamInfo->stream_details.audio.sample_rate = player_info->player.last_sample_rate;
        StreamInfo->stream_details.audio.size = player_info->player.last_channels * player_info->player.last_sample_count * sizeof(int16_t);
        strcpy(StreamInfo->stream_details.audio.language, "ENG");
    } else {
        return SCE_AVPLAYER_ERROR_INVALID_ARGUMENT;
    }
    return 0;
}

EXPORT(bool, sceAvPlayerGetVideoData, SceUID player_handle, SceAvPlayerFrameInfo *frame_info) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);

    Ptr<uint8_t> buffer;

    DecoderSize size = player_info->player.get_size();

    uint64_t framerate = player_info->player.get_framerate_microseconds();

    // needs new frame
    if (player_info->last_frame_time + framerate < current_time()) {
        if (CATCHUP_VIDEO_PLAYBACK)
            player_info->last_frame_time += framerate;
        else
            player_info->last_frame_time = current_time();

        if (player_info->paused) {
            if (REJECT_DATA_ON_PAUSE)
                return false;
            else
                buffer = get_buffer(player_info, MediaType::VIDEO, host.mem, H264DecoderState::buffer_size(size), false);
        } else {
            buffer = get_buffer(player_info, MediaType::VIDEO, host.mem, H264DecoderState::buffer_size(size), true);

            std::vector<uint8_t> data = player_info->player.receive_video();
            std::memcpy(buffer.get(host.mem), data.data(), data.size());
        }
    } else {
        buffer = get_buffer(player_info, MediaType::VIDEO, host.mem, H264DecoderState::buffer_size(size), true);
    }

    frame_info->timestamp = player_info->player.last_timestamp;
    frame_info->stream_details.video.width = size.width;
    frame_info->stream_details.video.height = size.height;
    frame_info->stream_details.video.aspect_ratio = static_cast<float>(size.width) / static_cast<float>(size.height);
    strcpy(frame_info->stream_details.video.language, "ENG");
    frame_info->data = buffer;
    return true;
}

EXPORT(int, sceAvPlayerGetVideoDataEx) {
    return UNIMPLEMENTED();
}

EXPORT(SceUID, sceAvPlayerInit, SceAvPlayerInfo *info) {
    if (host.cfg.video_playing) {
        SceUID player_handle = host.kernel.get_next_uid();
        PlayerPtr player = std::make_shared<PlayerInfoState>();
        host.kernel.players[player_handle] = player;

        if (info->memory_allocator.general_allocator)
            LOG_WARN("General Allocator will not be used.");
        if (info->memory_allocator.general_deallocator)
            LOG_WARN("General Deallocator will not be used.");
        if (info->memory_allocator.texture_allocator)
            LOG_WARN("Texture Allocator will not be used.");
        if (info->memory_allocator.texture_deallocator)
            LOG_WARN("Texture Deallocator will not be used.");

    LOG_DEBUG("SceAvPlayerInfo.memory_allocator: user_data:{}, general_allocator:{}, general_deallocator:{}, texture_allocator:{}, texture_deallocator:{}",
        log_hex(info->memory_allocator.user_data.address()), log_hex(info->memory_allocator.general_allocator.address()), log_hex(info->memory_allocator.general_deallocator.address()),
        log_hex(info->memory_allocator.texture_allocator.address()), log_hex(info->memory_allocator.texture_deallocator.address()));
    LOG_DEBUG("SceAvPlayerInfo.file_manager: user_data:{}, open_file:{}, close_file:{}, read_file:{}, file_size:{}",
        log_hex(info->file_manager.user_data.address()), log_hex(info->file_manager.open_file.address()), log_hex(info->file_manager.close_file.address()), log_hex(info->file_manager.read_file.address()),
        log_hex(info->file_manager.file_size.address()));
    LOG_DEBUG("SceAvPlayerInfo.event_manager: user_data:{}, event_callback:{}",
        log_hex(info->event_manager.user_data.address()), log_hex(info->event_manager.event_callback.address()));
    LOG_DEBUG("SceAvPlayerInfo: debug_level:{}, base_priority:{}, frame_buffer_count:{}, auto_start:{}, unknown0:{}, unknown1:{}",
        log_hex(info->debug_level), log_hex(info->base_priority), log_hex(info->frame_buffer_count), log_hex(info->auto_start),
        log_hex(info->unknown0), log_hex(info->unknown1));

    player->last_frame_time = current_time();
    player->memory_allocator = info->memory_allocator;
    player->file_manager = info->file_manager;
    player->event_manager = info->event_manager;

    init(host, thread_id);
    //host.kernel.watch_import_calls = true;
    // Result is defined as a void *, but I just call it SceUID because it is easier to deal with. Same size.
    return player_handle;
    } else {
        LOG_WARN("Video is skipped");
        return 0;
    }
}

EXPORT(bool, sceAvPlayerIsActive, SceUID player_handle) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    
    return !player_info->player.video_playing.empty();
}

EXPORT(int, sceAvPlayerJumpToTime) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAvPlayerPause, SceUID player_handle) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    player_info->paused = true;
    run_event_callback(host, thread_id, player_info, SCE_AVPLAYER_STATE_PAUSE, 0, 0);
    return 0;
}

EXPORT(int, sceAvPlayerPostInit) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAvPlayerResume, SceUID player_handle) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    if (player_info->paused) {
        run_event_callback(host, thread_id, player_info, SCE_AVPLAYER_STATE_UNKNOWN, 0, 0);
    } else {
        run_event_callback(host, thread_id, player_info, SCE_AVPLAYER_STATE_PLAY, 0, 0);
    }
    player_info->paused = false;
    return 0;
}

EXPORT(int, sceAvPlayerSetLooping, SceUID player_handle, bool do_loop) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    player_info->do_loop = do_loop;

    return STUBBED("LOOPING NOT IMPLEMENTED");
}

EXPORT(int, sceAvPlayerSetTrickSpeed) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceAvPlayerStart, SceUID player_handle) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    if (player_info->player.videos_queue.empty()) {
        run_event_callback(host, thread_id, player_info, 3, 0, 0);
        return 0;
    }
    player_info->player.pop_video();
    run_event_callback(host, thread_id, player_info, SCE_AVPLAYER_STATE_PLAY, 0, 0);
    return 0;
}

EXPORT(int, sceAvPlayerStop, SceUID player_handle) {
    const PlayerPtr &player_info = lock_and_find(player_handle, host.kernel.players, host.kernel.mutex);
    player_info->player.free_video();
    run_event_callback(host, thread_id, player_info, SCE_AVPLAYER_STATE_STOP, 0, 0);
    return 0;
}

BRIDGE_IMPL(sceAvPlayerAddSource)
BRIDGE_IMPL(sceAvPlayerClose)
BRIDGE_IMPL(sceAvPlayerCurrentTime)
BRIDGE_IMPL(sceAvPlayerDisableStream)
BRIDGE_IMPL(sceAvPlayerEnableStream)
BRIDGE_IMPL(sceAvPlayerGetAudioData)
BRIDGE_IMPL(sceAvPlayerGetStreamInfo)
BRIDGE_IMPL(sceAvPlayerGetVideoData)
BRIDGE_IMPL(sceAvPlayerGetVideoDataEx)
BRIDGE_IMPL(sceAvPlayerInit)
BRIDGE_IMPL(sceAvPlayerIsActive)
BRIDGE_IMPL(sceAvPlayerJumpToTime)
BRIDGE_IMPL(sceAvPlayerPause)
BRIDGE_IMPL(sceAvPlayerPostInit)
BRIDGE_IMPL(sceAvPlayerResume)
BRIDGE_IMPL(sceAvPlayerSetLooping)
BRIDGE_IMPL(sceAvPlayerSetTrickSpeed)
BRIDGE_IMPL(sceAvPlayerStart)
BRIDGE_IMPL(sceAvPlayerStop)
BRIDGE_IMPL(sceAvPlayerStreamCount)
