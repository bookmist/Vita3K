// Vita3K emulator project
// Copyright (C) 2024 Vita3K team
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

#include <module/module.h>

#include "../SceProcessmgr/SceProcessmgr.h"

#include <modules/module_parent.h>
#include <ngs/state.h>
#include <ngs/system.h>
#include <util/log.h>
#include <util/tracy.h>

TRACY_MODULE_NAME(SceNgsInternal);

struct SceNgsVolumeMatrix {
    SceFloat32 matrix[SCE_NGS_MAX_SYSTEM_CHANNELS][SCE_NGS_MAX_SYSTEM_CHANNELS];
};

struct SceNgsPatchAudioPropInfo {
    SceInt32 out_channels;
    SceInt32 in_channels;
    SceNgsVolumeMatrix volume_matrix;
};

struct SceNgsVoiceInfo {
    SceUInt32 voice_state;
    SceUInt32 num_modules;
    SceUInt32 num_inputs;
    SceUInt32 num_outputs;
    SceUInt32 num_patches_per_output;
    SceUInt32 update_passed;
};

static_assert(sizeof(SceNgsPatchAudioPropInfo) == 24);

struct SceNgsPatchDeliveryInfo {
    Ptr<ngs::Voice> source_voice_handle;
    SceInt32 output_index;
    SceInt32 output_subindex;
    Ptr<ngs::Voice> dest_voice_handle;
    SceInt32 input_index;
};

static_assert(sizeof(SceNgsPatchDeliveryInfo) == 20);

enum SceNgsErrorCode : uint32_t {
    SCE_NGS_OK = 0,
    SCE_NGS_ERROR = 0x804A0001,
    SCE_NGS_ERROR_INVALID_ARG = 0x804A0002,
    SCE_NGS_ERROR_INVALID_STATE = 0x804A0010,
    SCE_NGS_ERROR_PARAM_OUT_OF_RANGE = 0x804A0009,
    SCE_NGS_ERROR_INVALID_HANDLE = 0x804A000C,
    SCE_NGS_SIZE_MISMATCH = 0x804A000D
};

enum SceNgsVoiceState : uint32_t {
    SCE_NGS_VOICE_STATE_AVAILABLE = 0,
    SCE_NGS_VOICE_STATE_ACTIVE = 1 << 0,
    SCE_NGS_VOICE_STATE_FINALIZE = 1 << 2,
    SCE_NGS_VOICE_STATE_UNLOADING = 1 << 3,
    SCE_NGS_VOICE_STATE_PENDING = 1 << 4,
    SCE_NGS_VOICE_STATE_PAUSED = 1 << 5,
    SCE_NGS_VOICE_STATE_KEY_OFF = 1 << 6
};

static constexpr SceUInt32 SCE_NGS_MODULE_FLAG_NOT_BYPASSED = 0;
static constexpr SceUInt32 SCE_NGS_MODULE_FLAG_BYPASSED = 2;

enum SceNgsVoiceInitFlag {
    SCE_NGS_VOICE_INIT_BASE = 0,
    SCE_NGS_VOICE_INIT_ROUTING = 1,
    SCE_NGS_VOICE_INIT_PRESET = 2,
    SCE_NGS_VOICE_INIT_CALLBACKS = 4,
    SCE_NGS_VOICE_INIT_ALL = 7
};

static constexpr uint32_t SCE_NGS_SAMPLE_OFFSET_FROM_AT9_HEADER = 1 << 31;

EXPORT(int, sceNgsModuleCheckParamsInRangeInternal) {
    return UNIMPLEMENTED();
}
EXPORT(int, sceNgsSulphaGetInfoInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSulphaGetModuleListInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSulphaGetSynthUpdateCallbackInternal) {
    UNIMPLEMENTED();
    return SCE_NGS_ERROR;
}

EXPORT(int, sceNgsSulphaQueryModuleInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSulphaSetSynthUpdateCallbackInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemGetCallbackListInternal) {
    return UNIMPLEMENTED();
}
EXPORT(int, sceNgsSystemGetSysHandleFromRack) {
    return UNIMPLEMENTED();
}
EXPORT(int, sceNgsSystemIsFixForBugzilla89940) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemPullDataInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemPushDataInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsVoiceClearDirtyFlagInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsVoiceGetParamsOutOfRangeBufferedInternal) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsVoiceSetAllBypassesInternal) {
    return UNIMPLEMENTED();
}

