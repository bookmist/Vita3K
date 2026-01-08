// Vita3K emulator project
// Copyright (C) 2026 Vita3K team
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

#include <util/log.h>
#include <util/net_utils.h>

#include <curl/curl.h>

#ifdef _WIN32
#include <iphlpapi.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#include <condition_variable>
#include <mutex>

namespace net_utils {

std::string get_web_response(const std::string &url) {
    auto curl = curl_easy_init();
    if (!curl)
        return {};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Vita3K Emulator");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

#ifdef __ANDROID__
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

    std::string response_string;
    const auto writeFunc = +[](void *ptr, size_t size, size_t nmemb, std::string *data) {
        data->append((char *)ptr, size * nmemb);
        return size * nmemb;
    };
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    long response_code;
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(curl);

    if (response_code / 100 == 2)
        return response_string;

    return {};
}

std::string get_web_regex_result(const std::string &url, const std::regex &regex) {
    std::string result;

    // Get the response of the web
    const auto response = get_web_response(url);

    // Check if the response is not empty
    if (!response.empty()) {
        std::smatch match;
        // Check if the response matches the regex
        if (std::regex_search(response, match, regex)) {
            result = match[1];
        } else
            LOG_ERROR("No success found regex: {}", response);
    }

    return result;
}

static uint64_t get_current_time_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

typedef int (*CurlDownloadCallback)(void *c, long t, long d);

bool download_file(const std::string &url, const std::string &output_file_path, ProgressCallback progress_callback) {
    CURL *curl_download = curl_easy_init();
    if (!curl_download)
        return false;

    CurlDownloadCallback curl_callback = +[](void *user_data, long total_bytes, long downloaded_bytes) {
        const auto data = (CallbackData *)user_data;

        if ((total_bytes == 0) || (downloaded_bytes == 0))
            return 0; // Ignore if we dont have the total yet or if we haven't downloaded anything

        if (!data->second)
            return 0;

        // Calculate progress percentage
        const auto total_bytes_downloaded = static_cast<double>(downloaded_bytes + data->first.bytes_already_downloaded);
        const auto file_size = total_bytes + data->first.bytes_already_downloaded;
        const auto progress_percent = static_cast<float>(total_bytes_downloaded) / static_cast<float>(file_size) * 100.0f;

        // Calculate elapsed time
        const auto current_time = get_current_time_ms();
        const auto elapsed_time_ms = std::difftime(current_time, data->first.time);

        // Calculate remaining time in seconds
        const auto remaining_bytes = static_cast<double>(total_bytes - downloaded_bytes);
        const auto remaining_time = static_cast<uint64_t>((remaining_bytes / downloaded_bytes) * elapsed_time_ms) / 1000;

        ProgressState *callback_result = data->second(progress_percent, remaining_time);

        std::unique_lock<std::mutex> lock(callback_result->mutex);

        // Store the current pause state
        const auto pause = callback_result->pause;

        // Wait until the pause state becomes false (unpaused)
        callback_result->cv.wait(lock, [&]() {
            return !callback_result->pause;
        });

        // When coming out of pause, add the time spent to the start time to keep the time consistent
        if (pause)
            data->first.time += std::difftime(get_current_time_ms(), current_time);

        if (!callback_result->download) {
            return 1; // Returning anything that's not 0 aborts the request
        }
        return 0;
    };

    const auto start_time = get_current_time_ms();
    const uint64_t bytes_already_downloaded = fs::exists(output_file_path) ? fs::file_size(output_file_path) : 0;
    const auto callbackData = CallbackData({ start_time, bytes_already_downloaded }, progress_callback);

    curl_easy_setopt(curl_download, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_download, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_download, CURLOPT_NOPROGRESS, false); // Enable progress function
    curl_easy_setopt(curl_download, CURLOPT_RESUME_FROM_LARGE, bytes_already_downloaded);
    curl_easy_setopt(curl_download, CURLOPT_XFERINFODATA, &callbackData);
    curl_easy_setopt(curl_download, CURLOPT_XFERINFOFUNCTION, curl_callback);
    curl_easy_setopt(curl_download, CURLOPT_FOLLOWLOCATION, true); // Follow redirects

#ifdef __ANDROID__
    curl_easy_setopt(curl_download, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

    auto fp = fopen(output_file_path.c_str(), "ab");
    if (!fp) {
        LOG_CRITICAL("Could not fopen file {}", output_file_path);
        curl_easy_cleanup(curl_download);
        if (fs::exists(output_file_path))
            fs::remove(output_file_path);
        return false;
    }

    curl_easy_setopt(curl_download, CURLOPT_WRITEDATA, fp);
    int res = curl_easy_perform(curl_download);

    fclose(fp);
    curl_easy_cleanup(curl_download);
    if (progress_callback)
        progress_callback(0, 0);

    if (res == CURLE_ABORTED_BY_CALLBACK)
        LOG_CRITICAL("Aborted update by user");

    return res == CURLE_OK;
}

std::vector<AssignedAddr> get_all_assigned_addrs() {
    std::vector<AssignedAddr> out_addrs;
    const auto ret_addrs = [&out_addrs]() {
        if (out_addrs.empty())
            out_addrs.push_back({ "localhost", "127.0.0.1", "255.255.255.255" });

        return out_addrs;
    };

#ifdef _WIN32
    DWORD dwRetVal = 0;
    // UINT i;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        LOG_CRITICAL("Error allocating memory needed to call GetAdaptersinfo");
        return ret_addrs();
    }
    // Make an initial call to GetAdaptersInfo to get the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            LOG_CRITICAL("Error allocating memory needed to call GetAdaptersinfo");
            return ret_addrs();
        }
    }
    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        const std::string noAddress = "0.0.0.0";
        while (pAdapter) {
            IP_ADDR_STRING *pIPAddr = &pAdapter->IpAddressList;
            while (pIPAddr) {
                if (noAddress.compare(pIPAddr->IpAddress.String) != 0)
                    out_addrs.push_back({ pAdapter->Description, pIPAddr->IpAddress.String, pIPAddr->IpMask.String });
                pIPAddr = pIPAddr->Next;
            }
            pAdapter = pAdapter->Next;
        }
    } else {
        LOG_CRITICAL("GetAdaptersInfo failed with error: {}", dwRetVal);
    }
