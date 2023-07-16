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

#include "io/functions.h"
#include "io/io.h"

#include <modules/module_parent.h>

#include <cpu/functions.h>
#include <emuenv/state.h>
#include <io/device.h>
#include <io/state.h>
#include <io/vfs.h>
#include <kernel/load_self.h>
#include <kernel/state.h>
#include <module/load_module.h>
#include <nids/functions.h>
#include <util/arm.h>
#include <util/find.h>
#include <util/lock_and_find.h>
#include <util/log.h>

#include <unordered_set>

static constexpr bool LOG_UNK_NIDS_ALWAYS = false;

#define LIBRARY(name) extern const LibraryInitFn import_library_init_##name;
#include <modules/library_init_list.inc>
#undef LIBRARY

#define VAR_NID(name, nid) extern const ImportVarFactory import_##name;
#define NID(name, nid) extern const ImportFn import_##name;
#include <nids/nids.inc>
#undef NID
#undef VAR_NID

struct EmuEnvState;

static ImportFn resolve_import(uint32_t nid) {
    switch (nid) {
#define VAR_NID(name, nid)
#define NID(name, nid) \
    case nid:          \
        return import_##name;
#include <nids/nids.inc>
#undef NID
#undef VAR_NID
    }

    return ImportFn();
}

const std::array<VarExport, var_exports_size> &get_var_exports() {
    static std::array<VarExport, var_exports_size> var_exports = { {
#define NID(name, nid)
#define VAR_NID(name, nid) \
    {                      \
        nid,               \
        import_##name,     \
        #name              \
    },
#include <nids/nids.inc>
#undef VAR_NID
#undef NID
    } };
    return var_exports;
}

/**
 * \brief Resolves a function imported from a loaded module.
 * \param kernel Kernel state struct
 * \param nid NID to resolve
 * \return Resolved address, 0 if not found
 */
Address resolve_export(KernelState &kernel, uint32_t nid) {
    const std::shared_lock<std::shared_mutex> lock(kernel.export_nids_mutex);
    const ExportNids::iterator export_address = kernel.export_nids.find(nid);
    if (export_address == kernel.export_nids.end()) {
        return 0;
    }

    return export_address->second;
}

Ptr<void> create_vtable(const std::vector<uint32_t> &nids, MemState &mem) {
    // we need 4 bytes for the function pointer and 12 bytes for the syscall
    const uint32_t vtable_size = nids.size() * 4 * sizeof(uint32_t);
    Ptr<void> vtable = Ptr<void>(alloc(mem, vtable_size, "vtable"));
    uint32_t *function_pointer = vtable.cast<uint32_t>().get(mem);
    uint32_t *function_svc = function_pointer + nids.size();
    uint32_t function_location = vtable.address() + nids.size() * sizeof(uint32_t);
    for (uint32_t nid : nids) {
        *function_pointer = function_location;
        // encode svc call
        function_svc[0] = 0xef000000; // svc #0 - Call our interrupt hook.
        function_svc[1] = 0xe1a0f00e; // mov pc, lr - Return to the caller.
        function_svc[2] = nid; // Our interrupt hook will read this.

        function_pointer++;
        function_svc += 3;
        function_location += 3 * sizeof(uint32_t);
    }
    return vtable;
}

static void log_import_call(char emulation_level, uint32_t nid, SceUID thread_id, const std::unordered_set<uint32_t> &nid_blacklist, Address lr) {
    if (nid_blacklist.find(nid) == nid_blacklist.end()) {
        const char *const name = import_name(nid);
        LOG_TRACE("[{}LE] TID: {:<3} FUNC: {} {} at {}", emulation_level, thread_id, log_hex(nid), name, log_hex(lr));
    }
}