static void atrac9_get_buffer_parameter(const uint32_t start_sample, const uint32_t num_samples, const uint32_t info, SceNgsAT9SkipBufferInfo &parameter) {
    const uint8_t sample_rate_index = ((info & (0b1111 << 12)) >> 12);
    const uint8_t block_rate_index = ((info & (0b111 << 9)) >> 9);
    const uint16_t frame_bytes = ((((info & 0xFF0000) >> 16) << 3) | ((info & (0b111 << 29)) >> 29)) + 1;
    const uint8_t superframe_index = (info & (0b11 << 27)) >> 27;

    // Calculate bytes per superframe.
    const uint32_t frame_per_superframe = 1 << superframe_index;
    const uint32_t bytes_per_superframe = frame_bytes * frame_per_superframe;

    // Calculate total superframe
    static const int8_t sample_rate_index_to_frame_sample_power[] = {
        6, 6, 7, 7, 7, 8, 8, 8, 6, 6, 7, 7, 7, 8, 8, 8
    };

    const uint32_t samples_per_frame = 1 << sample_rate_index_to_frame_sample_power[sample_rate_index];
    const uint32_t samples_per_superframe = samples_per_frame * frame_per_superframe;

    const uint32_t start_superframe = (start_sample / samples_per_superframe);
    const uint32_t num_superframe = (start_sample + num_samples + samples_per_superframe - 1) / samples_per_superframe - start_superframe;

    parameter.num_bytes = num_superframe * bytes_per_superframe;
    parameter.is_super_packet = (frame_per_superframe == 1) ? 0 : 1;
    parameter.start_byte_offset = start_superframe * bytes_per_superframe;
    parameter.start_skip = (start_sample - (start_superframe * samples_per_superframe));
    parameter.end_skip = (start_superframe + num_superframe) * samples_per_superframe - (start_sample + num_samples);
}

static SceUInt32 sceNgsAT9GetSectionDetails_(SceInt32 samples_start, const SceInt32 num_samples, const SceUInt32 config_data, SceNgsAT9SkipBufferInfo *info, const uint32_t sdk_version) {
    constexpr auto export_name = __FUNCTION__;

    const uint32_t sample_rate_index = ((config_data & (0b1111 << 12)) >> 12);
    const int frame_per_superframe = 1 << (config_data >> 27 & 2U);
    short nb_samples_per_frame;
    if (sample_rate_index == 1) {
        nb_samples_per_frame = 0x40;
    } else if (sample_rate_index == 4) {
        nb_samples_per_frame = 0x80;
    } else if (sample_rate_index == 7) {
        nb_samples_per_frame = 0x100;
    } else {
        return RET_ERROR(SCE_NGS_ERROR);
    }
    // const int nb_samples_per_frame = nb_samples_per_frame;
    if (sdk_version >= 0x1500000) {
        uint32_t local_34 = samples_start & 0x80000000;
        samples_start = samples_start & 0x7fffffff;
        if (local_34 != 0) {
            auto x = samples_start - nb_samples_per_frame;
            samples_start = x & ~(x >> 31);
        }
        if (num_samples < 0)
            return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }
    if (!info)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    memset(info, 0, 0x10);
    if ((config_data & 0xffU) != 0xfe) {
        return RET_ERROR(SCE_NGS_ERROR);
    }
    const int frame_bytes = ((((config_data >> 0x18) << 8 | (config_data << 8 >> 0x18) << 0x10) >> 0xd) + 1) * frame_per_superframe;
    if (frame_per_superframe == 1) {
        info->is_super_packet = 0;
        int frames_offset = samples_start / nb_samples_per_frame;
        short start_skip = nb_samples_per_frame + (short)samples_start - (short)frames_offset * nb_samples_per_frame;
        info->start_byte_offset = frames_offset * frame_bytes;
        frames_offset = (start_skip + num_samples + (nb_samples_per_frame - 1)) / nb_samples_per_frame;
        info->num_bytes = frames_offset * frame_bytes;
        info->start_skip = start_skip;
        info->end_skip = (short)frames_offset * nb_samples_per_frame - ((short)num_samples + start_skip);
    } else {
        info->is_super_packet = 1;
        const short nb_samples_per_superframe = nb_samples_per_frame * 4;
        samples_start += nb_samples_per_frame;
        const int superframes_offset = samples_start / nb_samples_per_superframe;
        info->start_byte_offset = frame_bytes * superframes_offset;
        const int start_skip_samples = samples_start - superframes_offset * nb_samples_per_superframe;
        info->start_skip = start_skip_samples;

        int total_superframes = (samples_start + num_samples + nb_samples_per_superframe - 1) / nb_samples_per_superframe; // divide rounding up
        info->num_bytes = frame_bytes * (total_superframes - superframes_offset);
        // pAt9InfoBuffer->start_skip = (short)nStartSampleOffset - (short)superframes_offset * samples_per_superframe;
        // pAt9InfoBuffer->end_skip = (short)total_superframes * samples_per_superframe - ((short)nNumSamples + (short)nStartSampleOffset);
        info->end_skip = total_superframes * nb_samples_per_superframe - (samples_start + num_samples);
        if (info->start_skip < nb_samples_per_frame) {
            if (info->start_byte_offset != 0) {
                info->start_byte_offset -= frame_bytes;
                info->num_bytes += frame_bytes;
                info->start_skip += nb_samples_per_superframe;
            }
        }
    }
    return 0;
}

