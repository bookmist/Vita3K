// LICENSE

// Original code is taken from taihen https://github.com/yifanlu/taiHEN and deeply modified to fit the needs of the project
#pragma once

#include "kernel/cpu_protocol.h"
#include "kernel/state.h"
#include "kernel/types.h"
/* module.c -- nid lookup utilities
 *
 * Copyright (C) 2016 Yifan Lu
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "taihen/error.h"
#include "taihen/taihen.h"

#include <emuenv/state.h>
#include <module/module.h>
#include <util/log.h>

struct sce_module_imports_1 {
    uint16_t size; // size of this structure; 0x34
    uint16_t version; //
    uint16_t flags; //
    uint16_t num_functions; // number of imported functions
    uint16_t num_vars; // number of imported variables
    uint16_t num_tls_vars; // number of imported TLS variables
    uint32_t reserved1; // ?
    uint32_t lib_nid; // NID of the module to link to
    Ptr<char> lib_name; // name of module
    uint32_t reserved2; // ?
    Ptr<uint32_t> func_nid_table; // array of function NIDs (numFuncs)
    Ptr<Ptr<void>> func_entry_table; // parallel array of pointers to stubs; they're patched by the loader to jump to the final code
    Ptr<uint32_t> var_nid_table; // NIDs of the imported variables (numVars)
    Ptr<Ptr<void>> var_entry_table; // array of pointers to "ref tables" for each variable
    Ptr<uint32_t> tls_nid_table; // NIDs of the imported TLS variables (numTlsVars)
    Ptr<Ptr<void>> tls_entry_table; // array of pointers to ???
};

struct sce_module_imports_2 {
    uint16_t size; // 0x24
    uint16_t version;
    uint16_t flags;
    uint16_t num_functions;
    uint32_t reserved1;
    uint32_t lib_nid;
    Ptr<char> lib_name;
    Ptr<uint32_t> func_nid_table;
    Ptr<Ptr<void>> func_entry_table;
    uint32_t unk1;
    uint32_t unk2;
};

typedef union sce_module_imports {
    uint16_t size;
    struct sce_module_imports_1 type1;
    struct sce_module_imports_2 type2;
} sce_module_imports_t;

typedef struct sce_module_exports {
    uint16_t size; // size of this structure; 0x20 for Vita 1.x
    uint8_t lib_version[2]; //
    uint16_t attribute; // ?
    uint16_t num_functions; // number of exported functions
    uint16_t num_vars; // number of exported variables
    uint16_t unk;
    uint32_t num_tls_vars; // number of exported TLS variables?  <-- pretty sure wrong // yifanlu
    uint32_t lib_nid; // NID of this specific export list; one PRX can export several names
    Ptr<char> lib_name; // name of the export module
    Ptr<uint32_t> nid_table; // array of 32-bit NIDs for the exports, first functions then vars
    Ptr<Ptr<void>> entry_table; // array of pointers to exported functions and then variables
} sce_module_exports_t;

typedef struct sce_module_info {
    uint16_t modattribute; // ??
    uint16_t modversion; // always 1,1?
    char modname[27]; ///< Name of the module
    uint8_t type; // 6 = user-mode prx?
    Address gp_value; // always 0 on ARM
    uint32_t ent_top; // beginning of the export list (sceModuleExports array)
    uint32_t ent_end; // end of same
    uint32_t stub_top; // beginning of the import list (sceModuleStubInfo array)
    uint32_t stub_end; // end of same
    uint32_t module_nid; // ID of the PRX? seems to be unused
    uint32_t field_38; // unused in samples
    uint32_t field_3C; // I suspect these may contain TLS info
    uint32_t field_40; //
    uint32_t mod_start; // 44 module start function; can be 0 or -1; also present in exports
    uint32_t mod_stop; // 48 module stop function
    uint32_t exidx_start; // 4c ARM EABI style exception tables
    uint32_t exidx_end; // 50
    uint32_t extab_start; // 54
    uint32_t extab_end; // 58
} sce_module_info_t; // 5c?

#define MOD_LIST_SIZE (256)

/** The currently running FW version. */
static constexpr uint32_t fw_version = DEFAULT_FW_VERSION;

#define SCE_TYPE(type) type##_raw
#define SCE_PTR(type) Ptr<type>

