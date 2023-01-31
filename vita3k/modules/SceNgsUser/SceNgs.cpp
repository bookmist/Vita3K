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

#include <modules/module_parent.h>
#include <ngs/state.h>
#include <ngs/system.h>
#include <util/log.h>
#include <util/tracy.h>

#include "SceNgs.h"

TRACY_MODULE_NAME(SceNgs);

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

EXPORT(int, sceNgsAT9GetSectionDetails, uint32_t samples_start, const uint32_t num_samples, const uint32_t config_data, SceNgsAT9SkipBufferInfo *info) {
    TRACY_FUNC(sceNgsAT9GetSectionDetails, samples_start, num_samples, config_data, info);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return -1;
    }
    if (!info) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }
    // Check magic!
    if ((config_data & 0xFF) != 0xFE) {
        return RET_ERROR(SCE_NGS_ERROR);
    }
    samples_start &= ~SCE_NGS_SAMPLE_OFFSET_FROM_AT9_HEADER;

    ngs::atrac9_get_buffer_parameter(samples_start, num_samples, config_data, *info);
    return 0;
}

EXPORT(int, sceNgsModuleGetNumPresets) {
    TRACY_FUNC(sceNgsModuleGetNumPresets);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsModuleGetPreset) {
    TRACY_FUNC(sceNgsModuleGetPreset);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsPatchCreateRouting, SceNgsPatchSetupInfo *patch_info, Ptr<ngs::Patch> *handle) {
    TRACY_FUNC(sceNgsPatchCreateRouting, patch_info, handle);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!patch_info || !handle)
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

