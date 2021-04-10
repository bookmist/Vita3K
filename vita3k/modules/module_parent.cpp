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

#include <modules/module_parent.h>

#include <cpu/functions.h>
#include <host/import_fn.h>
#include <host/import_var.h>
#include <host/load_self.h>
#include <host/state.h>
#include <io/device.h>
#include <io/vfs.h>
#include <kernel/functions.h>
#include <module/load_module.h>
#include <nids/functions.h>
#include <util/arm.h>
#include <util/find.h>
#include <util/lock_and_find.h>
#include <util/log.h>

#include <unordered_set>

static constexpr bool LOG_UNK_NIDS_ALWAYS = false;

#define VAR_NID(name, nid) extern const ImportVarFactory import_##name;
#define NID(name, nid) extern const ImportFn import_##name;
#include <nids/nids.h>
#undef NID
#undef VAR_NID

struct HostState;

static ImportFn resolve_import(uint32_t nid) {
    switch (nid) {
#define VAR_NID(name, nid)
#define NID(name, nid) \
    case nid:          \
        return import_##name;
#include <nids/nids.h>
#undef NID
#undef VAR_NID
    }

    return ImportFn();
}

const std::array<VarExport, var_exports_size> &get_var_exports() {
    static std::array<VarExport, var_exports_size> var_exports = {
#define NID(name, nid)
#define VAR_NID(name, nid) \
    {                      \
        nid,               \
        import_##name,     \
        #name              \
    },
#include <nids/nids.h>
#undef VAR_NID
#undef NID
    };
    return var_exports;
}

/**
 * \brief Resolves a function imported from a loaded module.
 * \param kernel Kernel state struct
 * \param nid NID to resolve
 * \return Resolved address, 0 if not found
 */
Address resolve_export(KernelState &kernel, uint32_t nid) {
    const ExportNids::iterator export_address = kernel.export_nids.find(nid);
    if (export_address == kernel.export_nids.end()) {
        return 0;
    }

    return export_address->second;
}

uint32_t resolve_nid(KernelState &kernel, Address addr) {
    auto nid = kernel.nid_from_export.find(addr);
    if (nid == kernel.nid_from_export.end()) {
        // resolve the thumbs address
        addr = addr | 1;
        nid = kernel.nid_from_export.find(addr);
        if (nid == kernel.nid_from_export.end()) {
            return 0;
        }
    }

    return nid->second;
}

std::string resolve_nid_name(KernelState &kernel, Address addr) {
    auto nid = resolve_nid(kernel, addr);
    if (nid == 0) {
        return "";
    }
    return import_name(nid);
}

static void log_import_call(char emulation_level, uint32_t nid, SceUID thread_id, const std::unordered_set<uint32_t> &nid_blacklist, Address lr) {
    if (nid_blacklist.find(nid) == nid_blacklist.end()) {
        const char *const name = import_name(nid);
        LOG_TRACE("[{}LE] TID: {:<3} FUNC: {} {} at {}", emulation_level, thread_id, log_hex(nid), name, log_hex(lr));
    }
}

struct SceAvPlayerInfo {
    SceAvPlayerMemoryAllocator memory_allocator;
    SceAvPlayerFileManager file_manager;
    SceAvPlayerEventManager event_manager;
    uint32_t debug_level;
    uint32_t base_priority;
    int32_t frame_buffer_count;
    int32_t auto_start;
    uint32_t unknown0;
};