typedef struct sce_module_exports_raw {
    uint16_t size; /* Size of this struct, set to 0x20 */
    uint16_t version; /* 0x1 for normal export, 0x0 for main module export */
    uint16_t flags; /* 0x1 for normal export, 0x8000 for main module export */
    uint16_t num_syms_funcs; /* Number of function exports */
    uint32_t num_syms_vars; /* Number of variable exports */
    uint32_t num_syms_tls_vars; /* Number of TLS variable exports */
    uint32_t library_nid; /* NID of this library */
    Ptr<const char>
        library_name; /* Pointer to name of this library */
    Ptr<uint32_t>
        nid_table; /* Pointer to array of 32-bit NIDs to export */
    Ptr<const Ptr<void>>
        entry_table; /* Pointer to array of data pointers for each NID */
} sce_module_exports_raw;

typedef struct sce_module_imports_long_raw {
    uint16_t size; /* Size of this struct, set to 0x34 */
    uint16_t version; /* Set to 0x1 */
    uint16_t flags; /* Set to 0x0 */
    uint16_t num_syms_funcs; /* Number of function imports */
    uint16_t num_syms_vars; /* Number of variable imports */
    uint16_t num_syms_tls_vars; /* Number of TLS variable imports */

    uint32_t reserved1;
    uint32_t library_nid; /* NID of library to import */
    Ptr<const char>
        library_name; /* Pointer to name of imported library, for debugging */
    uint32_t reserved2;
    Ptr<uint32_t>
        func_nid_table; /* Pointer to array of function NIDs to import */
    Ptr<const Ptr<void>>
        func_entry_table; /* Pointer to array of stub functions to fill */
    Ptr<uint32_t>
        var_nid_table; /* Pointer to array of variable NIDs to import */
    Ptr<const Ptr<void>>
        var_entry_table; /* Pointer to array of data pointers to write to */
    Ptr<uint32_t>
        tls_var_nid_table; /* Pointer to array of TLS variable NIDs to import */
    Ptr<const Ptr<void>>
        tls_var_entry_table; /* Pointer to array of data pointers to write to */
} sce_module_imports_long_raw;

/* alternative module imports struct with a size of 0x24 */
typedef struct sce_module_imports_short_raw {
    uint16_t size; /* Size of this struct, set to 0x24 */
    uint16_t version; /* Set to 0x1 */
    uint16_t flags; /* Set to 0x0 */
    uint16_t num_syms_funcs; /* Number of function imports */
    uint16_t num_syms_vars; /* Number of variable imports */
    uint16_t num_syms_tls_vars; /* Number of TLS variable imports */

    uint32_t library_nid; /* NID of library to import */
    Ptr<const char>
        library_name; /* Pointer to name of imported library, for debugging */
    Ptr<uint32_t>
        func_nid_table; /* Pointer to array of function NIDs to import */
    Ptr<const Ptr<void>>
        func_entry_table; /* Pointer to array of stub functions to fill */
    Ptr<uint32_t>
        var_nid_table; /* Pointer to array of variable NIDs to import */
    Ptr<const Ptr<void>>
        var_entry_table; /* Pointer to array of data pointers to write to */
} sce_module_imports_short_raw;

typedef union sce_module_imports_raw {
    uint16_t size;
    sce_module_imports_long_raw type1;
    sce_module_imports_short_raw type2;
} sce_module_imports_raw_t;

struct sce_module_info_raw { // size is 0x5C-bytes
    uint16_t attributes;
    uint16_t version; /* Set to 0x0101 */
    char name[27]; /* Name of the library */
    uint8_t type; /* 0x0 for executable, 0x6 for PRX */
    Ptr<const void>
        gp_value;
    Ptr<sce_module_exports_raw>
        export_top; /* Offset to start of export table */
    Ptr<sce_module_exports_raw>
        export_end; /* Offset to end of export table */
    Ptr<sce_module_imports_raw>
        import_top; /* Offset to start of import table */
    Ptr<sce_module_imports_raw>
        import_end; /* Offset to end of import table */
    uint32_t module_nid; /* NID of this module */
    uint32_t tls_start;
    uint32_t tls_filesz;
    uint32_t tls_memsz;
    Ptr<const void>
        module_start; /* Offset to function to run when library is started, 0 to disable */
    Ptr<const void>
        module_stop; /* Offset to function to run when library is exiting, 0 to disable */
    Ptr<const void>
        exidx_top; /* Offset to start of ARM EXIDX (optional) */
    Ptr<const void>
        exidx_end; /* Offset to end of ARM EXIDX (optional) */
    Ptr<const void>
        extab_top; /* Offset to start of ARM EXTAB (optional) */
    Ptr<const void>
        extab_end; /* Offset to end of ARM EXTAB (optional */
};