EXPORT(int, sceNgsAT9GetSectionDetailsInternal, uint32_t samples_start, const uint32_t num_samples, uint32_t config_data, SceNgsAT9SkipBufferInfo *info) {
    TRACY_FUNC(sceNgsAT9GetSectionDetailsInternal, samples_start, num_samples, config_data, info);
    if (!emuenv.cfg.current_config.ngs_enable)
        return -1;

    if (!info)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    // Check magic!
    if ((config_data & 0xFF) != 0xFE)
        return RET_ERROR(SCE_NGS_ERROR);

    auto samples_start2 = samples_start & ~SCE_NGS_SAMPLE_OFFSET_FROM_AT9_HEADER;
    auto samples_start3 = samples_start;
    SceNgsAT9SkipBufferInfo old_info{};
    atrac9_get_buffer_parameter(samples_start2, num_samples, config_data, old_info);

    // the following content is reverse engineered
    const uint8_t sample_rate_index = ((config_data & (0b1111 << 12)) >> 12);
    const uint32_t frame_bytes = ((((config_data & 0xFF0000) >> 16) << 3) | ((config_data & (0b111 << 29)) >> 29)) + 1;

    int nb_samples_per_frame;
    if (sample_rate_index == 1)
        nb_samples_per_frame = 64;
    else if (sample_rate_index == 4)
        nb_samples_per_frame = 128;
    else if (sample_rate_index == 7)
        nb_samples_per_frame = 256;
    else
        return RET_ERROR(SCE_NGS_ERROR);

    const bool is_superframe = static_cast<bool>(config_data & (0b11 << 27));

    // SCE_NGS_SAMPLE_OFFSET_FROM_AT9_HEADER was added in sdk 3.36
    const int sdk_version = CALL_EXPORT(sceKernelGetMainModuleSdkVersion);
    const bool is_sdk_recent = sdk_version >= (336 << 16);
    SceNgsAT9SkipBufferInfo decomp_info{};
    sceNgsAT9GetSectionDetails_(samples_start, num_samples, config_data, &decomp_info, sdk_version);

    if (is_sdk_recent && (samples_start & SCE_NGS_SAMPLE_OFFSET_FROM_AT9_HEADER)) {
        samples_start &= ~SCE_NGS_SAMPLE_OFFSET_FROM_AT9_HEADER;

        // remove nb_samples_per_frame from it
        samples_start = (samples_start > nb_samples_per_frame) ? (samples_start - nb_samples_per_frame) : 0;
    }

    info->is_super_packet = static_cast<bool>(is_superframe);
    if (is_superframe) {
        // there are 4 frames per superframe
        const int superframe_bytes = frame_bytes * 4;
        const int nb_samples_per_superframe = nb_samples_per_frame * 4;

        samples_start += nb_samples_per_frame;

        const int superframes_offset = samples_start / nb_samples_per_superframe;

        info->start_byte_offset = superframes_offset * superframe_bytes;

        const int start_skip_samples = samples_start - superframes_offset * nb_samples_per_superframe;
        info->start_skip = static_cast<SceInt16>(start_skip_samples);

        const int total_superframes = (samples_start + num_samples + nb_samples_per_superframe - 1) / nb_samples_per_superframe;
        const int total_bytes_read = (total_superframes - superframes_offset) * superframe_bytes;
        info->num_bytes = total_bytes_read;

        info->end_skip = static_cast<SceInt16>(total_superframes * nb_samples_per_superframe - (samples_start + num_samples));

        // some special case, make sure to put a good amound of skipped samples
        if (start_skip_samples < nb_samples_per_frame && superframes_offset > 0) {
            // transfer one superframe into the skipped samples
            info->start_byte_offset -= superframe_bytes;
            info->start_skip += nb_samples_per_superframe;
            info->num_bytes += superframe_bytes;
        }
    } else {
        const int frames_offset = samples_start / nb_samples_per_frame;
        info->start_byte_offset = frames_offset * frame_bytes;
        // a frame is always added to skip
        const int start_skip_samples = (samples_start + nb_samples_per_frame) - frames_offset * nb_samples_per_frame;
        info->start_skip = static_cast<SceInt16>(start_skip_samples);

        const int total_frames = (start_skip_samples + num_samples + nb_samples_per_frame - 1) / nb_samples_per_frame;
        info->num_bytes = total_frames * frame_bytes;

        info->end_skip = static_cast<SceInt16>(total_frames * nb_samples_per_frame - (start_skip_samples + num_samples));
    }
    LOG_INFO("sceNgsAT9GetSectionDetails curr: start_sample: {}, num_samples: {}, config_data: {}, info: {{num_bytes: {}, is_super_packet: {}, start_byte_offset: {}, start_skip: {}, end_skip: {}}}",
        samples_start, num_samples, config_data, info->num_bytes, info->is_super_packet, info->start_byte_offset, info->start_skip, info->end_skip);
    LOG_DEBUG("sceNgsAT9GetSectionDetails old : start_sample: {}, num_samples: {}, config_data: {}, info: {{num_bytes: {}, is_super_packet: {}, start_byte_offset: {}, start_skip: {}, end_skip: {}}}",
        samples_start2, num_samples, config_data, old_info.num_bytes, old_info.is_super_packet, old_info.start_byte_offset, old_info.start_skip, old_info.end_skip);
    LOG_TRACE("sceNgsAT9GetSectionDetails reve: start_sample: {}, num_samples: {}, config_data: {}, info: {{num_bytes: {}, is_super_packet: {}, start_byte_offset: {}, start_skip: {}, end_skip: {}}}",
        samples_start3, num_samples, config_data, decomp_info.num_bytes, decomp_info.is_super_packet, decomp_info.start_byte_offset, decomp_info.start_skip, decomp_info.end_skip);
    // sceNgsAT9GetSectionDetails_(samples_start3, num_samples, config_data, info, sdk_version);
    return 0;
}