void call_import(HostState &host, CPUState &cpu, uint32_t nid, SceUID thread_id) {
    Address export_pc = resolve_export(host.kernel, nid);

    if (!export_pc) {
        // HLE - call our C++ function
        if (is_returning(cpu))
            return;
        if (host.kernel.watch_import_calls) {
            const std::unordered_set<uint32_t> hle_nid_blacklist = {
                0xB295EB61, // sceKernelGetTLSAddr
                0x46E7BE7B, // sceKernelLockLwMutex
                0x91FA6614, // sceKernelUnlockLwMutex
                //
                /*
                0x29C34DF5, //sceGxmSetFragmentTexture
                0x9EB4380F, //sceGxmSetVertexTexture
                0x65DD0C84, //sceGxmSetUniformDataF
                0xDBA8D061, //sceGxmProgramParameterGetArraySize
                0xBD2998D1, //sceGxmProgramParameterGetComponentCount
                0xBB58267D, //sceGxmProgramParameterGetContainerIndex
                0x3148C6B6, //sceKernelLockLwMutexCB
                0xF65D4917, //sceGxmTextureGetType
                0x8FA3F9C3, //sceGxmProgramGetDefaultUniformBufferSize
                0xBC059AFC, //sceGxmDraw
                0x97118913, //sceGxmReserveVertexDefaultUniformBuffer
                0xB110C123, //sceKernelGetProcessTimeWide
                0xFA695FD7, //sceGxmTextureSetMagFilter
                0x416764E3, //sceGxmTextureSetMinFilter
                0x1CA9FE0B, //sceGxmTextureSetMipFilter
                0x4281763E, //sceGxmTextureSetUAddrMode
                0x126CDAA3, //sceGxmTextureSetVAddrMode
                0x575958A8, //sceGxmSetFrontFragmentProgramEnable
                0xEC94DFF7, //sceKernelSetEventFlag
                0xF32CBF34, //sceGxmSetFrontDepthWriteEnable
                0x14BD831F, //sceGxmSetFrontDepthFunc
                0x895DF2E9, //sceGxmSetVertexStream
                0xAD2F48D9, //sceGxmSetFragmentProgram
                0x31FF8ABD, //sceGxmSetVertexProgram
                0xAAA97F81, //sceGxmSetFrontDepthBias
                0xFD93209D, //sceGxmSetFrontPolygonMode
                0xE1CA72AE, //sceGxmSetCullMode
                0xB8645A9A, //sceGxmSetFrontStencilFunc
                0x7B1FABB6, //sceGxmReserveFragmentDefaultUniformBuffer
                0xE9F973B1, //sceKernelGetProcessTimeLow
                0xA93EA96, //sceNgsSystemUnlock
                0xB9D971F2, //sceNgsSystemLock
                0x1FBB0FE1, //sceKernelPollEventFlag
                0x9C0180E1, //sceKernelDelayThreadCB
                0x6FF9151, //sceGxmProgramGetParameter
                0xF33D9980, //sceGxmColorSurfaceGetStrideInPixels
                0x814C90AF, //sceDisplayWaitSetFrameBufCB
                0x7A410B64, //sceDisplaySetFrameBuf
                0x2DB6026C, //sceGxmColorSurfaceGetData
                0x8734FF4E, //sceGxmBeginScene
                0x70C86868, //sceGxmSetRegionClip
                0x6752183, //sceGxmSetFrontPointLineWidth
                0x3EB3380B, //sceGxmSetViewport
                0xFE300E2F, //sceGxmEndScene
                0xF5D3F3E8, //sceGxmDepthStencilSurfaceSetBackgroundStencil
                0xC44ACD7, //sceGxmDepthStencilSurfaceSetForceLoadMode
                0x8FA6FE44, //sceGxmSetFrontStencilRef
                0x32F280F0, //sceGxmDepthStencilSurfaceSetBackgroundDepth
                0x12AAA7AF, //sceGxmDepthStencilSurfaceSetForceStoreMode
                0xEC5C26B5, //sceGxmDisplayQueueAddEntry
                0x3D25FCE9, //sceGxmPadHeartbeat
                0x3B0AE9A9, //sceNpCheckCallback
                0x83C0E2AF, //sceKernelWaitEventFlag
                0xC9B8C0B4, //sceNgsVoiceGetStateData
                0x684F080C, //sceNgsSystemUpdate
                0x2DB3F5F, //sceAudioOutOutput
                0x4CB87CA7, //sceKernelClearEventFlag
                0xFB972F9,
                /**/
            };
            auto lr = read_lr(cpu);
            log_import_call('H', nid, thread_id, hle_nid_blacklist, lr);
        }
        const ImportFn fn = resolve_import(nid);
        if (fn) {
            fn(host, cpu, thread_id);
        } else if (host.missing_nids.count(nid) == 0 || LOG_UNK_NIDS_ALWAYS) {
            const ThreadStatePtr thread = lock_and_find(thread_id, host.kernel.threads, host.kernel.mutex);
            LOG_ERROR("Import function for NID {} not found (thread name: {}, thread ID: {})", log_hex(nid), thread->name, thread_id);

            if (!LOG_UNK_NIDS_ALWAYS)
                host.missing_nids.insert(nid);
        }
    } else {
        auto pc = read_pc(cpu);
        /*
        uint32_t *const stub = Ptr<uint32_t>(Address(pc)).get(host.mem);

        stub[0] = encode_arm_inst(INSTRUCTION_MOVW, (uint16_t)export_pc, 12);
        stub[1] = encode_arm_inst(INSTRUCTION_MOVT, (uint16_t)(export_pc >> 16), 12);
        stub[2] = encode_arm_inst(INSTRUCTION_BRANCH, 0, 12);
        */
        // LLE - directly run ARM code imported from some loaded module
        if (is_returning(cpu)) {
            LOG_TRACE("[LLE] TID: {:<3} FUNC: {} returned {}", thread_id, import_name(nid), log_hex(read_reg(cpu, 0)));
            return;
        }
        if (nid == 0x4C847ADF) {
            Ptr<SceAvPlayerInfo> p(read_reg(cpu, 0));
            p.get(host.mem)->debug_level = 3;
            LOG_DEBUG("NID(sceAvPlayerInit, 0x4C847ADF)");
        }
        const std::unordered_set<uint32_t> lle_nid_blacklist = {};
        log_import_call('L', nid, thread_id, lle_nid_blacklist, pc);
        write_pc(cpu, export_pc);
    }
}