/**
 * @brief      Converts internal SCE structure to a usable form
 *
 *             This is needed since the internal SceKernelModulemgr structures
 *             change in different firmware versions.
 *
 * @param[in]  pid      The pid
 * @param[in]  sceinfo  Return from `ksceKernelGetModuleInternal`
 * @param[out] taiinfo  Output data structure
 *
 * @return     Zero on success, < 0 on error
 */
static int sce_to_tai_module_info(SceUID pid, void *sceinfo, tai_module_info_t *taiinfo) {
    /*
    SceKernelFwInfo fwinfo;

  if (fw_version == 0) {
    fwinfo.size = sizeof(fwinfo);
    if (ksceKernelGetSystemSwVersion(&fwinfo) < 0) {
      fw_version = DEFAULT_FW_VERSION;
    } else {
      fw_version = fwinfo.version;
    }
    LOG("ksceKernelGetSystemSwVersion: 0x%08X", fw_version);
  }
  */
    char *info;

    if (taiinfo->size < sizeof(tai_module_info_t)) {
        LOG("Structure size too small: %d", taiinfo->size);
        return TAI_ERROR_SYSTEM;
    }

    info = (char *)sceinfo;
    if (fw_version >= 0x3600000) {
        if (pid == KERNEL_PID) {
            taiinfo->modid = *(SceUID *)(info + 0xC);
        } else {
            taiinfo->modid = *(SceUID *)(info + 0x10);
        }
        snprintf(taiinfo->name, 27, "%s", *(const char **)(info + 0x1C));
        taiinfo->name[26] = '\0';
        taiinfo->module_nid = *(uint32_t *)(info + 0x30);
        taiinfo->exports_start = *(uintptr_t *)(info + 0x20);
        taiinfo->exports_end = *(uintptr_t *)(info + 0x24);
        taiinfo->imports_start = *(uintptr_t *)(info + 0x28);
        taiinfo->imports_end = *(uintptr_t *)(info + 0x2C);
    } else if (fw_version >= 0x1692000) {
        if (pid == KERNEL_PID) {
            taiinfo->modid = *(SceUID *)(info + 0x0);
        } else {
            taiinfo->modid = *(SceUID *)(info + 0x4);
        }
        taiinfo->module_nid = *(uint32_t *)(info + 0x3C);
        snprintf(taiinfo->name, 27, "%s", (const char *)(info + 0xC));
        taiinfo->name[26] = '\0';
        taiinfo->exports_start = *(uintptr_t *)(info + 0x2C);
        taiinfo->exports_end = *(uintptr_t *)(info + 0x30);
        taiinfo->imports_start = *(uintptr_t *)(info + 0x34);
        taiinfo->imports_end = *(uintptr_t *)(info + 0x38);
    } else {
        LOG("Unsupported FW 0x%08X", fw_version);
        return TAI_ERROR_SYSTEM;
    }
    return TAI_SUCCESS;
}

/**
 * @brief      Finds an integer in userspace.
 *
 * This only finds 4-byte aligned integers in the specified range!
 *
 * @param[in]  pid     The pid
 * @param[in]  src     The source
 * @param[in]  needle  The needle
 * @param[in]  size    The size
 *
 * @return     0 if not found or the offset to the needle
 */
/*
static int find_int_for_user(Address src, uint32_t needle, SceSize size) {
int my_context[3];
int *other_context;
int flags;
uintptr_t end;
uint32_t data;
int count;
int ret;

count = 0;
end = (src + size) & ~3; // align to last 4 byte boundary
src = (src + 3) & ~3; // align to next 4 byte boundary
if (end <= src) {
    return 0;
}
if (ret >= 0) {
    while (count < size) {
        //asm("ldrt %0, [%1]" : "=r"(data) : "r"(src + count));
        if (data == needle) {
            break;
        }
        count += 4;
    }
}
if (count >= size) {
    return -1;
} else {
    return count;
}
}*/