#else
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;
        if ((ifa->ifa_flags & IFF_LOOPBACK) != 0)
            continue;
        if (ifa->ifa_flags)
            if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
                char netMaskAddrStr[INET_ADDRSTRLEN];
                auto netMaskAddr = ((sockaddr_in *)ifa->ifa_netmask)->sin_addr;
                inet_ntop(AF_INET, &netMaskAddr, netMaskAddrStr, INET_ADDRSTRLEN);
                // is a valid IP4 Address
                tmpAddrPtr = &((sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                out_addrs.push_back({ ifa->ifa_name, addressBuffer, netMaskAddrStr });
            }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);
#endif
    return ret_addrs();
}

AssignedAddr get_selected_assigned_addr(int32_t &outIndex) {
    const auto addrs = get_all_assigned_addrs();
    if (outIndex >= addrs.size()) {
        LOG_ERROR("Invalid index {}, returning first address", outIndex);
        outIndex = 0;
    }
    return addrs[outIndex];
}

void init_address(int32_t &outIndex, uint32_t &netAddr, uint32_t &broadcastAddr) {
    // Initialize the net and broadcast address based on the assigned address and netmask
    const auto addr = get_selected_assigned_addr(outIndex);
    int netMask;
    inet_pton(AF_INET, addr.addr.c_str(), &netAddr);
    inet_pton(AF_INET, addr.netMask.c_str(), &netMask);
    broadcastAddr = netAddr | ~netMask;
}

// Add these includes near the top with the other includes
#include <vector>

// Helper: return system DNS servers as IPv4 addresses in network byte order
static std::vector<uint32_t> get_system_dns_servers() {
    std::vector<uint32_t> out;
#ifdef _WIN32
    ULONG size = 0;
    if (GetNetworkParams(nullptr, &size) == ERROR_BUFFER_OVERFLOW) {
        FIXED_INFO *info = static_cast<FIXED_INFO *>(std::malloc(size));
        if (info) {
            if (GetNetworkParams(info, &size) == NO_ERROR) {
                IP_ADDR_STRING *p = &info->DnsServerList;
                while (p) {
                    if (p->IpAddress.String[0]) {
                        uint32_t a = inet_addr(p->IpAddress.String); // returns network byte order
                        out.push_back(a);
                    }
                    p = p->Next;
                }
            }
            std::free(info);
        }
    }
#else
    std::ifstream f("/etc/resolv.conf");
    if (!f)
        return out;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string token;
        if (!(iss >> token))
            continue;
        if (token == "nameserver") {
            std::string ip;
            if (!(iss >> ip))
                continue;
            struct in_addr addr;
            if (inet_pton(AF_INET, ip.c_str(), &addr) == 1) {
                out.push_back(addr.s_addr); // network byte order
            }
        }
    }
#endif
    return out;
}

} // namespace net_utils