EXPORT(int, sceNgsModuleGetNumPresetsInternal, ngs::System *system, const SceUInt32 module, SceUInt32 *num_presets) {
    TRACY_FUNC(sceNgsModuleGetNumPresetsInternal, system, module, num_presets);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsModuleGetPresetInternal, ngs::System *system, const SceUInt32 module, const SceUInt32 preset_index, void *params_buffer) {
    TRACY_FUNC(sceNgsModuleGetPresetInternal, system, module, preset_index, params_buffer);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsPatchCreateRoutingInternal, SceNgsPatchSetupInfo *patch_info, Ptr<ngs::Patch> *handle) {
    TRACY_FUNC(sceNgsPatchCreateRoutingInternal, patch_info, handle);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!patch_info || !handle)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    if (!patch_info->source || !patch_info->dest)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    // Make the scheduler order this right based on dependencies request
    ngs::Voice *source = patch_info->source.get(emuenv.mem);

    if (!source)
        return RET_ERROR(SCE_NGS_ERROR);

    *handle = source->rack->system->voice_scheduler.patch(emuenv.mem, patch_info);

    if (!*handle) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsPatchGetInfoInternal, ngs::Patch *patch, SceNgsPatchAudioPropInfo *prop_info, SceNgsPatchDeliveryInfo *deli_info) {
    TRACY_FUNC(sceNgsPatchGetInfoInternal, patch, prop_info, deli_info);

    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!patch) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (prop_info) {
        memcpy(prop_info->volume_matrix.matrix, patch->volume_matrix, sizeof(patch->volume_matrix));
        prop_info->in_channels = patch->dest->rack->channels_per_voice;
        prop_info->out_channels = patch->source->rack->channels_per_voice;
    }

    if (deli_info) {
        deli_info->input_index = patch->dest_index;
        deli_info->output_index = patch->output_index;
        deli_info->output_subindex = patch->output_sub_index;
        deli_info->source_voice_handle = Ptr<ngs::Voice>(patch->source, emuenv.mem);
        deli_info->dest_voice_handle = Ptr<ngs::Voice>(patch->dest, emuenv.mem);
    }

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsPatchRemoveRoutingInternal, Ptr<ngs::Patch> patch) {
    TRACY_FUNC(sceNgsPatchRemoveRoutingInternal, patch);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!patch) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (!patch.get(emuenv.mem)->source->remove_patch(emuenv.mem, patch)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    return 0;
}

EXPORT(int, sceNgsRackGetRequiredMemorySizeInternal, ngs::System *system, SceNgsRackDescription *description, uint32_t *size) {
    TRACY_FUNC(sceNgsRackGetRequiredMemorySizeInternal, system, description, size);
    if (!system) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_HANDLE);
    }
    if (!description || !description->definition || !size)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    if (!emuenv.cfg.current_config.ngs_enable) {
        *size = 1;
        return 0;
    }

    auto definition = description->definition.get(emuenv.mem);
    if (definition->output_count == 0 || definition->type >= ngs::BussType::BUSS_MAX)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    if (description->voice_count <= 0 || description->channels_per_voice < 0 || description->channels_per_voice > 2
        || description->max_patches_per_input < 0 || description->patches_per_output < 0) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }
    *size = ngs::Rack::get_required_memspace_size(emuenv.mem, description);
    return 0;
}