EXPORT(SceInt32, sceNgsPatchGetInfo, ngs::Patch *patch, SceNgsPatchAudioPropInfo *prop_info, SceNgsPatchDeliveryInfo *deli_info) {
    TRACY_FUNC(sceNgsPatchGetInfo, patch, prop_info, deli_info);
    // Always stereo
    if (prop_info) {
        prop_info->in_channels = 2;
        prop_info->out_channels = 2;
    }

    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!patch) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (prop_info) {
        prop_info->volume_matrix.matrix[0][0] = patch->volume_matrix[0][0];
        prop_info->volume_matrix.matrix[0][1] = patch->volume_matrix[0][1];
        prop_info->volume_matrix.matrix[1][0] = patch->volume_matrix[1][0];
        prop_info->volume_matrix.matrix[1][1] = patch->volume_matrix[1][1];
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

EXPORT(int, sceNgsPatchRemoveRouting, Ptr<ngs::Patch> patch) {
    TRACY_FUNC(sceNgsPatchRemoveRouting, patch);
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

EXPORT(int, sceNgsRackGetRequiredMemorySize, ngs::System *system, SceNgsRackDescription *description, uint32_t *size) {
    TRACY_FUNC(sceNgsRackGetRequiredMemorySize, system, description, size);
    if (!emuenv.cfg.current_config.ngs_enable) {
        *size = 1;
        return 0;
    }

    *size = ngs::Rack::get_required_memspace_size(emuenv.mem, description);
    return 0;
}

EXPORT(SceUInt32, sceNgsRackGetVoiceHandle, ngs::Rack *rack, const uint32_t index, Ptr<ngs::Voice> *voice) {
    TRACY_FUNC(sceNgsRackGetVoiceHandle, rack, index, voice);
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

EXPORT(SceUInt32, sceNgsRackInit, ngs::System *system, SceNgsBufferInfo *info, const SceNgsRackDescription *description, Ptr<ngs::Rack> *rack) {
    TRACY_FUNC(sceNgsRackInit, system, info, description, rack);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }
    if (!info || !system || !description) {// I'm not sure about description. I found no signs of check this parameter in disasmed code
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }
    

    if (!ngs::init_rack(emuenv.ngs, emuenv.mem, system, info, description)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    *rack = info->data.cast<ngs::Rack>();
    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsRackRelease, ngs::Rack *rack, Ptr<void> callback) {
    TRACY_FUNC(sceNgsRackRelease, rack, callback);
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
        static int has_happened = false;
        LOG_WARN_IF(!has_happened, "sceNgsRackRelease called in a synchronous way during a ngs update, contact devs if your game softlocks now.");
        has_happened = true;

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

EXPORT(int, sceNgsRackSetParamErrorCallback) {
    TRACY_FUNC(sceNgsRackSetParamErrorCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemGetRequiredMemorySize, SceNgsSystemInitParams *params, uint32_t *size) {
    TRACY_FUNC(sceNgsSystemGetRequiredMemorySize, params, size);
    if (!emuenv.cfg.current_config.ngs_enable) {
        *size = 1;
        return 0;
    }

    *size = ngs::System::get_required_memspace_size(params); // System struct size
    return 0;
}

EXPORT(SceUInt32, sceNgsSystemInit, Ptr<void> memspace, const uint32_t memspace_size, SceNgsSystemInitParams *params,
    Ptr<ngs::System> *system) {
    TRACY_FUNC(sceNgsSystemInit, memspace, memspace_size, params, system);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!ngs::init_system(emuenv.ngs, emuenv.mem, params, memspace, memspace_size)) {
        return RET_ERROR(SCE_NGS_ERROR); // TODO: Better error code
    }

    *system = memspace.cast<ngs::System>();
    return SCE_NGS_OK;
}

EXPORT(int, sceNgsSystemLock) {
    TRACY_FUNC(sceNgsSystemLock);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceNgsSystemRelease, ngs::System *system) {
    TRACY_FUNC(sceNgsSystemRelease, system);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!system)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    {
        std::unique_lock<std::recursive_mutex> lock(system->voice_scheduler.mutex);
        if (system->voice_scheduler.is_updating) {
            static int has_happened = false;
            LOG_WARN_IF(!has_happened, "sceNgsSystemRelease called during a ngs update, contact devs if your game softlocks now.");
            has_happened = true;

            system->voice_scheduler.condvar.wait(lock);
        }
    }

    ngs::release_system(emuenv.ngs, emuenv.mem, system);

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsSystemSetFlags) {
    TRACY_FUNC(sceNgsSystemSetFlags);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemSetParamErrorCallback) {
    TRACY_FUNC(sceNgsSystemSetParamErrorCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNgsSystemUnlock) {
    TRACY_FUNC(sceNgsSystemUnlock);
    return UNIMPLEMENTED();
}

EXPORT(SceUInt32, sceNgsSystemUpdate, ngs::System *system) {
    TRACY_FUNC(sceNgsSystemUpdate, system);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    system->voice_scheduler.update(emuenv.kernel, emuenv.mem, thread_id);

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceBypassModule, ngs::Voice *voice, const SceUInt32 module, const SceUInt32 bypass_flag) {
    TRACY_FUNC(sceNgsVoiceBypassModule, voice, module, bypass_flag);
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

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetAtrac9Voice) {
    TRACY_FUNC(sceNgsVoiceDefGetAtrac9Voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_ATRAC9);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetCompressorBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetCompressorBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_COMPRESSOR);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetCompressorSideChainBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetCompressorSideChainBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SIDE_CHAIN_COMPRESSOR);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetDelayBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetDelayBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_DELAY);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetDistortionBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetDistortionBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_DISTORTION);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetEnvelopeBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetEnvelopeBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_ENVELOPE);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetEqBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetEqBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_EQUALIZATION);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetMasterBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetMasterBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_MASTER);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetMixerBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetMixerBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_MIXER);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetPauserBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetPauserBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_PAUSER);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetPitchShiftBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetPitchShiftBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_PITCH_SHIFT);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetReverbBuss) {
    TRACY_FUNC(sceNgsVoiceDefGetReverbBuss);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_REVERB);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetSasEmuVoice) {
    TRACY_FUNC(sceNgsVoiceDefGetSasEmuVoice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SAS_EMULATION);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetScreamAtrac9Voice) {
    TRACY_FUNC(sceNgsVoiceDefGetScreamAtrac9Voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SCREAM_ATRAC9);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetScreamVoice) {
    TRACY_FUNC(sceNgsVoiceDefGetScreamVoice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SCREAM);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetSimpleAtrac9Voice) {
    TRACY_FUNC(sceNgsVoiceDefGetSimpleAtrac9Voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SIMPLE_ATRAC9);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetSimpleVoice) {
    TRACY_FUNC(sceNgsVoiceDefGetSimpleVoice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_SIMPLE);
}

EXPORT(Ptr<ngs::VoiceDefinition>, sceNgsVoiceDefGetTemplate1) {
    TRACY_FUNC(sceNgsVoiceDefGetTemplate1);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return Ptr<ngs::VoiceDefinition>(0);
    }

    return ngs::get_voice_definition(emuenv.ngs, emuenv.mem, ngs::BussType::BUSS_NORMAL_PLAYER);
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

