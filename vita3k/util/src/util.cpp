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

#include <codecvt> // std::codecvt_utf8
#include <map>
#include <sstream>
#include <string>
#include <util/fs.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <util/log_to_file.h>
std::map<std::string, fs::ofstream> log_files;
void log_to_file(const std::string &file_name, const char *data, const size_t size) {
    fs::ofstream &file = log_files[file_name];
    if (!file.is_open()) {
        file.open(file_name, std::ios::binary | std::ios::out | std::ios::trunc);
    }
    file.write(data, size);
}