EXPORT(SceUInt32, sceNgsRackGetVoiceHandleInternal, ngs::Rack *rack, const uint32_t index, Ptr<ngs::Voice> *voice) {
    TRACY_FUNC(sceNgsRackGetVoiceHandleInternal, rack, index, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!rack || !voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (index >= rack->voices.size()) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    *voice = rack->voices[index];
    return SCE_NGS_OK;
}

EXPORT(SceUInt32, sceNgsRackInitInternal, ngs::System *system, SceNgsBufferInfo *info, const SceNgsRackDescription *description, Ptr<ngs::Rack> *rack) {
    TRACY_FUNC(sceNgsRackInitInternal, system, info, description, rack);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }
    if (!info || !system || !description || !description->definition || !rack) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (!ngs::init_rack(emuenv.ngs, emuenv.mem, system, info, description)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    *rack = info->data.cast<ngs::Rack>();
    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsRackReleaseInternal, ngs::Rack *rack, Ptr<void> callback) {
    TRACY_FUNC(sceNgsRackReleaseInternal, rack, callback);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!rack)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    std::unique_lock<std::recursive_mutex> lock(rack->system->voice_scheduler.mutex);
    if (!rack->system->voice_scheduler.is_updating) {
        ngs::release_rack(emuenv.ngs, emuenv.mem, rack->system, rack);
    } else if (!callback) {
        // wait for the update to finish
        // if this is called in an interrupt handler it will softlock ngs
        // but I don't think this is allowed (and if it is I don't know how to prevent this)
        LOG_WARN_ONCE("sceNgsRackRelease called in a synchronous way during a ngs update, contact devs if your game softlocks now.");

        rack->system->voice_scheduler.condvar.wait(lock);
        ngs::release_rack(emuenv.ngs, emuenv.mem, rack->system, rack);
    } else {
        // destroy rack asynchronously
        ngs::OperationPending op;
        op.type = ngs::PendingType::ReleaseRack;
        op.system = rack->system;
        op.release_data.state = &emuenv.ngs;
        op.release_data.rack = rack;
        op.release_data.callback = callback.address();
        rack->system->voice_scheduler.operations_pending.push(op);
    }

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsRackSetParamErrorCallbackInternal) {
    TRACY_FUNC(sceNgsRackSetParamErrorCallbackInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemGetRequiredMemorySizeInternal, SceNgsSystemInitParams *params, uint32_t *size) {
    TRACY_FUNC(sceNgsSystemGetRequiredMemorySizeInternal, params, size);
    if (!params || !size)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    if (!emuenv.cfg.current_config.ngs_enable) {
        *size = 1;
        return 0;
    }
    *size = ngs::System::get_required_memspace_size(params); // System struct size
    return 0;
}

EXPORT(SceUInt32, sceNgsSystemInitInternal, SceNgsBufferInfo *buffer_info, SceUInt32 compiled_sdk_version, SceNgsSystemInitParams *params, Ptr<ngs::System> *system) {
    TRACY_FUNC(sceNgsSystemInitInternal, buffer_info, compiled_sdk_version, params, system);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!ngs::init_system(emuenv.ngs, emuenv.mem, params, buffer_info->data, buffer_info->size)) {
        return RET_ERROR(SCE_NGS_ERROR); // TODO: Better error code
    }

    *system = buffer_info->data.cast<ngs::System>();
    return SCE_NGS_OK;
}

EXPORT(int, sceNgsSystemLockInternal) {
    TRACY_FUNC(sceNgsSystemLockInternal);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceNgsSystemReleaseInternal, ngs::System *system) {
    TRACY_FUNC(sceNgsSystemReleaseInternal, system);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!system)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    {
        std::unique_lock<std::recursive_mutex> lock(system->voice_scheduler.mutex);
        if (system->voice_scheduler.is_updating) {
            LOG_WARN_ONCE("sceNgsSystemRelease called during a ngs update, contact devs if your game softlocks now.");

            system->voice_scheduler.condvar.wait(lock);
        }
    }

    ngs::release_system(emuenv.ngs, emuenv.mem, system);

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsSystemSetFlagsInternal) {
    TRACY_FUNC(sceNgsSystemSetFlagsInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemSetParamErrorCallbackInternal) {
    TRACY_FUNC(sceNgsSystemSetParamErrorCallbackInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemUnlockInternal) {
    TRACY_FUNC(sceNgsSystemUnlockInternal);
    return UNIMPLEMENTED();
}

EXPORT(SceUInt32, sceNgsSystemUpdateInternal, ngs::System *system) {
    TRACY_FUNC(sceNgsSystemUpdateInternal, system);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    system->voice_scheduler.update(emuenv.kernel, emuenv.mem, thread_id);

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceBypassModuleInternal, ngs::Voice *voice, const SceUInt32 module, const SceUInt32 bypass_flag) {
    TRACY_FUNC(sceNgsVoiceBypassModuleInternal, voice, module, bypass_flag);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    ngs::ModuleData *storage = voice->module_storage(module);

    if (!storage)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    // no need to lock a mutex for this
    storage->is_bypassed = (bypass_flag & SCE_NGS_MODULE_FLAG_BYPASSED);

    return SCE_NGS_OK;
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetAtrac9VoiceInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetAtrac9VoiceInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_ATRAC9);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetCompressorBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetCompressorBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_COMPRESSOR);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetCompressorSideChainBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetCompressorSideChainBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SIDE_CHAIN_COMPRESSOR);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetDelayBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetDelayBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_DELAY);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetDistortionBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetDistortionBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_DISTORTION);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetEnvelopeBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetEnvelopeBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_ENVELOPE);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetEqBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetEqBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_EQUALIZATION);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetMasterBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetMasterBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_MASTER);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetMixerBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetMixerBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_MIXER);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetPauserBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetPauserBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_PAUSER);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetPitchShiftBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetPitchShiftBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_PITCH_SHIFT);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetReverbBussInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetReverbBussInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_REVERB);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetSasEmuVoiceInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetSasEmuVoiceInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SAS_EMULATION);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetScreamVoiceAT9Internal) {
    TRACY_FUNC(sceNgsVoiceDefGetScreamVoiceAT9Internal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SCREAM_ATRAC9);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetScreamVoiceInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetScreamVoiceInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SCREAM);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetSimpleAtrac9VoiceInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetSimpleAtrac9VoiceInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SIMPLE_ATRAC9);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetSimpleVoiceInternal) {
    TRACY_FUNC(sceNgsVoiceDefGetSimpleVoiceInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SIMPLE);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetTemplate1Internal) {
    TRACY_FUNC(sceNgsVoiceDefGetTemplate1Internal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_NORMAL_PLAYER);
}