EXPORT(SceInt32, sceNgsVoiceGetInfo, ngs::Voice *voice, SceNgsVoiceInfo *info) {
    TRACY_FUNC(sceNgsVoiceGetInfo, voice, info);
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

EXPORT(SceInt32, sceNgsVoiceGetModuleBypass, ngs::Voice *voice, const SceUInt32 module, SceUInt32 *bypass_flag) {
    TRACY_FUNC(sceNgsVoiceGetModuleBypass, voice, module, bypass_flag);
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

EXPORT(int, sceNgsVoiceGetModuleType) {
    TRACY_FUNC(sceNgsVoiceGetModuleType);
    return UNIMPLEMENTED();
}

EXPORT(SceInt32, sceNgsVoiceGetOutputPatch, ngs::Voice *voice, const SceInt32 output_index, const SceInt32 output_subindex, Ptr<ngs::Patch> *patch) {
    TRACY_FUNC(sceNgsVoiceGetOutputPatch, voice, output_index, output_subindex, patch);
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
        LOG_WARN("Getting non-existen output patch port {}:{}", output_index, output_subindex);
        *patch = nullptr;
    }

    return 0;
}

EXPORT(int, sceNgsVoiceGetParamsOutOfRange) {
    TRACY_FUNC(sceNgsVoiceGetParamsOutOfRange);
    return UNIMPLEMENTED();
}

std::map<std::string, std::ofstream> ngs_sound_log_files;

EXPORT(SceInt32, sceNgsVoiceGetStateData, ngs::Voice *voice, const SceUInt32 module, void *mem, const SceUInt32 mem_size) {
    TRACY_FUNC(sceNgsVoiceGetStateData, voice, module, mem, mem_size);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    ngs::ModuleData *storage = voice->module_storage(module);
    if (!storage)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    if (mem) {
        memcpy(mem, storage->voice_state_data.data(), std::min<std::size_t>(mem_size, storage->voice_state_data.size()));

	    if (mem_size > 128) {
	        std::string file_name = fmt::format("ngs_voice_{}_module_{}.dat", log_hex(voice_handle.address()), module);
	        std::ofstream &file = ngs_sound_log_files[file_name];
	        if (!file.is_open()) {
	            file.open(file_name, std::ios::binary | std::ios::out | std::ios::trunc);
	        }
	        file.write(reinterpret_cast<char *>(mem), mem_size);
	    }
	}
    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceInit, ngs::Voice *voice, const SceNgsVoicePreset *preset, const SceUInt32 init_flags) {
    TRACY_FUNC(sceNgsVoiceInit, voice, preset, init_flags);
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

EXPORT(SceInt32, sceNgsVoiceKeyOff, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoiceKeyOff, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return SCE_NGS_OK;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    voice->is_keyed_off = true;
    voice->rack->system->voice_scheduler.off(voice);

    // call the finish callback, I got no idea what the module id should be in this case
    voice->invoke_callback(emuenv.kernel, emuenv.mem, thread_id, voice->finished_callback, voice->finished_callback_user_data, 0);

    voice->is_keyed_off = false;
    voice->rack->system->voice_scheduler.stop(voice);
    return SCE_NGS_OK;
}

EXPORT(int, sceNgsVoiceKill, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoiceKill, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    voice->rack->system->voice_scheduler.stop(voice);

    return 0;
}

EXPORT(SceUInt32, sceNgsVoiceLockParams, ngs::Voice *voice, SceUInt32 module, SceUInt32 param_interface_id, SceNgsBufferInfo *buf) {
    TRACY_FUNC(sceNgsVoiceLockParams, voice, module, param_interface_id, buf);
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

EXPORT(SceInt32, sceNgsVoicePatchSetVolume, ngs::Patch *patch, const SceInt32 output_channel, const SceInt32 input_channel, const SceFloat32 vol) {
    TRACY_FUNC(sceNgsVoicePatchSetVolume, patch, output_channel, input_channel, vol);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!patch || patch->output_sub_index == -1)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    patch->volume_matrix[output_channel][input_channel] = vol;

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoicePatchSetVolumes, ngs::Patch *patch, const SceInt32 output_channel, const SceFloat32 *volumes, const SceInt32 vols) {
    TRACY_FUNC(sceNgsVoicePatchSetVolumes, patch, output_channel, volumes, vols);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!patch || patch->output_sub_index == -1)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    for (int i = 0; i < std::min(vols, 2); i++)
        patch->volume_matrix[output_channel][i] = volumes[i];

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoicePatchSetVolumesMatrix, ngs::Patch *patch, const SceNgsVolumeMatrix *matrix) {
    TRACY_FUNC(sceNgsVoicePatchSetVolumesMatrix, patch, matrix);
    if (!emuenv.cfg.current_config.ngs_enable)
        return 0;

    if (!patch || patch->output_sub_index == -1)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    patch->volume_matrix[0][0] = matrix->matrix[0][0];
    patch->volume_matrix[0][1] = matrix->matrix[0][1];
    patch->volume_matrix[1][0] = matrix->matrix[1][0];
    patch->volume_matrix[1][1] = matrix->matrix[1][1];

    return SCE_NGS_OK;
}

EXPORT(int, sceNgsVoicePause, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoicePause, voice);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice) {
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);
    }

    if (voice->is_paused)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_STATE);

    if (!voice->rack->system->voice_scheduler.pause(voice)) {
        return RET_ERROR(SCE_NGS_ERROR);
    }

    return SCE_NGS_OK;
}

