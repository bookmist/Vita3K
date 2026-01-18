#pragma once

#include <util/log.h>

#include <condition_variable>
#include <cstdint>
#include <regex>

namespace net_utils {

struct ProgressState {
    std::condition_variable cv;
    std::mutex mutex;
    bool download = true;
    bool pause = false;
};

struct ProgressData {
    uint64_t time;
    uint64_t bytes_already_downloaded;
};

struct AssignedAddr {
    std::string name; // Name of the interface
    std::string addr; // Assigned address
    std::string netMask; // Network mask
};

typedef const std::function<ProgressState *(float, uint64_t)> &ProgressCallback;
typedef std::pair<ProgressData, ProgressCallback> CallbackData;

bool download_file(const std::string &url, const std::string &output_file_path, ProgressCallback progress_callback = nullptr);
std::string get_web_response(const std::string &url);
std::string get_web_regex_result(const std::string &url, const std::regex &regex);

std::vector<AssignedAddr> get_all_assigned_addrs();
AssignedAddr get_selected_assigned_addr(int32_t &outIndex);
void init_address(int32_t &outIndex, uint32_t &netAddr, uint32_t &broadcastAddr);

} // namespace net_utils