EXPORT(int, sceNgsVoiceDefinitionGetPresetInternal) {
    TRACY_FUNC(sceNgsVoiceDefinitionGetPresetInternal);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    return UNIMPLEMENTED();
}

static SceUInt32 ngsVoiceStateFromHLEState(const ngs::Voice *voice) {
    SceUInt32 state;
    switch (voice->state) {
    case ngs::VoiceState::VOICE_STATE_AVAILABLE:
        state = SCE_NGS_VOICE_STATE_AVAILABLE;
        break;

    case ngs::VoiceState::VOICE_STATE_ACTIVE:
        state = SCE_NGS_VOICE_STATE_ACTIVE;
        break;

    case ngs::VoiceState::VOICE_STATE_FINALIZING:
        state = SCE_NGS_VOICE_STATE_FINALIZE;
        break;

    case ngs::VoiceState::VOICE_STATE_UNLOADING:
        state = SCE_NGS_VOICE_STATE_UNLOADING;
        break;

    default:
        assert(false && "Invalid voice state to translate");
        return 0;
    }

    if (voice->is_pending)
        state |= SCE_NGS_VOICE_STATE_PENDING;

    if (voice->is_paused)
        state |= SCE_NGS_VOICE_STATE_PAUSED;

    if (voice->is_keyed_off)
        state |= SCE_NGS_VOICE_STATE_KEY_OFF;

    return state;
}

EXPORT(SceInt32, sceNgsVoiceGetInfoInternal, ngs::Voice *voice, SceNgsVoiceInfo *info) {
    TRACY_FUNC(sceNgsVoiceGetInfoInternal, voice, info);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!voice || !info) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    const std::lock_guard<std::mutex> guard(*voice->voice_mutex);

    info->voice_state = ngsVoiceStateFromHLEState(voice);
    info->num_modules = static_cast<SceUInt32>(voice->datas.size());
    info->num_inputs = static_cast<SceUInt32>(voice->inputs.inputs.size());
    info->num_outputs = voice->rack->vdef->output_count;
    info->num_patches_per_output = static_cast<SceUInt32>(voice->rack->patches_per_output);
    info->update_passed = voice->frame_count;

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceGetModuleBypassInternal, ngs::Voice *voice, const SceUInt32 module, SceUInt32 *bypass_flag) {
    TRACY_FUNC(sceNgsVoiceGetModuleBypassInternal, voice, module, bypass_flag);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    ngs::ModuleData *storage = voice->module_storage(module);

    if (!storage)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    if (storage->is_bypassed)
        *bypass_flag = SCE_NGS_MODULE_FLAG_BYPASSED;
    else
        *bypass_flag = SCE_NGS_MODULE_FLAG_NOT_BYPASSED;

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsVoiceGetModuleTypeInternal, ngs::Voice *voice, const SceUInt32 module, SceUInt32 *module_type) {
    TRACY_FUNC(sceNgsVoiceGetModuleTypeInternal, voice, module, module_type);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!voice || !module_type)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    if (module >= voice->rack->modules.size())
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    *module_type = voice->rack->modules[module]->module_id();
    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceGetOutputPatchInternal, ngs::Voice *voice, const SceInt32 output_index, const SceInt32 output_subindex, Ptr<ngs::Patch> *patch) {
    TRACY_FUNC(sceNgsVoiceGetOutputPatchInternal, voice, output_index, output_subindex, patch);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice || !patch) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if ((output_subindex < 0) || (output_index < 0)) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if ((output_index >= static_cast<SceInt32>(voice->rack->vdef->output_count)) || (output_subindex >= voice->rack->patches_per_output)) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    *patch = voice->patches[output_index][output_subindex];
    if (!(*patch) || (patch->get(emuenv.mem))->output_sub_index == -1) {
        LOG_WARN_ONCE("Getting non-existen output patch port {}:{}", output_index, output_subindex);
        *patch = Ptr<ngs::Patch>(0);
    }

    return 0;
}