void call_import(EmuEnvState &emuenv, CPUState &cpu, uint32_t nid, SceUID thread_id) {
    if (nid == 0xF3917021) {
        static Ptr<char> rep = Ptr<char>(alloc(emuenv.mem, 0x10, "ip_field"));
        strcpy(rep.get(emuenv.mem), "127.0.0.1");
        write_reg(cpu, 0, rep.address());
        return;
    }

    Address export_pc = resolve_export(emuenv.kernel, nid);

    if (!export_pc) {
        // HLE - call our C++ function
        if (emuenv.kernel.debugger.watch_import_calls) {
            const std::unordered_set<uint32_t> hle_nid_blacklist = {
                0xB295EB61, // sceKernelGetTLSAddr
                0x46E7BE7B, // sceKernelLockLwMutex
                0x91FA6614, // sceKernelUnlockLwMutex
            };
            auto lr = read_lr(cpu);
            log_import_call('H', nid, thread_id, hle_nid_blacklist, lr);
        }
        const ImportFn fn = resolve_import(nid);
        if (fn) {
            fn(emuenv, cpu, thread_id);
        } else if (emuenv.missing_nids.count(nid) == 0 || LOG_UNK_NIDS_ALWAYS) {
            const ThreadStatePtr thread = lock_and_find(thread_id, emuenv.kernel.threads, emuenv.kernel.mutex);
            LOG_ERROR("Import function for NID {} not found (thread name: {}, thread ID: {})", log_hex(nid), thread->name, thread_id);
            LOG_DEBUG("{}\n{}", save_context(*thread->cpu).description(), thread->log_stack_traceback(emuenv.kernel));

            if (!LOG_UNK_NIDS_ALWAYS)
                emuenv.missing_nids.insert(nid);
        }
    } else {
        auto pc = read_pc(cpu);

        assert((pc & 1) == 0);

        pc -= 4; // Move back to SVC (SuperVisor Call) instruction

        uint32_t *const stub = Ptr<uint32_t>(Address(pc)).get(emuenv.mem);

        stub[0] = encode_arm_inst(INSTRUCTION_MOVW, (uint16_t)export_pc, 12);
        stub[1] = encode_arm_inst(INSTRUCTION_MOVT, (uint16_t)(export_pc >> 16), 12);
        stub[2] = encode_arm_inst(INSTRUCTION_BRANCH, 0, 12);

        // LLE - directly run ARM code imported from some loaded module
        // TODO: resurrect this
        /*if (is_returning(cpu)) {
            LOG_TRACE("[LLE] TID: {:<3} FUNC: {} returned {}", thread_id, import_name(nid), log_hex(read_reg(cpu, 0)));
            return;
        }*/

        const std::unordered_set<uint32_t> lle_nid_blacklist = {};
        log_import_call('L', nid, thread_id, lle_nid_blacklist, pc);
        write_pc(cpu, export_pc);
        // TODO: invalidate cache for all threads. Now invalidate_jit_cache is not thread safe.
        invalidate_jit_cache(cpu, pc, 4 * 3);
    }
}

SceUID load_module(EmuEnvState &emuenv, const std::string &module_path) {
    // Check if module is already loaded
    const auto &loaded_modules = emuenv.kernel.loaded_modules;
    auto module_iter = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](const auto &p) {
        return std::string(p.second->path) == module_path;
    });

    if (module_iter != loaded_modules.end()) {
        return module_iter->first;
    }
    LOG_INFO("Loading module \"{}\"", module_path);
    vfs::FileBuffer module_buffer;
    VitaIoDevice device = device::get_device(module_path);
    auto translated_module_path = translate_path(module_path.c_str(), device, emuenv.io);
    if (!vfs::read_file(device, module_buffer, emuenv.pref_path, translated_module_path)) {
        LOG_ERROR("Failed to read module file {}", module_path);
        return 0;
        // return SCE_ERROR_ERRNO_ENOENT;
    }
    SceUID module_id = load_self(emuenv.kernel, emuenv.mem, module_buffer.data(), module_path);
    if (module_id >= 0) {
        const auto module = emuenv.kernel.loaded_modules[module_id];
        LOG_INFO("Module {} (at \"{}\") loaded", module->module_name, module_path);
    } else {
        LOG_ERROR("Failed to load module {}", module_path);
    }
    return module_id;
}

uint32_t start_module(EmuEnvState &emuenv, const std::shared_ptr<SceKernelModuleInfo> &module, SceSize args, const Ptr<void> argp) {
    const auto module_start = module->start_entry;
    if (module_start) {
        const auto module_name = module->module_name;

        LOG_DEBUG("Running module_start of library: {} at address {}", module_name, log_hex(module_start.address()));
        SceInt32 priority = SCE_KERNEL_DEFAULT_PRIORITY_USER;
        SceInt32 stack_size = SCE_KERNEL_STACK_SIZE_USER_MAIN;
        SceInt32 affinity = SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT;
        // module_start is always called from new thread
        const ThreadStatePtr module_thread = emuenv.kernel.create_thread(emuenv.mem, module_name, module_start, priority, affinity, stack_size, nullptr);

        const auto ret = module_thread->run_guest_function(emuenv.kernel, module_start.address(), args, argp);
        module_thread->exit_delete(emuenv.kernel);

        LOG_INFO("Module {} (at \"{}\") module_start returned {}", module_name, module->path, log_hex(ret));
        return ret;
    }
    return 0;
}

/**
 * \return False on failure, true on success
 */