/**
 * @brief      Gets a loaded module by name or NID or both
 *
 *             If `name` is NULL, then only the NID is used to locate the loaded
 *             module. If `name` is not NULL then it will be used to lookup the
 *             loaded module. If NID is not `TAI_IGNORE_MODULE_NID`, then it
 *             will be used in the lookup too. If `name` is NULL and NID is
 *             `TAI_IGNORE_MODULE_NID` then the first module loaded for the
 *             process will be returned.
 *
 * @param[in]  pid   The pid
 * @param[in]  name  The name to lookup. Can be NULL.
 * @param[in]  nid   The nid to lookup. Can be `TAI_IGNORE_MODULE_NID`.
 * @param[out] info  The information
 *
 * @return     Zero on success, < 0 on error
 *             - TAI_ERROR_INVALID_MODULE if both `name` and NID are undefined
 *               and `pid` is KERNEL_PID.
 *             - TAI_ERROR_MODULE_OVERFLOW if there are more than
 *               `MOD_LIST_SIZE` modules loaded for the process. This is a
 *               system error and should be reported.
 */
int module_get_by_name_nid(EmuEnvState &emuenv, SceUID pid, const char *module_name, uint32_t module_nid, tai_module_info_t *info) {
    if (info->size != sizeof(tai_module_info_t)) {
        LOG_ERROR("Structure size too small: %d", info->size);
        return TAI_ERROR_SYSTEM;
    }

    int module_id = 0;
    if (module_name == nullptr && module_nid == TAI_IGNORE_MODULE_NID) {
        for (auto &[module_id_, module] : emuenv.kernel.loaded_modules) {
            if (module->info.path == "app0:" + emuenv.self_path) {
                module_id = module_id_;
            }
        }
    } else if (module_nid != TAI_IGNORE_MODULE_NID) {
        for (auto &[module_id_, module_info] : emuenv.kernel.loaded_modules) {
            const sce_module_info_raw *int_mod_info = reinterpret_cast<const sce_module_info_raw *>(module_info->info_segment_address.get(emuenv.mem) + module_info->info_offset);
            if (int_mod_info->module_nid == module_nid) {
                module_id = module_id_;
            }
        }

    } else {
        for (auto &[module_id_, module] : emuenv.kernel.loaded_modules) {
            if (strncmp(module->info.module_name, module_name, sizeof(module->info.module_name)) == 0) {
                module_id = module_id_;
            }
        }
    }
    if (module_id == 0) {
        return TAI_ERROR_NOT_FOUND;
    }
    auto &module_info = emuenv.kernel.loaded_modules[module_id];
    const sce_module_info_raw *int_mod_info = reinterpret_cast<const sce_module_info_raw *>(module_info->info_segment_address.get(emuenv.mem) + module_info->info_offset);
    info->modid = module_id;
    info->module_nid = int_mod_info->module_nid;
    info->exports_start = int_mod_info->export_top;
    info->exports_end = int_mod_info->export_end;
    info->imports_start = int_mod_info->import_top;
    info->imports_end = int_mod_info->import_end;
    strncpy(info->name, int_mod_info->name, 27);
    info->name[26] = '\0';
    return TAI_SUCCESS;
}

/**
 * @brief      Gets an offset from a segment in a module
 *
 * @param[in]  pid     The pid of caller
 * @param[in]  modid   The module to offset from
 * @param[in]  segidx  Segment in module to offset from
 * @param[in]  offset  Offset from segment
 * @param[out] addr    Output final address
 *
 * @return     Zero on success, < 0 on error
 */
int module_get_offset(EmuEnvState &emuenv, SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr) {
    // SceKernelModuleInfo sceinfo;
    size_t count;
    int ret;

    if (segidx > 3) {
        LOG("Invalid segment index: %d", segidx);
        return TAI_ERROR_INVALID_ARGS;
    }
    LOG("Getting offset for pid:%x, modid:%x, segidx:%d, offset:%x", pid, modid, segidx, offset);
    // sceinfo.size = sizeof(sceinfo);
    const std::lock_guard<std::mutex> lock(emuenv.kernel.mutex);

    auto module = emuenv.kernel.loaded_modules.find(modid);
    if (module == emuenv.kernel.loaded_modules.end()) {
        LOG("Error getting module info for %d", modid);
        return SCE_KERNEL_ERROR_LIBRARYDB_NO_MOD;
    }
    auto &sceinfo = module->second->info;

    if (offset > sceinfo.segments[segidx].memsz) {
        LOG("Offset %x overflows segment size %x", offset, sceinfo.segments[segidx].memsz);
        return TAI_ERROR_INVALID_ARGS;
    }
    *addr = sceinfo.segments[segidx].vaddr.address() + offset;
    LOG("found address: 0x%08X", *addr);

    return TAI_SUCCESS;
}