EXPORT(int, sceNgsVoiceGetParamsOutOfRangeInternal) {
    TRACY_FUNC(sceNgsVoiceGetParamsOutOfRangeInternal);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceNgsVoiceGetStateDataInternal, ngs::Voice *voice, const SceUInt32 module, void *mem, const SceUInt32 mem_size) {
    TRACY_FUNC(sceNgsVoiceGetStateDataInternal, voice, module, mem, mem_size);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    ngs::ModuleData *storage = voice->module_storage(module);
    if (!storage)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    if (mem) {
        memset(mem, 0, mem_size);
        memcpy(mem, storage->voice_state_data.data(), std::min<std::size_t>(mem_size, storage->voice_state_data.size()));
    }
    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceInitInternal, ngs::Voice *voice, const SceNgsVoicePreset *preset, const SceUInt32 init_flags) {
    TRACY_FUNC(sceNgsVoiceInitInternal, voice, preset, init_flags);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    if (voice->state == ngs::VoiceState::VOICE_STATE_ACTIVE)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_STATE);

    std::lock_guard<std::mutex> guard(*voice->voice_mutex);

    if (init_flags == SCE_NGS_VOICE_INIT_BASE || init_flags == SCE_NGS_VOICE_INIT_ALL) {
        voice->state = ngs::VoiceState::VOICE_STATE_AVAILABLE;
    }

    if (init_flags & SCE_NGS_VOICE_INIT_ROUTING) {
        // reset all patches
        for (auto &patches : voice->patches) {
            for (auto &patch : patches) {
                if (patch) {
                    patch.get(emuenv.mem)->output_sub_index = -1;
                }
            }
        }
    }

    if (init_flags & SCE_NGS_VOICE_INIT_PRESET) {
        if (!preset) {
            STUBBED("Default preset not implemented");
        } else if (!voice->set_preset(emuenv.mem, preset)) {
            return RET_ERROR(SCE_NGS_ERROR);
        }
    }

    if (init_flags & SCE_NGS_VOICE_INIT_CALLBACKS) {
        for (auto &module_data : voice->datas) {
            module_data.callback = Ptr<void>();
        }
    }

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceKeyOffInternal, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoiceKeyOffInternal, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return SCE_NGS_OK;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    voice->is_keyed_off = true;
    voice->rack->system->voice_scheduler.off(emuenv.mem, voice);

    // call the finish callback, I got no idea what the module id should be in this case
    voice->invoke_callback(emuenv.kernel, emuenv.mem, thread_id, voice->finished_callback, voice->finished_callback_user_data, 0);

    voice->is_keyed_off = false;
    voice->rack->system->voice_scheduler.stop(emuenv.mem, voice);
    return SCE_NGS_OK;
}

EXPORT(int, sceNgsVoiceKillInternal, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoiceKillInternal, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    voice->rack->system->voice_scheduler.stop(emuenv.mem, voice);

    return 0;
}

EXPORT(SceUInt32, sceNgsVoiceLockParamsInternal, ngs::Voice *voice, SceUInt32 module, SceUInt32 param_interface_id, SceNgsBufferInfo *buf) {
    TRACY_FUNC(sceNgsVoiceLockParamsInternal, voice, module, param_interface_id, buf);
    if (!emuenv.cfg.current_config.ngs_enable) {
        *buf = {
            Ptr<void>(alloc(emuenv.mem, 10, "SceNgs buffer stub")), 10
        };

        return 0;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    ngs::ModuleData *data = voice->module_storage(module);
    if (!data) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    SceNgsBufferInfo *info = data->lock_params(emuenv.mem);
    if (!info) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    info->data.cast<SceNgsParamsDescriptor>().get(emuenv.mem)->id = param_interface_id;

    *buf = *info;
    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoicePatchSetVolumeInternal, ngs::Patch *patch, const SceInt32 output_channel, const SceInt32 input_channel, const SceFloat32 vol) {
    TRACY_FUNC(sceNgsVoicePatchSetVolumeInternal, patch, output_channel, input_channel, vol);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!patch || patch->output_sub_index == -1)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    patch->volume_matrix[output_channel][input_channel] = vol;

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoicePatchSetVolumesInternal, ngs::Patch *patch, const SceInt32 output_channel, const SceFloat32 *volumes, const SceInt32 vols) {
    TRACY_FUNC(sceNgsVoicePatchSetVolumesInternal, patch, output_channel, volumes, vols);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!patch || patch->output_sub_index == -1)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    for (int i = 0; i < std::min(vols, 2); i++)
        patch->volume_matrix[output_channel][i] = volumes[i];

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoicePatchSetVolumesMatrixInternal, ngs::Patch *patch, const SceNgsVolumeMatrix *matrix) {
    TRACY_FUNC(sceNgsVoicePatchSetVolumesMatrixInternal, patch, matrix);
    if (!emuenv.cfg.current_config.ngs_enable)
        return 0;

    if (!patch || patch->output_sub_index == -1)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    memcpy(patch->volume_matrix, matrix->matrix, sizeof(matrix->matrix));

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsVoicePauseInternal, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoicePauseInternal, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (voice->is_paused)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_STATE);

    if (!voice->rack->system->voice_scheduler.pause(emuenv.mem, voice)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    return SCE_NGS_OK;
}