bool load_sys_module(EmuEnvState &emuenv, SceSysmoduleModuleId module_id) {
    const auto &module_paths = sysmodule_paths[module_id];
    for (const auto module_filename : module_paths) {
        std::string module_path;
        if (module_id == SCE_SYSMODULE_SMART || module_id == SCE_SYSMODULE_FACE || module_id == SCE_SYSMODULE_ULT) {
            module_path = fmt::format("app0:sce_module/{}.suprx", module_filename);
        } else {
            module_path = fmt::format("vs0:sys/external/{}.suprx", module_filename);
        }

        auto loaded_module_uid = load_module(emuenv, module_path);

        if (loaded_module_uid < 0) {
            if (module_id == SCE_SYSMODULE_ULT && loaded_module_uid == SCE_ERROR_ERRNO_ENOENT) {
                module_path = fmt::format("vs0:sys/external/{}.suprx", module_filename);
                loaded_module_uid = load_module(emuenv, module_path);
                if (loaded_module_uid < 0)
                    return false;
            } else
                return false;
        }
        const auto module = emuenv.kernel.loaded_modules[loaded_module_uid];
        start_module(emuenv, module);
    }

    emuenv.kernel.loaded_sysmodules.push_back(module_id);
    return true;
}

bool load_sys_module_internal_with_arg(EmuEnvState &emuenv, SceUID thread_id, SceSysmoduleInternalModuleId module_id, SceSize args, Ptr<void> argp, int *retcode) {
    LOG_INFO("Loading internal module ID: {}", log_hex(module_id));

    if (!sysmodule_internal_paths.contains(module_id))
        return false;

    const auto &module_paths = sysmodule_internal_paths.at(module_id);

    for (const auto module_filename : module_paths) {
        std::string module_path = fmt::format("vs0:sys/external/{}.suprx", module_filename);
        auto loaded_module_uid = load_module(emuenv, module_path);

            if (loaded_module_uid < 0) {
                return false;
            }
            const auto module = emuenv.kernel.loaded_modules[loaded_module_uid];
        auto ret = start_module(emuenv, module, args, argp);
                if (retcode)
                    *retcode = static_cast<int>(ret);
            }
    emuenv.kernel.loaded_internal_sysmodules.push_back(module_id);
    return true;
}

int load_app_by_path(EmuEnvState &emuenv, const std::string &self_path, const char *titleid, const char *app_param) {
    vfs::FileBuffer self_buffer;
    const auto device = device::get_device(self_path);
    const auto relative_path = device::remove_device_from_path(self_path, device);
    LOG_DEBUG("device: {}, relative_path: {}", device._to_string(), relative_path);
    const auto res = vfs::read_file(device, self_buffer, emuenv.pref_path, relative_path);
    if (res) {
        SceUID module_id = load_self(emuenv.kernel, emuenv.mem, self_buffer.data(), self_path);
        if (module_id >= 0) {
            const auto module = emuenv.kernel.loaded_modules[module_id];
            Ptr<const void> entry_point = module->start_entry;
            const ThreadStatePtr thread = emuenv.kernel.create_thread(emuenv.mem, titleid, entry_point, SCE_KERNEL_DEFAULT_PRIORITY_USER, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, static_cast<int>(SCE_KERNEL_STACK_SIZE_USER_MAIN), nullptr);
            const SceUID self_thread_id = thread->id;

            LOG_INFO("Self executable {} ({}) loaded", module->module_name, self_path);

            if (entry_point) {
                LOG_DEBUG("Running module_start of module: {}", module->module_name);
                SceKernelThreadOptParam param{ 0, 0 };
                if (app_param) {
                    std::vector<uint8_t> buf;
                    buf.insert(buf.end(), app_param, app_param + strlen(app_param) + 1);
                    auto arr = Ptr<uint8_t>(alloc(emuenv.mem, static_cast<uint32_t>(buf.size()), "arg"));
                    memcpy(arr.get(emuenv.mem), buf.data(), buf.size());
                    param.size = SceSize(buf.size());
                    param.attr = arr.address();
                }
                const ThreadStatePtr self_thread = util::find(self_thread_id, emuenv.kernel.threads);
                if (self_thread->start(emuenv.kernel, param.size, Ptr<void>(param.attr)) < 0) {
                    LOG_ERROR("Error when starting main thread of module at \"{}\"", self_path);
                    return -1;
                }
                return self_thread_id;
            } else {
                LOG_ERROR("Error when loading self executable at \"{}\"", self_path);
                return -1;
            }
        } else {
            LOG_ERROR("Executable at \"{}\" not present", self_path);
            return -1;
        }
    } else {
        LOG_ERROR("Error when reading executable at \"{}\"", self_path);
        return -1;
    }
}

void init_libraries(EmuEnvState &emuenv) {
#define LIBRARY(name) import_library_init_##name(emuenv);
#include <modules/library_init_list.inc>
#undef LIBRARY
}