EXPORT(SceUInt32, sceNgsVoicePlay, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoicePlay, voice);
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

EXPORT(int, sceNgsVoiceResume, ngs::Voice *voice) {
    TRACY_FUNC(sceNgsVoiceResume, voice);
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

EXPORT(SceInt32, sceNgsVoiceSetFinishedCallback, ngs::Voice *voice, Ptr<void> callback, Ptr<void> user_data) {
    TRACY_FUNC(sceNgsVoiceSetFinishedCallback, voice, callback, user_data);
    if (!emuenv.cfg.current_config.ngs_enable) {
        return 0;
    }

    if (!voice)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    voice->finished_callback = callback;
    voice->finished_callback_user_data = user_data;

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceSetModuleCallback, ngs::Voice *voice, const SceUInt32 module, Ptr<void> callback, Ptr<void> user_data) {
    TRACY_FUNC(sceNgsVoiceSetModuleCallback, voice, module, callback, user_data);
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

EXPORT(SceInt32, sceNgsVoiceSetParamsBlock, ngs::Voice *voice, const SceNgsModuleParamHeader *header,
    const SceUInt32 size, SceInt32 *pNumErrors) {
    TRACY_FUNC(sceNgsVoiceSetParamsBlock, voice, header, size, pNumErrors);
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

EXPORT(SceInt32, sceNgsVoiceSetPreset, ngs::Voice *voice, const SceNgsVoicePreset *preset) {
    TRACY_FUNC(sceNgsVoiceSetPreset, voice, preset);
    if (!emuenv.cfg.current_config.ngs_enable)
        return SCE_NGS_OK;

    if (!voice || !preset)
        return RET_ERROR(SCE_NGS_ERROR_INVALID_ARG);

    const std::lock_guard<std::mutex> guard(*voice->voice_mutex);
    if (!voice->set_preset(emuenv.mem, preset))
        return RET_ERROR(SCE_NGS_ERROR);

    return SCE_NGS_OK;
}

EXPORT(SceInt32, sceNgsVoiceUnlockParams, ngs::Voice *voice, const SceUInt32 module_index) {
    TRACY_FUNC(sceNgsVoiceUnlockParams, voice, module_index);
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

EXPORT(int, sceSulphaNgsGetDefaultConfig) {
    TRACY_FUNC(sceSulphaNgsGetDefaultConfig);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsGetNeededMemory) {
    TRACY_FUNC(sceSulphaNgsGetNeededMemory);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsInit) {
    TRACY_FUNC(sceSulphaNgsInit);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetRackName) {
    TRACY_FUNC(sceSulphaNgsSetRackName);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetSampleName) {
    TRACY_FUNC(sceSulphaNgsSetSampleName);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetSynthName) {
    TRACY_FUNC(sceSulphaNgsSetSynthName);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsSetVoiceName) {
    TRACY_FUNC(sceSulphaNgsSetVoiceName);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsShutdown) {
    TRACY_FUNC(sceSulphaNgsShutdown);
    return UNIMPLEMENTED();
}

EXPORT(int, sceSulphaNgsTrace) {
    TRACY_FUNC(sceSulphaNgsTrace);
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceNgsAT9GetSectionDetails)
BRIDGE_IMPL(sceNgsModuleGetNumPresets)
BRIDGE_IMPL(sceNgsModuleGetPreset)
BRIDGE_IMPL(sceNgsPatchCreateRouting)
BRIDGE_IMPL(sceNgsPatchGetInfo)
BRIDGE_IMPL(sceNgsPatchRemoveRouting)
BRIDGE_IMPL(sceNgsRackGetRequiredMemorySize)
BRIDGE_IMPL(sceNgsRackGetVoiceHandle)
BRIDGE_IMPL(sceNgsRackInit)
BRIDGE_IMPL(sceNgsRackRelease)
BRIDGE_IMPL(sceNgsRackSetParamErrorCallback)
BRIDGE_IMPL(sceNgsSystemGetRequiredMemorySize)
BRIDGE_IMPL(sceNgsSystemInit)
BRIDGE_IMPL(sceNgsSystemLock)
BRIDGE_IMPL(sceNgsSystemRelease)
BRIDGE_IMPL(sceNgsSystemSetFlags)
BRIDGE_IMPL(sceNgsSystemSetParamErrorCallback)
BRIDGE_IMPL(sceNgsSystemUnlock)
BRIDGE_IMPL(sceNgsSystemUpdate)
BRIDGE_IMPL(sceNgsVoiceBypassModule)
BRIDGE_IMPL(sceNgsVoiceDefGetAtrac9Voice)
BRIDGE_IMPL(sceNgsVoiceDefGetCompressorBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetCompressorSideChainBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetDelayBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetDistortionBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetEnvelopeBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetEqBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetMasterBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetMixerBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetPauserBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetPitchShiftBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetReverbBuss)
BRIDGE_IMPL(sceNgsVoiceDefGetSasEmuVoice)
BRIDGE_IMPL(sceNgsVoiceDefGetScreamAtrac9Voice)
BRIDGE_IMPL(sceNgsVoiceDefGetScreamVoice)
BRIDGE_IMPL(sceNgsVoiceDefGetSimpleAtrac9Voice)
BRIDGE_IMPL(sceNgsVoiceDefGetSimpleVoice)
BRIDGE_IMPL(sceNgsVoiceDefGetTemplate1)
BRIDGE_IMPL(sceNgsVoiceGetInfo)
BRIDGE_IMPL(sceNgsVoiceGetModuleBypass)
BRIDGE_IMPL(sceNgsVoiceGetModuleType)
BRIDGE_IMPL(sceNgsVoiceGetOutputPatch)
BRIDGE_IMPL(sceNgsVoiceGetParamsOutOfRange)
BRIDGE_IMPL(sceNgsVoiceGetStateData)
BRIDGE_IMPL(sceNgsVoiceInit)
BRIDGE_IMPL(sceNgsVoiceKeyOff)
BRIDGE_IMPL(sceNgsVoiceKill)
BRIDGE_IMPL(sceNgsVoiceLockParams)
BRIDGE_IMPL(sceNgsVoicePatchSetVolume)
BRIDGE_IMPL(sceNgsVoicePatchSetVolumes)
BRIDGE_IMPL(sceNgsVoicePatchSetVolumesMatrix)
BRIDGE_IMPL(sceNgsVoicePause)
BRIDGE_IMPL(sceNgsVoicePlay)
BRIDGE_IMPL(sceNgsVoiceResume)
BRIDGE_IMPL(sceNgsVoiceSetFinishedCallback)
BRIDGE_IMPL(sceNgsVoiceSetModuleCallback)
BRIDGE_IMPL(sceNgsVoiceSetParamsBlock)
BRIDGE_IMPL(sceNgsVoiceSetPreset)
BRIDGE_IMPL(sceNgsVoiceUnlockParams)
BRIDGE_IMPL(sceSulphaNgsGetDefaultConfig)
BRIDGE_IMPL(sceSulphaNgsGetNeededMemory)
BRIDGE_IMPL(sceSulphaNgsInit)
BRIDGE_IMPL(sceSulphaNgsSetRackName)
BRIDGE_IMPL(sceSulphaNgsSetSampleName)
BRIDGE_IMPL(sceSulphaNgsSetSynthName)
BRIDGE_IMPL(sceSulphaNgsSetVoiceName)
BRIDGE_IMPL(sceSulphaNgsShutdown)
BRIDGE_IMPL(sceSulphaNgsTrace)