/**
 * @brief      Gets an exported function address
 *
 * @param[in]  pid      The pid
 * @param[in]  modname  The name of module to lookup
 * @param[in]  libnid   NID of the exporting library. Can be `TAI_ANY_LIBRARY`.
 * @param[in]  funcnid  NID of the exported function
 * @param[out] func     Output address of the function
 *
 * @return     Zero on success, < 0 on error
 */
int module_get_export_func(EmuEnvState &emuenv, SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, Address *func) {
    sce_module_exports_t local;
    tai_module_info_t info;
    //    sce_module_exports_t *export_;
    Address cur;
    int found;
    int i;
    int ret;

    LOG("Getting export for pid:%x, modname:%s, libnid:%x, funcnid:%x", pid, modname, libnid, funcnid);
    info.size = sizeof(info);
    if (module_get_by_name_nid(emuenv, pid, modname, TAI_IGNORE_MODULE_NID, &info) < 0) {
        LOG("Failed to find module: %s", modname);
        return TAI_ERROR_NOT_FOUND;
    }

    for (cur = info.exports_start.address(); cur < info.exports_end.address();) {
        sce_module_exports_t *export_ = Ptr<sce_module_exports_t>(cur).get(emuenv.mem);

        if (libnid == TAI_ANY_LIBRARY || export_->lib_nid == libnid) {
            for (i = 0; i < export_->num_functions; i++) {
                if (export_->nid_table.get(emuenv.mem)[i] == funcnid) {
                    *func = export_->entry_table.get(emuenv.mem)[i].address();
                    LOG("found kernel address: 0x%08X", *func);
                    return TAI_SUCCESS;
                }
            }
        }
        cur += export_->size;
    }

    return TAI_ERROR_NOT_FOUND;
}

/**
 * @brief      Gets an imported function stub address
 *
 * @param[in]  pid            The pid
 * @param[in]  modname        The name of the module importing the function
 * @param[in]  target_libnid  The target's library NID. Can be `TAI_ANY_LIBRARY`
 * @param[in]  funcnid        The target's function NID
 * @param[out] stub           Output address to stub calling the imported
 *                            function
 *
 * @return     Zero on success, < 0 on error
 */
int module_get_import_func(EmuEnvState &emuenv, SceUID pid, const char *modname, uint32_t target_libnid, uint32_t funcnid, Address *stub) {
    // sce_module_imports_t local;
    tai_module_info_t info;
    // sce_module_imports_t *import;
    // int found;
    int i;
    // int ret;

    LOG("Getting import for pid:%x, modname:%s, target_libnid:%x, funcnid:%x", pid, modname, target_libnid, funcnid);
    info.size = sizeof(info);
    if (module_get_by_name_nid(emuenv, pid, modname, TAI_IGNORE_MODULE_NID, &info) < 0) {
        LOG("Failed to find module: %s", modname);
        return TAI_ERROR_NOT_FOUND;
    }

    for (Address cur = info.imports_start.address(); cur < info.imports_end.address();) {
        sce_module_imports_t *import = Ptr<sce_module_imports_t>(cur).get(emuenv.mem);

        // LOG("import size is 0x%04X", import->size);
        if (import->size == sizeof(struct sce_module_imports_1)) {
            if (target_libnid == TAI_ANY_LIBRARY || import->type1.lib_nid == target_libnid) {
                for (i = 0; i < import->type1.num_functions; i++) {
                    if (import->type1.func_nid_table.get(emuenv.mem)[i] == funcnid) {
                        *stub = import->type1.func_entry_table.get(emuenv.mem)[i].address();
                        LOG("found kernel address: 0x%08X", *stub);
                        return TAI_SUCCESS;
                    }
                }
            }
        } else if (import->size == sizeof(struct sce_module_imports_2)) {
            if (target_libnid == TAI_ANY_LIBRARY || import->type2.lib_nid == target_libnid) {
                for (i = 0; i < import->type2.num_functions; i++) {
                    if (import->type2.func_nid_table.get(emuenv.mem)[i] == funcnid) {
                        *stub = import->type2.func_entry_table.get(emuenv.mem)[i].address();
                        LOG("found kernel address: 0x%08X", *stub);
                        return TAI_SUCCESS;
                    }
                }
            }
        } else {
            LOG("Invalid import size: %d", import->size);
        }
        cur += import->size;
    }

    return TAI_ERROR_NOT_FOUND;
}