EXPORT(SceUInt32, sceNgsVoicePlayInternal, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoicePlayInternal, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    voice->is_pending = true;
    if (!voice->rack->system->voice_scheduler.play(emuenv.mem, voice)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }
    voice->is_pending = false;

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsVoiceResumeInternal, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoiceResumeInternal, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (!voice->is_paused)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_STATE);

    if (!voice->rack->system->voice_scheduler.resume(emuenv.mem, voice)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceSetFinishedCallbackInternal, ngs::Voice *voice, Ptr<void> callback, Ptr<void> user_data) {
    TRACY_FUNC(sceNgsVoiceSetFinishedCallbackInternal, voice, callback, user_data);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    voice->finished_callback = callback;
    voice->finished_callback_user_data = user_data;

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceSetModuleCallbackInternal, ngs::Voice *voice, const SceUInt32 module, Ptr<void> callback, Ptr<void> user_data) {
    TRACY_FUNC(sceNgsVoiceSetModuleCallbackInternal, voice, module, callback, user_data);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    ngs::ModuleData *storage = voice->module_storage(module);
    if (!storage) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    storage->callback = callback;
    storage->user_data = user_data;

    return 0;
}

EXPORT(SceInt32, sceNgsVoiceSetParamsBlockInternal, ngs::Voice *voice, const SceNgsModuleParamHeader *header,
    const SceUInt32 size, SceInt32 *pNumErrors) {
    TRACY_FUNC(sceNgsVoiceSetParamsBlockInternal, voice, header, size, pNumErrors);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    const std::lock_guard<std::mutex> guard(*voice->voice_mutex);

    const SceInt32 num_errors = voice->parse_params_block(emuenv.mem, header, size);
    if (pNumErrors != nullptr) {
        *pNumErrors = num_errors;
    }

    if (num_errors == 0)
        return SCE_NGS_OK;
    else
        return RET_ERROR(SCE_NGS_ERROR);
}

EXPORT(SceInt32, sceNgsVoiceSetPresetInternal, ngs::Voice *voice, const SceNgsVoicePreset *preset) {
    TRACY_FUNC(sceNgsVoiceSetPresetInternal, voice, preset);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!voice || !preset)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    const std::lock_guard<std::mutex> guard(*voice->voice_mutex);
    if (!voice->set_preset(emuenv.mem, preset))
        return RET_ERROR(SCE_NGS_ERROR);

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceUnlockParamsInternal, ngs::Voice *voice, const SceUInt32 module_index) {
    TRACY_FUNC(sceNgsVoiceUnlockParamsInternal, voice, module_index);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    ngs::ModuleData *data = voice->module_storage(module_index);

    if (!data) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (!data->unlock_params(emuenv.mem)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    return SCE_NGS_OK;
}

EXPORT(int, sceSulphaNgsGetDefaultConfigInternal) {
    TRACY_FUNC(sceSulphaNgsGetDefaultConfigInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsGetNeededMemoryInternal) {
    TRACY_FUNC(sceSulphaNgsGetNeededMemoryInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsInitInternal) {
    TRACY_FUNC(sceSulphaNgsInitInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetRackNameInternal) {
    TRACY_FUNC(sceSulphaNgsSetRackNameInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetSampleNameInternal) {
    TRACY_FUNC(sceSulphaNgsSetSampleNameInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetSynthNameInternal) {
    TRACY_FUNC(sceSulphaNgsSetSynthNameInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetVoiceNameInternal) {
    TRACY_FUNC(sceSulphaNgsSetVoiceNameInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsShutdownInternal) {
    TRACY_FUNC(sceSulphaNgsShutdownInternal);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsTraceInternal) {
    TRACY_FUNC(sceSulphaNgsTraceInternal);
    return UNIMPLEMENTED();
}