/**
 * \return False on failure, true on success
 */
bool load_module(HostState &host, SceSysmoduleModuleId module_id) {
    LOG_INFO("Loading module ID: {}", log_hex(module_id));

    const auto module_paths = sysmodule_paths[module_id];

    for (std::string module_path : module_paths) {
        module_path = "sys/external/" + module_path + ".suprx";

        vfs::FileBuffer module_buffer;
        Ptr<const void> lib_entry_point;

        if (vfs::read_file(VitaIoDevice::vs0, module_buffer, host.pref_path, module_path)) {
            SceUID loaded_module_uid = load_self(lib_entry_point, host.kernel, host.mem, module_buffer.data(), module_path, host.cfg);
            const auto module = host.kernel.loaded_modules[loaded_module_uid];
            const auto module_name = module->module_name;

            if (loaded_module_uid >= 0) {
                LOG_INFO("Module {} (at \"{}\") loaded", module_name, module_path);
            } else {
                LOG_ERROR("Error when loading module at \"{}\"", module_path);
                return false;
            }

            if (lib_entry_point) {
                LOG_DEBUG("Running module_start of module: {}", module_name);

                Ptr<void> argp = Ptr<void>();
                auto inject = create_cpu_dep_inject(host);
                const SceUID module_thread_id = create_thread(lib_entry_point, host.kernel, host.mem, module_name, SCE_KERNEL_DEFAULT_PRIORITY_USER,
                    static_cast<int>(SCE_KERNEL_STACK_SIZE_USER_DEFAULT), inject, nullptr);
                const ThreadStatePtr module_thread = util::find(module_thread_id, host.kernel.threads);
                const auto ret = run_on_current(*module_thread, lib_entry_point, 0, argp);

                module_thread->to_do = ThreadToDo::exit;
                module_thread->something_to_do.notify_all(); // TODO Should this be notify_one()?

                const std::lock_guard<std::mutex> lock(host.kernel.mutex);
                host.kernel.running_threads.erase(module_thread_id);
                host.kernel.threads.erase(module_thread_id);
                LOG_INFO("Module {} (at \"{}\") module_start returned {}", module_name, module->path, log_hex(ret));
            }

        } else {
            LOG_ERROR("Module at \"{}\" not present", module_path);
            // ignore and assume it was loaded
        }
    }

    host.kernel.loaded_sysmodules.push_back(module_id);
    return true;
}

CPUDepInject create_cpu_dep_inject(HostState &host) {
    const CallImport call_import = [&host](CPUState &cpu, uint32_t nid, SceUID main_thread_id) {
        ::call_import(host, cpu, nid, main_thread_id);
    };
    const ResolveNIDName resolve_nid_name = [&host](Address addr) {
        return ::resolve_nid_name(host.kernel, addr);
    };
    auto get_watch_memory_addr = [&host](Address addr) {
        return ::get_watch_memory_addr(host.kernel, addr);
    };

    CPUDepInject inject;
    inject.call_import = call_import;
    inject.resolve_nid_name = resolve_nid_name;
    inject.trace_stack = host.cfg.stack_traceback;
    inject.get_watch_memory_addr = get_watch_memory_addr;
    inject.module_regions = host.kernel.module_regions;
    const CallSVC call_svc = [inject, &host](CPUState &cpu, uint32_t imm, Address pc) {
        uint32_t nid;
        if (is_returning(cpu)) {
            nid = *Ptr<uint32_t>(pc).get(host.mem);
        } else {
            nid = *Ptr<uint32_t>(pc + 4).get(host.mem);
        }
        inject.call_import(cpu, nid, get_thread_id(cpu));
    };
    inject.call_svc = call_svc;
    return inject;
}
