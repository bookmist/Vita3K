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

#include "SceRegMgrForSDK.h"

#include <io/state.h>

EXPORT(int, sceRegMgrUtilityGetBin, const int id, char *buf, int size) {
    LOG_DEBUG("id: {}, size: {}", log_hex(id), size);
    // 0x00450F32: /CONFIG/NP/account_id
    return UNIMPLEMENTED();
}

const std::vector<unsigned char> xorKey = { 0x89, 0xFA, 0x95, 0x48, 0xCB, 0x6D, 0x77, 0x9D, 0xA2, 0x25, 0x34, 0xFD, 0xA9, 0x35, 0x59, 0x6E };

void decryptRegistryFile(const fs::path reg_path) {
    // Ouverture du fichier en mode binaire
    std::ifstream file(reg_path.string(), std::ios::binary);
    if (!file) {
        LOG_ERROR("Erreur lors de l'ouverture du fichier.");
        return;
    }

    // Lecture des données chiffrées depuis le fichier
    std::vector<uint8_t> encryptedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Vérification de la taille des données
    if (encryptedData.empty()) {
        std::cout << "Le fichier est vide." << std::endl;
        return;
    }

    encryptedData.erase(encryptedData.begin(), encryptedData.begin() + 138);

    for (size_t i = 0; i < encryptedData.size(); i++) {
        encryptedData[i] ^= xorKey[i & 0xF];
    }

    std::string out;
    for (const auto &byte : encryptedData) {
        out += byte;
    }

    // Ecriture des données déchiffrées dans un fichier
    /* std::ofstream outFile("registry.txt", std::ios::binary);
    if (!outFile) {
                LOG_ERROR("Erreur lors de l'ouverture du fichier.");
                return;
        }
    outFile.write(out.c_str(), out.size()); // (out.c_str(), out.size()
    outFile.close();*/

    LOG_DEBUG("out: {}", out);
}

void loadSystemRegFile(const fs::path system_reg_path) {
    // Ouverture du fichier en mode binaire
    std::ifstream file(system_reg_path.string(), std::ios::binary);
    if (!file) {
        LOG_ERROR("Erreur lors de l'ouverture du fichier.");
        return;
    }

    // Lecture des données chiffrées depuis le fichier
    std::vector<uint8_t> systemRegData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Vérification de la taille des données
    if (systemRegData.empty()) {
        std::cout << "Le fichier est vide." << std::endl;
        return;
    }

    std::string out;
    for (const auto &byte : systemRegData) {
        out += byte;
    }

    // Ecriture des données déchiffrées dans un fichier
    std::ofstream outFile("system.txt", std::ios::binary);
    if (!outFile) {
        LOG_ERROR("Erreur lors de l'ouverture du fichier.");
        return;
    }
    outFile.write(out.c_str(), out.size());
    outFile.close();

    LOG_DEBUG("out: {}", out);
}

EXPORT(int, sceRegMgrUtilityGetInt, const int id, int *buf) {
    // decryptRegistryFile(fs::path(emuenv.pref_path) / "os0/kd/registry.db0");
    // loadSystemRegFile(fs::path(emuenv.pref_path) / "vd0/registry/system.dreg");

    switch (id) {
    case 0x00037502: // /CONFIG/SYSTEM/language
        *buf = emuenv.cfg.sys_lang;
        LOG_WARN("using Cfg value of language");
        break;
    case 0x00229142: // /CONFIG/SYSTEM/button_assign
        *buf = emuenv.cfg.sys_button;
        LOG_WARN("using Cfg value of button assign");
        break;
    case 0x00683DCD: // /CONFIG/SYSTEM/key_pad
        *buf = 1;
        LOG_WARN("using default value of key pad");
        break;
    case 0x00088776: // /CONFIG/DATE/date_format
        *buf = emuenv.cfg.sys_date_format;
        LOG_WARN("using cfg value of date format");
        break;
    case 0x00668503: // /CONFIG/DATE/time_format
        *buf = emuenv.cfg.sys_time_format;
        LOG_WARN("using cfg value of time format");
        break;
    default:
        LOG_WARN("unknown id: {}", log_hex(id));
        return UNIMPLEMENTED();
    }

    return 0;
}

EXPORT(int, sceRegMgrUtilityGetStr, const int id, char *buf, const int size) {
    switch (id) {
    case 0x00598438: // /CONFIG/SYSTEM/username
        strncpy(buf, emuenv.io.user_name.c_str(), size);
        LOG_WARN("Using user name of current user: {}", buf);
        break;
    default:
        LOG_WARN("unknown id: {}", log_hex(id));
        break;
    }

    return 0;
}

EXPORT(int, sceRegMgrUtilitySetBin) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrUtilitySetInt) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceRegMgrUtilitySetStr) {
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceRegMgrUtilityGetBin)
BRIDGE_IMPL(sceRegMgrUtilityGetInt)
BRIDGE_IMPL(sceRegMgrUtilityGetStr)
BRIDGE_IMPL(sceRegMgrUtilitySetBin)
BRIDGE_IMPL(sceRegMgrUtilitySetInt)
BRIDGE_IMPL(sceRegMgrUtilitySetStr)
