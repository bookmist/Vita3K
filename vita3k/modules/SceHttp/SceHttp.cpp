// Vita3K emulator project
// Copyright (C) 2022 Vita3K team
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

#include "SceHttp.h"

#ifdef WIN32 // windows moment
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define write(x, y, z) _write(x, y, z)
#define read(x, y, z) _read(x, y, z)
#else
#include <netdb.h>
#include <sys/socket.h>
#endif

#include <http/state.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <util/log.h>
#include <util/net_utils.h>
#include <util/tracy.h>

#include <string>
#include <thread>

TRACY_MODULE_NAME(SceHttp);

template <>
std::string to_debug_str<SceHttpAddHeaderMode>(const MemState &mem, SceHttpAddHeaderMode type) {
    switch (type) {
    case SCE_HTTP_HEADER_OVERWRITE:
        return "SCE_HTTP_HEADER_OVERWRITE";
    case SCE_HTTP_HEADER_ADD:
        return "SCE_HTTP_HEADER_ADD";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceHttpVersion>(const MemState &mem, SceHttpVersion type) {
    switch (type) {
    case SCE_HTTP_VERSION_1_0:
        return "SCE_HTTP_VERSION_1_0";
    case SCE_HTTP_VERSION_1_1:
        return "SCE_HTTP_VERSION_1_1";
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceHttpMethods>(const MemState &mem, SceHttpMethods type) {
    switch (type) {
    case SCE_HTTP_METHOD_GET:
        return "SCE_HTTP_METHOD_GET";
    case SCE_HTTP_METHOD_POST:
        return "SCE_HTTP_METHOD_POST";
    case SCE_HTTP_METHOD_HEAD:
        return "SCE_HTTP_METHOD_HEAD";
    case SCE_HTTP_METHOD_OPTIONS:
        return "SCE_HTTP_METHOD_OPTIONS";
    case SCE_HTTP_METHOD_PUT:
        return "SCE_HTTP_METHOD_PUT";
    case SCE_HTTP_METHOD_DELETE:
        return "SCE_HTTP_METHOD_DELETE";
    case SCE_HTTP_METHOD_TRACE:
        return "SCE_HTTP_METHOD_TRACE";
    case SCE_HTTP_METHOD_CONNECT:
        return "SCE_HTTP_METHOD_CONNECT";
    case SCE_HTTP_METHOD_INVALID:
        break; // Invalid should fallback to raw decimal value
    }
    return std::to_string(type);
}

template <>
std::string to_debug_str<SceHttpsFlags>(const MemState &mem, SceHttpsFlags type) {
    std::string out;

    if (type & SCE_HTTPS_FLAG_SERVER_VERIFY)
        out += "SCE_HTTPS_FLAG_SERVER_VERIFY ";
    if (type & SCE_HTTPS_FLAG_CLIENT_VERIFY)
        out += "SCE_HTTPS_FLAG_CLIENT_VERIFY ";
    if (type & SCE_HTTPS_FLAG_CN_CHECK)
        out += "SCE_HTTPS_FLAG_CN_CHECK ";
    if (type & SCE_HTTPS_FLAG_NOT_AFTER_CHECK)
        out += "SCE_HTTPS_FLAG_NOT_AFTER_CHECK ";
    if (type & SCE_HTTPS_FLAG_NOT_BEFORE_CHECK)
        out += "SCE_HTTPS_FLAG_NOT_BEFORE_CHECK ";
    if (type & SCE_HTTPS_FLAG_KNOWN_CA_CHECK)
        out += "SCE_HTTPS_FLAG_KNOWN_CA_CHECK";

    if (out.empty())
        return std::to_string(type);

    return out;
}

EXPORT(int, sceHttpAbortRequest) {
    TRACY_FUNC(sceHttpAbortRequest);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpAbortRequestForce) {
    TRACY_FUNC(sceHttpAbortRequestForce);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpAbortWaitRequest) {
    TRACY_FUNC(sceHttpAbortWaitRequest);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpAddCookie) {
    TRACY_FUNC(sceHttpAddCookie);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpAddRequestHeader, SceInt reqId, const char *name, const char *value, SceHttpAddHeaderMode mode) {
    TRACY_FUNC(sceHttpAddRequestHeader, reqId, name, value, mode);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (!name || !value)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto &req = emuenv.http.requests.find(reqId)->second;

    if (mode == SCE_HTTP_HEADER_OVERWRITE) {
        if (req.headers.find(std::string(name)) != req.headers.end()) {
            // Entry already exists
            req.headers.find(name)->second = std::string(value);
        } else {
            // entry doesn't exists, we can insert it
            req.headers.insert({ name, value });
        }
    } else if (mode == SCE_HTTP_HEADER_ADD) {
        if (req.headers.find(name) != req.headers.end())
            return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

        req.headers.insert({ name, value });
    } else
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    return 0;
}

EXPORT(int, sceHttpAddRequestHeaderRaw) {
    TRACY_FUNC(sceHttpAddRequestHeaderRaw);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpAuthCacheFlush) {
    TRACY_FUNC(sceHttpAuthCacheFlush);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpCookieExport) {
    TRACY_FUNC(sceHttpCookieExport);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpCookieFlush) {
    TRACY_FUNC(sceHttpCookieFlush);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpCookieImport) {
    TRACY_FUNC(sceHttpCookieImport);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpCreateConnectionWithURL, SceInt tmplId, const char *url, SceBool enableKeepalive) {
    TRACY_FUNC(sceHttpCreateConnectionWithURL, tmplId, url, enableKeepalive);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.templates.find(tmplId) == emuenv.http.templates.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (!url)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    auto urlStr = std::string(url);

    auto tmpl = emuenv.http.templates.find(tmplId);

    int connId = emuenv.http.next_conn;
    emuenv.http.next_conn++;

    net_utils::parsedUrl parsed;
    auto parseRet = net_utils::parse_url(url, parsed);
    if (parseRet != 0) {
        switch (parseRet) {
        case SCE_HTTP_ERROR_UNKNOWN_SCHEME: {
            LOG_WARN("SCHEME IS: {}", parsed.scheme);
            return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_SCHEME);
        }
        case SCE_HTTP_ERROR_OUT_OF_SIZE: return RET_ERROR(SCE_HTTP_ERROR_OUT_OF_SIZE);
        default:
            LOG_WARN("Returning missing case of parse_url {}", parseRet);
            assert(false);
            return parseRet;
        }
    }

    if (parsed.invalid)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    bool isSecure = urlStr[4] == 's';
    // Check if scheme is https
    // https://
    // 012345
    //     ^Check this one

    std::string port; // 65535\0
    // if URL doesn't have port, use protocol default
    if (parsed.port.empty())
        port = isSecure ? "443" : "80";
    else
        port = parsed.port;
    // If fifth character is an s (meaning https) use 443, else 80

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd);
    if (sockfd < 0) {
        LOG_ERROR("ERROR opening socket");
        return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN);
    }

    if (!emuenv.cfg.http_enable) {
        // Need to push the connection here so the id exists when "sending" the request
        emuenv.http.connections.emplace(connId, SceConnection{ tmplId, urlStr, enableKeepalive, isSecure, sockfd });
        return 0;
    }

    const addrinfo hints = {
        AI_PASSIVE, /* For wildcard IP address */
        AF_UNSPEC, /* Allow IPv4 or IPv6 */
        SOCK_DGRAM, /* Datagram socket */
        0, /* Any protocol */
    };
    addrinfo *result = { 0 };

    auto ret = getaddrinfo(parsed.hostname.c_str(), port.c_str(), &hints, &result);
    if (ret < 0) {
        LOG_ERROR("getaddrinfo({},{},...) = {}", url, port, ret);
        return RET_ERROR(SCE_HTTP_ERROR_RESOLVER_ENODNS);
    }

    ret = connect(sockfd, result->ai_addr, result->ai_addrlen);
    if (ret < 0) {
        LOG_ERROR("connect({},...) = {}, errno={}({})", sockfd, ret, errno, strerror(errno));
        return RET_ERROR(SCE_HTTP_ERROR_RESOLVER_ENOHOST);
    }

    LOG_TRACE("Connected to {}", url);

    if (isSecure) {
        if (!emuenv.http.sslInited) {
            LOG_ERROR("SSL not inited on secure connection");
            return RET_ERROR(SCE_HTTP_ERROR_SSL);
        }

        SSL_set_fd((SSL *)tmpl->second.ssl, sockfd);

        int err = SSL_connect((SSL *)tmpl->second.ssl);
        if (err != 1) {
            int sslErr = SSL_get_error((SSL *)tmpl->second.ssl, err);
            LOG_ERROR("SSL_connect(...) = {}, SSLERR = {}", err, sslErr);
            if (sslErr == SSL_ERROR_SSL)
                return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN);
        }

        long verify_flag = SSL_get_verify_result((SSL *)tmpl->second.ssl);
        if (verify_flag != X509_V_OK && verify_flag != X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
            LOG_ERROR("Certificate verification error ({}) but continuing...\n", (int)verify_flag);
    }

    emuenv.http.connections.emplace(connId, SceConnection{ tmplId, urlStr, enableKeepalive, isSecure, sockfd });

    return connId;
}

EXPORT(SceInt, sceHttpCreateConnection, SceInt tmplId, const char *hostname, const char *scheme, SceUShort16 port, SceBool enableKeepalive) {
    TRACY_FUNC(sceHttpCreateConnection, tmplId, hostname, scheme, port, enableKeepalive);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.templates.find(tmplId) == emuenv.http.templates.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (!scheme)
        return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_SCHEME);

    if (strcmp(scheme, "http") != 0 || strcmp(scheme, "https") != 0) {
        LOG_WARN("SCHEME IS: {}", scheme);
        return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_SCHEME);
    }

    if (!hostname)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    if (port == 0)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    std::string url = std::string(scheme) + "://" + std::string(hostname) + ":" + std::to_string(port);

    return CALL_EXPORT(sceHttpCreateConnectionWithURL, tmplId, url.c_str(), enableKeepalive);
}

EXPORT(int, sceHttpCreateEpoll) {
    TRACY_FUNC(sceHttpCreateEpoll);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpCreateRequestWithURL, SceInt connId, SceHttpMethods method, const char *url, SceULong64 contentLength) {
    TRACY_FUNC(sceHttpCreateRequestWithURL, connId, method, url, contentLength);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.connections.find(connId) == emuenv.http.connections.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (method >= SCE_HTTP_METHOD_INVALID || method < 0)
        return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_METHOD);

    if (!url)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    auto urlStr = std::string(url);

    auto conn = emuenv.http.connections.find(connId);
    auto tmpl = emuenv.http.templates.find(conn->second.tmplId);

    // Check if the URL of the request and connection are the same
    if (conn->second.url != urlStr)
        LOG_WARN("URL != Connection URL");

    int reqId = emuenv.http.next_req;
    emuenv.http.next_req++;

    std::string httpVer = tmpl->second.httpVersion == SCE_HTTP_VERSION_1_0 ? "HTTP/1.0" : "HTTP/1.1";

    net_utils::parsedUrl parsed;
    auto parseRet = net_utils::parse_url(urlStr, parsed);
    if (parseRet != 0) {
        switch (parseRet) {
        case SCE_HTTP_ERROR_UNKNOWN_SCHEME: {
            LOG_WARN("SCHEME IS: {}", parsed.scheme);
            return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_SCHEME);
        }
        case SCE_HTTP_ERROR_OUT_OF_SIZE: return RET_ERROR(SCE_HTTP_ERROR_OUT_OF_SIZE);
        default:
            LOG_WARN("Returning missing case of parse_url {}", parseRet);
            assert(false);
            return parseRet;
        }
    }

    if (parsed.invalid)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    std::string slash = "/";
    if (parsed.path.empty())
        parsed.path = slash;
    if (parsed.query.empty())
        parsed.query = "";
    if (parsed.fragment.empty())
        parsed.fragment = "";
    std::string resourcePath = parsed.path + parsed.query + parsed.fragment;

    SceRequest req;
    req.connId = connId;
    req.method = method;

    req.url = urlStr;
    req.contentLength = contentLength;

    req.headers.insert({ "User-Agent", tmpl->second.userAgent });
    if (tmpl->second.httpVersion == SCE_HTTP_VERSION_1_1)
        req.headers.insert({ "Host", parsed.hostname });

    if (tmpl->second.httpVersion == SCE_HTTP_VERSION_1_1 && conn->second.keepAlive)
        req.headers.insert({ "Connection", "Keep-Alive" });

    std::string methodStr;
    switch (method) {
    case SCE_HTTP_METHOD_GET:
        methodStr = "GET";
        break;
    case SCE_HTTP_METHOD_POST:
        methodStr = "POST";
        break;
    case SCE_HTTP_METHOD_HEAD:
        methodStr = "HEAD";
        break;
    case SCE_HTTP_METHOD_OPTIONS:
        methodStr = "OPTIONS";
        break;
    case SCE_HTTP_METHOD_PUT:
        methodStr = "PUT";
        break;
    case SCE_HTTP_METHOD_DELETE:
        methodStr = "DELETE";
        break;
    case SCE_HTTP_METHOD_TRACE:
        methodStr = "TRACE";
        break;
    case SCE_HTTP_METHOD_CONNECT:
        methodStr = "CONNECT";
        break;
    default:
        return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_METHOD);
    }

    req.requestLine = methodStr + " " + resourcePath + " " + httpVer;

    emuenv.http.requests.emplace(reqId, req);

    return reqId;
}

EXPORT(SceInt, sceHttpCreateRequest, SceInt connId, SceHttpMethods method, const char *path, SceULong64 contentLength) {
    TRACY_FUNC(sceHttpCreateRequest, connId, method, path, contentLength);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.connections.find(connId) == emuenv.http.connections.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (method >= SCE_HTTP_METHOD_INVALID || method < 0)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    if (!path)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    auto conn = emuenv.http.connections.find(connId);

    return CALL_EXPORT(sceHttpCreateRequestWithURL, connId, method, conn->second.url.c_str(), contentLength);
}

EXPORT(SceInt, sceHttpCreateRequest2, SceInt connId, const char *method, const char *path, SceULong64 contentLength) {
    TRACY_FUNC(sceHttpCreateRequest2, connId, method, path, contentLength);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.connections.find(connId) == emuenv.http.connections.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (!path)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    auto conn = emuenv.http.connections.find(connId);

    auto intMethod = (SceHttpMethods)net_utils::char_method_to_int(method);
    // Even if it returns error (-1), it will get handled in the call

    return CALL_EXPORT(sceHttpCreateRequestWithURL, connId, intMethod, conn->second.url.c_str(), contentLength);
}

EXPORT(SceInt, sceHttpCreateRequestWithURL2, SceInt connId, const char *method, const char *path, SceULong64 contentLength) {
    TRACY_FUNC(sceHttpCreateRequestWithURL2, connId, method, path, contentLength);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.connections.find(connId) == emuenv.http.connections.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (!path)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    auto intMethod = (SceHttpMethods)net_utils::char_method_to_int(method);
    // Even if it returns error (-1), it will get handled in the call

    return CALL_EXPORT(sceHttpCreateRequestWithURL, connId, intMethod, path, contentLength);
}

EXPORT(SceInt, sceHttpCreateTemplate, const char *userAgent, SceHttpVersion httpVer, SceBool autoProxyConf) {
    TRACY_FUNC(sceHttpCreateTemplate, userAgent, httpVer, autoProxyConf);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!userAgent)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    if (httpVer != SceHttpVersion::SCE_HTTP_VERSION_1_0 && httpVer != SceHttpVersion::SCE_HTTP_VERSION_1_1)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VERSION);

    SceInt tmplId = emuenv.http.next_temp;
    emuenv.http.next_temp++;

    void *ssl_ctx = nullptr;
    if (emuenv.http.sslInited)
        ssl_ctx = emuenv.http.ssl_ctx;
    else
        ssl_ctx = SSL_CTX_new(SSLv23_client_method());

    SSL_set_mode((SSL *)ssl_ctx, SSL_MODE_AUTO_RETRY);

    auto ssl = SSL_new((SSL_CTX *)ssl_ctx);

    emuenv.http.templates.emplace(tmplId, SceTemplate{ std::string(userAgent), httpVer, autoProxyConf, ssl });

    return tmplId;
}

EXPORT(SceInt, sceHttpDeleteConnection, SceInt connId) {
    TRACY_FUNC(sceHttpDeleteConnection, connId);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.connections.find(connId) == emuenv.http.connections.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto connIt = emuenv.http.connections.find(connId);

    close(connIt->second.sockfd);

    emuenv.http.connections.erase(connIt);

    return 0;
}

EXPORT(SceInt, sceHttpDeleteRequest, SceInt reqId) {
    TRACY_FUNC(sceHttpDeleteRequest, reqId);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto it = emuenv.http.requests.find(reqId);
    if (it->second.res.responseRaw)
        delete[] it->second.res.responseRaw;
    for (auto &pointer : it->second.guestPointers) {
        free(emuenv.mem, pointer.address());
    }
    emuenv.http.requests.erase(it);

    return 0;
}

EXPORT(SceInt, sceHttpDeleteTemplate, SceInt tmplId) {
    TRACY_FUNC(sceHttpDeleteTemplate, tmplId);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.templates.find(tmplId) == emuenv.http.templates.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto it = emuenv.http.templates.find(tmplId);

    SSL_free((SSL *)it->second.ssl);

    emuenv.http.templates.erase(it);

    return 0;
}

EXPORT(int, sceHttpDestroyEpoll) {
    TRACY_FUNC(sceHttpDestroyEpoll);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetAcceptEncodingGZIPEnabled) {
    TRACY_FUNC(sceHttpGetAcceptEncodingGZIPEnabled);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpGetAllResponseHeaders, SceInt reqId, Ptr<char> *header, SceSize *headerSize) {
    TRACY_FUNC(sceHttpGetAllResponseHeaders, reqId, header, headerSize);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto req = emuenv.http.requests.find(reqId);

    auto headers = net_utils::constructHeaders(req->second.res.headers);

    // is alloc name ok?
    auto h = Ptr<char>(alloc(emuenv.mem, sizeof(char), "header")); // Allocate on guest mem
    memcpy(h.get(emuenv.mem), headers.data(), headers.length() + 1); // Put header data on guest mem
    req->second.guestPointers.push_back(h); // Save the pointer to free it later
    *header = h; // make header point to the guest address where headers are located

    *headerSize = headers.length() + 1;

    return 0;
}

EXPORT(int, sceHttpGetAuthEnabled) {
    TRACY_FUNC(sceHttpGetAuthEnabled);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetAutoRedirect) {
    TRACY_FUNC(sceHttpGetAutoRedirect);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetCookie) {
    TRACY_FUNC(sceHttpGetCookie);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetCookieEnabled) {
    TRACY_FUNC(sceHttpGetCookieEnabled);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetCookieStats) {
    TRACY_FUNC(sceHttpGetCookieStats);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetEpoll) {
    TRACY_FUNC(sceHttpGetEpoll);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetEpollId) {
    TRACY_FUNC(sceHttpGetEpollId);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetIcmOption) {
    TRACY_FUNC(sceHttpGetIcmOption);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpGetLastErrno, SceInt reqId, SceInt *errNum) {
    TRACY_FUNC(sceHttpGetLastErrno, reqId, errNum);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    *errNum = (int)errno;

    return 0;
}

EXPORT(SceInt, sceHttpGetMemoryPoolStats, SceHttpMemoryPoolStats *currentStat) {
    TRACY_FUNC(sceHttpGetMemoryPoolStats, currentStat);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!currentStat)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpGetNonblock) {
    TRACY_FUNC(sceHttpGetNonblock);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpGetResponseContentLength, SceInt reqId, SceULong64 *contentLength) {
    TRACY_FUNC(sceHttpGetResponseContentLength, reqId, contentLength);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto req = emuenv.http.requests.find(reqId);

    auto length_it = req->second.res.headers.find("Content-Length");
    if (length_it == req->second.res.headers.end())
        return RET_ERROR(SCE_HTTP_ERROR_NO_CONTENT_LENGTH);

    SceULong64 length = std::stoi(length_it->second);
    *contentLength = length;

    req->second.res.contentLength = length;

    return 0;
}

EXPORT(SceInt, sceHttpGetStatusCode, SceInt reqId, SceInt *statusCode) {
    TRACY_FUNC(sceHttpGetStatusCode, reqId, statusCode);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto req = emuenv.http.requests.find(reqId);

    *statusCode = req->second.res.statusCode;
    return 0;
}

EXPORT(SceInt, sceHttpInit, SceSize poolSize) {
    TRACY_FUNC(sceHttpInit, poolSize);
    if (emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_ALREADY_INITED);

    if (poolSize == 0)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    STUBBED("ignore poolSize");

    if (emuenv.http.sslInited)
        emuenv.http.ssl_ctx = SSL_CTX_new(SSLv23_client_method());

    emuenv.http.inited = true;

    return 0;
}

EXPORT(SceInt, sceHttpParseResponseHeader, Ptr<const char> headers, SceSize headersLen, const char *fieldStr, Ptr<char> *fieldValue, SceSize *valueLen) {
    TRACY_FUNC(sceHttpParseResponseHeader, headers, headersLen, fieldStr, fieldValue, valueLen);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!headers.valid(emuenv.mem) || headersLen == 0)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    *valueLen = 0; // reset to 0 to check after

    std::string headerStr = std::string(headers.get(emuenv.mem));
    char *ptr;
    ptr = strtok(headerStr.data(), "\r\n");
    // use while loop to check ptr is not null
    while (ptr != NULL) {
        auto line = std::string(ptr);
        auto name = line.substr(0, line.find(":"));
        auto value = line.substr(line.find(" ") + 1);

        if (strcmp(name.c_str(), fieldStr) == 0) { // found header

            // is alloc name ok?
            auto h = Ptr<char>(alloc(emuenv.mem, sizeof(char), "fieldValue")); // Allocate on guest mem
            memcpy(h.get(emuenv.mem), value.data(), value.length() + 1); // Put header data on guest mem
            emuenv.http.guestPointers.push_back(h); // Save the pointer to free it later
            *fieldValue = h; // make header point to the guest address where headers are located

            *valueLen = value.length() + 1;
            break;
        }

        ptr = strtok(NULL, "\r\n");
    }

    if (*valueLen == 0) {
        LOG_TRACE("Asked Header doesn't exists");
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);
    }

    return 0;
}

EXPORT(SceInt, sceHttpParseStatusLine, const char *statusLine, SceSize lineLen, SceInt *httpMajorVer, SceInt *httpMinorVer, SceInt *responseCode, Ptr<char> *reasonPhrase, SceSize *phraseLen) {
    TRACY_FUNC(sceHttpParseStatusLine, statusLine, lineLen, httpMajorVer, httpMinorVer, responseCode, reasonPhrase, phraseLen);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!statusLine)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    STUBBED("Ignore lineLen");

    // TODO: test

    auto line = std::string(statusLine);

    auto cleanLine = line.substr(0, line.find("\r\n"));
    // even if there is no \r\n, the result will still be the whole string

    auto httpString = cleanLine.substr(0, cleanLine.find(" "));
    auto version = httpString.substr(httpString.find("/") + 1);
    auto majorVer = version.substr(0, 1);
    *httpMajorVer = std::stoi(majorVer);
    if (version.find(".") != std::string::npos) {
        auto minorVer = version.substr(version.find(".") + 1);
        *httpMinorVer = std::stoi(minorVer);
    } else {
        *httpMinorVer = 0;
    }

    auto statusCodeLine = cleanLine.substr(cleanLine.find(" ") + 1, 3);
    *responseCode = std::stoi(statusCodeLine);

    auto codeAndPhrase = cleanLine.substr(cleanLine.find(" ") + 1); // 200 OK

    auto reason = codeAndPhrase.substr(codeAndPhrase.find(" ") + 1); // OK

    auto h = Ptr<char>(alloc(emuenv.mem, sizeof(char), "reasonPhrase"));
    memcpy(h.get(emuenv.mem), reason.data(), reason.length() + 1);
    emuenv.http.guestPointers.push_back(h);
    *reasonPhrase = h;

    *phraseLen = reason.length() + 1;

    return 0;
}

EXPORT(SceInt, sceHttpReadData, SceInt reqId, void *data, SceSize size) {
    TRACY_FUNC(sceHttpReadData, reqId, data, size);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto req = emuenv.http.requests.find(reqId);

    // These methods have no body
    if (req->second.method == SCE_HTTP_METHOD_HEAD || req->second.method == SCE_HTTP_METHOD_OPTIONS)
        return 0;

    auto conn = emuenv.http.connections.find(req->second.connId);
    auto tmpl = emuenv.http.templates.find(conn->second.tmplId);

    SceSize read = 0;

    // If the game wants to read more than whats available, change the read ammount to what is available
    if (size > (req->second.res.contentLength - req->second.res.responseRead)) {
        size = req->second.res.contentLength - req->second.res.responseRead;
    }

    if (req->second.res.responseRead == req->second.res.contentLength) {
        // If we already have read all the response.
        return 0;
    }

    memcpy(data, req->second.res.body + req->second.res.responseRead, size);

    req->second.res.responseRead += read;

    return read;
}

EXPORT(int, sceHttpRedirectCacheFlush) {
    TRACY_FUNC(sceHttpRedirectCacheFlush);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpRemoveRequestHeader, SceInt reqId, const char *name) {
    TRACY_FUNC(sceHttpRemoveRequestHeader, reqId, name);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto req = emuenv.http.requests.find(reqId);

    if (req->second.headers.find(name) != req->second.headers.end())
        req->second.headers.erase(name);

    return 0;
}

EXPORT(SceInt, sceHttpRequestGetAllHeaders, SceInt reqId, Ptr<char> *header, SceSize *headerSize) {
    TRACY_FUNC(sceHttpRequestGetAllHeaders, reqId, header, headerSize);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto req = emuenv.http.requests.find(reqId);

    auto headers = net_utils::constructHeaders(req->second.headers);

    auto h = Ptr<char>(alloc(emuenv.mem, sizeof(char), "headers"));
    memcpy(h.get(emuenv.mem), headers.data(), headers.length() + 1);
    req->second.guestPointers.push_back(h);
    *header = h;

    *headerSize = headers.length() + 1;

    return 0;
}

EXPORT(SceInt, sceHttpSendRequest, SceInt reqId, const char *postData, SceSize size) {
    TRACY_FUNC(sceHttpSendRequest, reqId, postData, size);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (!emuenv.cfg.http_enable)
        return 0;

    auto req = emuenv.http.requests.find(reqId);
    auto conn = emuenv.http.connections.find(req->second.connId);
    auto tmpl = emuenv.http.templates.find(conn->second.tmplId);

    LOG_DEBUG("Sending {} request to {}", net_utils::int_method_to_char(req->second.method), req->second.url);

    // TODO: Also support file scheme, doesn't really require any connections, not sure how it handles headers and such

    /* TODO:
        TRACE
        CONNECT
     */
    int bytes, sent, received, total;
    switch (req->second.method) {
    case SCE_HTTP_METHOD_GET:
    case SCE_HTTP_METHOD_HEAD:
    case SCE_HTTP_METHOD_OPTIONS:
    case SCE_HTTP_METHOD_DELETE: {
        auto headers = net_utils::constructHeaders(req->second.headers);

        req->second.message = req->second.requestLine + "\r\n" + headers + "\r\n";

        total = req->second.message.length();
        sent = 0;
        do {
            if (conn->second.isSecure)
                bytes = SSL_write((SSL *)tmpl->second.ssl, req->second.message.c_str() + sent, total - sent);
            else
                bytes = write(conn->second.sockfd, req->second.message.c_str() + sent, total - sent);

            if (bytes < 0) {
                LOG_ERROR("ERROR writing GET message to socket");
                assert(false);
                return RET_ERROR(SCE_HTTP_ERROR_NETWORK);
            }
            LOG_TRACE("Sent {} bytes to {}", bytes, req->second.url);
            if (bytes == 0)
                break;
            sent += bytes;
        } while (sent < total);

        // Make sockets non blocking only for the response
        // as we can run out of data to read so it blocks like a whole minute
        if (!net_utils::socketSetBlocking(conn->second.sockfd, false)) {
            LOG_WARN("Failed to change blocking, socket={}, blocking={}", conn->second.sockfd, false);
            assert(false);
        }

        /* receive the response */
        int attempts = 1;
        auto reqResponse = new char[emuenv.http.defaultResponseHeaderSize]();
        total = emuenv.http.defaultResponseHeaderSize - 1;
        received = 0;
        do {
            if (conn->second.isSecure)
                bytes = SSL_read((SSL *)tmpl->second.ssl, reqResponse + received, total - received);
            else
                bytes = read(conn->second.sockfd, reqResponse + received, total - received);
            if (bytes < 0) {
                if (bytes == -1) {
                    if (errno == EWOULDBLOCK) {
                        if (attempts > emuenv.cfg.http_read_end_attempts)
                            break; // we can assume there is no more data to read
                        LOG_TRACE("No data available. Sleep for {}. Attempt {}", emuenv.cfg.http_read_end_sleep_ms, attempts);
                        std::this_thread::sleep_for(std::chrono::milliseconds(emuenv.cfg.http_read_end_sleep_ms));
                        attempts++;
                        continue;
                    } else {
                        LOG_ERROR("ERROR reading GET response");
                        assert(false);
                        delete[] reqResponse;
                        return RET_ERROR(SCE_HTTP_ERROR_NETWORK);
                    }
                }
            }
            LOG_TRACE("Received {} bytes from {}", bytes, req->second.url);
            if (bytes == 0) {
                if (strcmp(reqResponse, "") == 0) {
                    if (attempts > emuenv.cfg.http_timeout_attempts)
                        break; // Give up
                    LOG_TRACE("Response is null. Sleep for {}. Attempt {}", emuenv.cfg.http_timeout_sleep_ms, attempts);
                    std::this_thread::sleep_for(std::chrono::milliseconds(emuenv.cfg.http_timeout_sleep_ms));
                    attempts++;
                    continue;
                }
                break; // we have  2 line ends, meaning that its the end of the headers
            }

            received += bytes;
        } while (received < total);

        LOG_TRACE("Finished reading data");

        if (!net_utils::socketSetBlocking(conn->second.sockfd, true)) {
            LOG_WARN("Failed to change blocking, socket={}, blocking={}", conn->second.sockfd, true);
            assert(false);
        }
        /*
         * if the number of received bytes is the total size of the
         * array then we have run out of space to store the response
         * and it hasn't all arrived yet - so that's a BAD thing
         */
        if (received == total) {
            LOG_ERROR("ERROR storing complete GET response");
            assert(false);
            delete[] reqResponse;
            return RET_ERROR(SCE_HTTP_ERROR_TOO_LARGE_RESPONSE_HEADER);
        }

        if (strcmp(reqResponse, "") == 0) {
            LOG_ERROR("Received empty GET response. Probably due to unknown protocol");
            assert(false);
            delete[] reqResponse;
            return RET_ERROR(SCE_HTTP_ERROR_BAD_RESPONSE);
        }

        // we need to separate the body and the headers
        std::string reqResponseHeaders;
        std::string reqResponseStr = std::string(reqResponse);
        auto headerEndPos = reqResponseStr.find("\r\n\r\n");
        char *ptr;
        ptr = strtok(reqResponseStr.data(), "\r\n");
        // use while loop to check ptr is not null
        while (ptr != nullptr) {
            reqResponseHeaders.append(ptr);
            reqResponseHeaders.append("\r\n");

            auto headerLen = reqResponseHeaders.length();
            // -2 because its the length of the remaining \r\n on the last header
            if (headerLen - 2 == headerEndPos)
                break;

            ptr = strtok(NULL, "\r\n");
        };

        req->second.res.responseRaw = reqResponse;
        req->second.res.body = reqResponse + reqResponseHeaders.length() + 2;

        // partialResponse is now the contents of the headers, we should parse them
        net_utils::parseResponse(reqResponseHeaders, req->second.res);

        break;
    }
    // PUT and POST are equal
    case SCE_HTTP_METHOD_PUT:
    case SCE_HTTP_METHOD_POST: {
        // If there is a length header, use that header as length
        // else
        // size and predefined length are equal?
        // if they are then we ok, use that
        // else use the predefined one

        // Priority: Header > Predefined > size

        if (req->second.headers.find("Content-Length") != req->second.headers.end()) {
            // There is a content length header, probably by the game, use it
            auto contHeader = req->second.headers.find("Content-Length");
            SceSize contLen = std::stoi(contHeader->second);

            // Its ok to have the content length be less or equal than size,
            // but not the other way around. It would be sending undefined data leading to undefined behavior
            if (contLen > size)
                LOG_WARN("POST/PUT request Header: ContentLength > size.");

            // Set size to contLen to not send extra stuff the server will ignore
            size = contLen;
        } else {
            // No Content-Length header

            // if size and predefined aren't equal, we will use predefined
            if (req->second.contentLength != size) {
                LOG_WARN("POST/PUT request Header: predefined != size.");
                size = req->second.contentLength;
            }

            auto contLen = std::to_string(size);
            auto ret = CALL_EXPORT(sceHttpAddRequestHeader, reqId, "Content-Length", contLen.c_str(), SCE_HTTP_HEADER_ADD);
            if (ret < 0) {
                LOG_WARN("huh?");
                assert(false);
            }
        }

        auto headers = net_utils::constructHeaders(req->second.headers);

        req->second.message = req->second.requestLine + "\r\n" + headers + "\r\n";

        total = req->second.message.length();
        sent = 0;
        do {
            if (conn->second.isSecure)
                bytes = SSL_write((SSL *)tmpl->second.ssl, req->second.message.c_str() + sent, total - sent);
            else
                bytes = write(conn->second.sockfd, req->second.message.c_str() + sent, total - sent);

            if (bytes < 0) {
                LOG_ERROR("ERROR writing POST message to socket");
                assert(false);
                return RET_ERROR(SCE_HTTP_ERROR_NETWORK);
            }
            LOG_TRACE("Sent {} bytes to {}", bytes, req->second.url);
            if (bytes == 0)
                break;
            sent += bytes;
        } while (sent < total);

        //  Once we send the request we need to send the actual data
        auto dataSent = 0;
        auto dataBytes = 0;
        do {
            if (conn->second.isSecure)
                dataBytes = SSL_write((SSL *)tmpl->second.ssl, postData + dataSent, size - dataSent);
            else
                dataBytes = write(conn->second.sockfd, postData + dataSent, size - dataSent);

            if (dataBytes < 0) {
                LOG_ERROR("ERROR writing POST data to socket");
                assert(false);
                return RET_ERROR(SCE_HTTP_ERROR_NETWORK);
            }
            LOG_TRACE("Sent {} data bytes to {}", dataBytes, req->second.url);
            if (dataBytes == 0)
                break;
            dataSent += dataBytes;
        } while (dataSent < size);

        if (!net_utils::socketSetBlocking(conn->second.sockfd, false)) {
            LOG_WARN("Failed to change blocking, socket={}, blocking={}", conn->second.sockfd, false);
            assert(false);
        }
        /* receive the response */
        int attempts = 1;
        auto reqResponse = new char[emuenv.http.defaultResponseHeaderSize]();
        total = emuenv.http.defaultResponseHeaderSize - 1;
        received = 0;
        do {
            if (conn->second.isSecure)
                bytes = SSL_read((SSL *)tmpl->second.ssl, reqResponse + received, total - received);
            else
                bytes = read(conn->second.sockfd, reqResponse + received, total - received);
            if (bytes < 0) {
                if (bytes == -1) {
                    if (errno == EWOULDBLOCK) {
                        if (attempts > emuenv.cfg.http_read_end_attempts)
                            break; // we can assume there is no more data to read
                        LOG_TRACE("No data available. Sleep for {}. Attempt {}", emuenv.cfg.http_read_end_sleep_ms, attempts);
                        std::this_thread::sleep_for(std::chrono::milliseconds(emuenv.cfg.http_read_end_sleep_ms));
                        attempts++;
                        continue;
                    } else {
                        LOG_ERROR("ERROR reading POST response");
                        assert(false);
                        delete[] reqResponse;
                        return RET_ERROR(SCE_HTTP_ERROR_NETWORK);
                    }
                }
            }
            LOG_TRACE("Received {} bytes from {}", bytes, req->second.url);
            if (bytes == 0) {
                if (strcmp(reqResponse, "") == 0) {
                    if (attempts > emuenv.cfg.http_timeout_attempts)
                        break; // Give up
                    LOG_TRACE("Response is null. Sleep for {}. Attempt {}", emuenv.cfg.http_timeout_sleep_ms, attempts);
                    std::this_thread::sleep_for(std::chrono::milliseconds(emuenv.cfg.http_timeout_sleep_ms));
                    attempts++;
                    continue;
                }
                break; // we have  2 line ends, meaning that its the end of the headers
            }
            received += bytes;
        } while (received < total);

        LOG_TRACE("Finished reading data");

        if (!net_utils::socketSetBlocking(conn->second.sockfd, true)) {
            LOG_WARN("Failed to change blocking, socket={}, blocking={}", conn->second.sockfd, true);
            assert(false);
        }

        /*
         * if the number of received bytes is the total size of the
         * array then we have run out of space to store the response
         * and it hasn't all arrived yet - so that's a BAD thing
         */
        if (received == total) {
            LOG_ERROR("ERROR storing complete POST response from socket");
            assert(false);
            delete[] reqResponse;
            return RET_ERROR(SCE_HTTP_ERROR_TOO_LARGE_RESPONSE_HEADER);
        }

        if (strcmp(reqResponse, "") == 0) {
            LOG_ERROR("Received empty GET response");
            assert(false);
            delete[] reqResponse;
            return RET_ERROR(SCE_HTTP_ERROR_BAD_RESPONSE);
        }

        // we need to separate the body and the headers
        std::string reqResponseHeaders;
        std::string reqResponseStr = std::string(reqResponse);
        auto headerEndPos = reqResponseStr.find("\r\n\r\n");
        char *ptr;
        ptr = strtok(reqResponseStr.data(), "\r\n");
        // use while loop to check ptr is not null
        while (ptr != NULL) {
            reqResponseHeaders.append(ptr);
            reqResponseHeaders.append("\r\n");

            auto headerLen = reqResponseHeaders.length();
            // -2 because its the length of the remaining \r\n on the last header
            if (headerLen - 2 == headerEndPos)
                break;

            ptr = strtok(NULL, "\r\n");
        };

        req->second.res.responseRaw = reqResponse;
        req->second.res.body = reqResponse + reqResponseHeaders.length() + 2;

        // partialResponse is now the contents of the headers, we should parse them
        net_utils::parseResponse(reqResponseHeaders, req->second.res);

        break;
    }
    default: {
        if (req->second.method < 0 || req->second.method >= SCE_HTTP_METHOD_INVALID) { // Outside any known method
            LOG_ERROR("Invalid method {}", req->second.method);
            return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_METHOD);
        } else { // its a known method but its not implemented
            LOG_WARN("Unimplemented method {}, report to devs", req->second.method);
        }
    }
    }

    return 0;
}

EXPORT(int, sceHttpSetAcceptEncodingGZIPEnabled) {
    TRACY_FUNC(sceHttpSetAcceptEncodingGZIPEnabled);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetAuthEnabled) {
    TRACY_FUNC(sceHttpSetAuthEnabled);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetAuthInfoCallback) {
    TRACY_FUNC(sceHttpSetAuthInfoCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetAutoRedirect) {
    TRACY_FUNC(sceHttpSetAutoRedirect);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetConnectTimeOut) {
    TRACY_FUNC(sceHttpSetConnectTimeOut);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetCookieEnabled) {
    TRACY_FUNC(sceHttpSetCookieEnabled);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetCookieMaxNum) {
    TRACY_FUNC(sceHttpSetCookieMaxNum);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetCookieMaxNumPerDomain) {
    TRACY_FUNC(sceHttpSetCookieMaxNumPerDomain);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetCookieMaxSize) {
    TRACY_FUNC(sceHttpSetCookieMaxSize);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetCookieRecvCallback) {
    TRACY_FUNC(sceHttpSetCookieRecvCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetCookieSendCallback) {
    TRACY_FUNC(sceHttpSetCookieSendCallback);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetCookieTotalMaxSize) {
    TRACY_FUNC(sceHttpSetCookieTotalMaxSize);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetDefaultAcceptEncodingGZIPEnabled) {
    TRACY_FUNC(sceHttpSetDefaultAcceptEncodingGZIPEnabled);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetEpoll) {
    TRACY_FUNC(sceHttpSetEpoll);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetEpollId) {
    TRACY_FUNC(sceHttpSetEpollId);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetIcmOption) {
    TRACY_FUNC(sceHttpSetIcmOption);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetInflateGZIPEnabled) {
    TRACY_FUNC(sceHttpSetInflateGZIPEnabled);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetNonblock) {
    TRACY_FUNC(sceHttpSetNonblock);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetRecvTimeOut) {
    TRACY_FUNC(sceHttpSetRecvTimeOut);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetRedirectCallback) {
    TRACY_FUNC(sceHttpSetRedirectCallback);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpSetRequestContentLength, SceInt reqId, SceULong64 contentLength) {
    TRACY_FUNC(sceHttpSetRequestContentLength, reqId, contentLength);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto req = emuenv.http.requests.find(reqId);

    req->second.contentLength = contentLength;

    return 0;
}

EXPORT(int, sceHttpSetResolveRetry) {
    TRACY_FUNC(sceHttpSetResolveRetry);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSetResolveTimeOut) {
    TRACY_FUNC(sceHttpSetResolveTimeOut);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpSetResponseHeaderMaxSize, SceInt reqId, SceSize headerSize) {
    TRACY_FUNC(sceHttpSetResponseHeaderMaxSize, reqId, headerSize);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.requests.find(reqId) == emuenv.http.requests.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (headerSize > SCE_HTTP_DEFAULT_RESPONSE_HEADER_MAX)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    // TODO: set this per http request
    emuenv.http.defaultResponseHeaderSize = headerSize;

    return 0;
}

EXPORT(int, sceHttpSetSendTimeOut) {
    TRACY_FUNC(sceHttpSetSendTimeOut);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpSslIsCtxCreated) {
    TRACY_FUNC(sceHttpSslIsCtxCreated);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpTerm) {
    TRACY_FUNC(sceHttpTerm);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    // clear everything

    for (auto &tmpl : emuenv.http.templates) {
        CALL_EXPORT(sceHttpDeleteTemplate, tmpl.first);
    }
    emuenv.http.templates.clear();

    for (auto &conn : emuenv.http.connections) {
        CALL_EXPORT(sceHttpDeleteConnection, conn.first);
    }
    emuenv.http.connections.clear();

    for (auto &req : emuenv.http.requests) {
        CALL_EXPORT(sceHttpDeleteRequest, req.first);
    }
    emuenv.http.requests.clear();

    for (auto &pointer : emuenv.http.guestPointers) {
        free(emuenv.mem, pointer.address());
    }

    return 0;
}

EXPORT(int, sceHttpUnsetEpoll) {
    TRACY_FUNC(sceHttpUnsetEpoll);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpUriBuild, char *out, SceSize *require, SceSize prepare, const SceHttpUriElement *srcElement, SceUInt option) {
    TRACY_FUNC(sceHttpUriBuild, out, require, prepare, srcElement, option);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!out || !srcElement)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    // TODO: need example from game

    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpUriEscape, char *out, SceSize *require, SceSize prepare, const char *in) {
    TRACY_FUNC(sceHttpUriEscape, out, require, prepare, in);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!out || !in)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpUriMerge, char *mergedUrl, const char *url, const char *relativeUrl, SceSize *require, SceSize prepare, SceUInt option) {
    TRACY_FUNC(sceHttpUriMerge, mergedUrl, url, relativeUrl, require, prepare, option);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!mergedUrl || !relativeUrl)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    if (!url)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    // TODO: need example from game

    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpUriParse, SceHttpUriElement *out, const char *srcUrl, void *pool, SceSize *require, SceSize prepare) {
    TRACY_FUNC(sceHttpUriParse, out, srcUrl, pool, require, prepare);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!srcUrl)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    if (!out)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    net_utils::parsedUrl parsed;
    auto parseRet = net_utils::parse_url(srcUrl, parsed);
    if (parseRet != 0) {
        switch (parseRet) {
        case SCE_HTTP_ERROR_UNKNOWN_SCHEME: {
            LOG_WARN("SCHEME IS: {}", parsed.scheme);
            return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_SCHEME);
        }
        case SCE_HTTP_ERROR_OUT_OF_SIZE: return RET_ERROR(SCE_HTTP_ERROR_OUT_OF_SIZE);
        default:
            LOG_WARN("Returning missing case of parse_url {}", parseRet);
            assert(false);
            return parseRet;
        }
    }

    if (parsed.invalid)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    if (parsed.port.empty())
        parsed.port = "0"; // Threat 0 as invalid, even if it isn't

    out->opaque = false;
    out->scheme = parsed.scheme.data();
    out->username = parsed.username.data();
    out->password = parsed.password.data();
    out->hostname = parsed.hostname.data();
    out->path = parsed.path.data();
    out->query = parsed.query.data();
    out->fragment = parsed.fragment.data();
    SceUShort16 port = std::stoi(parsed.port);
    out->port = port;

    return 0;
}

EXPORT(SceInt, sceHttpUriSweepPath, char *dst, const char *src, SceSize srcSize) {
    TRACY_FUNC(sceHttpUriSweepPath, dst, src, srcSize);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!dst)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    if (!src)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    if (srcSize == 0)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    net_utils::parsedUrl parsed;
    auto parseRet = net_utils::parse_url(src, parsed);
    if (parseRet != 0) {
        switch (parseRet) {
        case SCE_HTTP_ERROR_UNKNOWN_SCHEME: {
            LOG_WARN("SCHEME IS: {}", parsed.scheme);
            return RET_ERROR(SCE_HTTP_ERROR_UNKNOWN_SCHEME);
        }
        case SCE_HTTP_ERROR_OUT_OF_SIZE: return RET_ERROR(SCE_HTTP_ERROR_OUT_OF_SIZE);
        default:
            LOG_WARN("Returning missing case of parse_url {}", parseRet);
            assert(false);
            return parseRet;
        }
    }

    if (parsed.invalid)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_URL);

    std::string credentials = "";
    if (!parsed.username.empty()) { // we have credentials
        credentials.append(parsed.username);
        if (!parsed.password.empty()) {
            credentials.append(":");
            credentials.append(parsed.password);
        }
        credentials.append("@");
    }
    std::string port; // 65535
    if (!parsed.port.empty()) {
        port.append(":");
        port.append(parsed.port);
    }

    auto result = parsed.scheme + "://" + credentials + parsed.hostname + port;

    memcpy(dst, result.data(), result.length() + 1);

    return 0;
}

EXPORT(int, sceHttpUriUnescape, char *out, SceSize *require, SceSize prepare, const char *in) {
    TRACY_FUNC(sceHttpUriUnescape, out, require, prepare, in);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (!out || !in)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpWaitRequest) {
    TRACY_FUNC(sceHttpWaitRequest);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpWaitRequestCB) {
    TRACY_FUNC(sceHttpWaitRequestCB);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpsDisableOption, SceHttpsFlags sslFlags) {
    TRACY_FUNC(sceHttpsDisableOption, sslFlags);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    // TODO: Do sslFlags match the ones of openssl?
    STUBBED("Directly pass ssl flags to context");
    SSL_CTX_clear_options((SSL_CTX *)emuenv.http.ssl_ctx, sslFlags);

    return 0;
}

EXPORT(SceInt, sceHttpsDisableOption2, SceInt tmplId, SceHttpsFlags sslFlags) {
    TRACY_FUNC(sceHttpsDisableOption2, tmplId, sslFlags);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.templates.find(tmplId) == emuenv.http.templates.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto tmpl = emuenv.http.templates.find(tmplId);

    // TODO: Do sslFlags match the ones of openssl?
    STUBBED("Directly pass ssl flags to context");
    SSL_CTX *ctx = SSL_get_SSL_CTX((SSL *)tmpl->second.ssl);

    SSL_CTX_clear_options(ctx, sslFlags);

    return 0;
}

EXPORT(int, sceHttpsDisableOptionPrivate) {
    TRACY_FUNC(sceHttpsDisableOptionPrivate);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpsEnableOption2, SceInt tmplId, SceHttpsFlags sslFlags) {
    TRACY_FUNC(sceHttpsEnableOption2, tmplId, sslFlags);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.templates.find(tmplId) == emuenv.http.templates.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    auto tmpl = emuenv.http.templates.find(tmplId);

    // TODO: Do sslFlags match the ones of openssl?
    STUBBED("Directly pass ssl flags to context");
    SSL_CTX *ctx = SSL_get_SSL_CTX((SSL *)tmpl->second.ssl);

    SSL_CTX_set_options(ctx, sslFlags);

    return 0;
}

EXPORT(SceInt, sceHttpsEnableOption, SceHttpsFlags sslFlags) {
    TRACY_FUNC(sceHttpsEnableOption, sslFlags);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    // TODO: Do sslFlags match the ones of openssl?
    STUBBED("Directly pass ssl flags to context");
    SSL_CTX_set_options((SSL_CTX *)emuenv.http.ssl_ctx, sslFlags);

    return 0;
}

EXPORT(int, sceHttpsEnableOptionPrivate) {
    TRACY_FUNC(sceHttpsEnableOptionPrivate);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpsFreeCaList) {
    TRACY_FUNC(sceHttpsFreeCaList);
    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpsGetCaList) {
    TRACY_FUNC(sceHttpsGetCaList);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpsGetSslError, SceInt tmplId, SceInt *errNum, SceUInt *detail) {
    TRACY_FUNC(sceHttpsGetSslError, tmplId, errNum, detail);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.templates.find(tmplId) == emuenv.http.templates.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpsLoadCert) {
    TRACY_FUNC(sceHttpsLoadCert);
    return UNIMPLEMENTED();
}

EXPORT(SceInt, sceHttpsSetSslCallback, SceInt tmplId, SceHttpsCallback cbFunction, void *userArg) {
    TRACY_FUNC(sceHttpsSetSslCallback, tmplId, cbFunction, userArg);
    if (!emuenv.http.inited)
        return RET_ERROR(SCE_HTTP_ERROR_BEFORE_INIT);

    if (emuenv.http.templates.find(tmplId) == emuenv.http.templates.end())
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_ID);

    if (!cbFunction)
        return RET_ERROR(SCE_HTTP_ERROR_INVALID_VALUE);

    return UNIMPLEMENTED();
}

EXPORT(int, sceHttpsUnloadCert) {
    TRACY_FUNC(sceHttpsUnloadCert);
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceHttpAbortRequest)
BRIDGE_IMPL(sceHttpAbortRequestForce)
BRIDGE_IMPL(sceHttpAbortWaitRequest)
BRIDGE_IMPL(sceHttpAddCookie)
BRIDGE_IMPL(sceHttpAddRequestHeader)
BRIDGE_IMPL(sceHttpAddRequestHeaderRaw)
BRIDGE_IMPL(sceHttpAuthCacheFlush)
BRIDGE_IMPL(sceHttpCookieExport)
BRIDGE_IMPL(sceHttpCookieFlush)
BRIDGE_IMPL(sceHttpCookieImport)
BRIDGE_IMPL(sceHttpCreateConnection)
BRIDGE_IMPL(sceHttpCreateConnectionWithURL)
BRIDGE_IMPL(sceHttpCreateEpoll)
BRIDGE_IMPL(sceHttpCreateRequest)
BRIDGE_IMPL(sceHttpCreateRequest2)
BRIDGE_IMPL(sceHttpCreateRequestWithURL)
BRIDGE_IMPL(sceHttpCreateRequestWithURL2)
BRIDGE_IMPL(sceHttpCreateTemplate)
BRIDGE_IMPL(sceHttpDeleteConnection)
BRIDGE_IMPL(sceHttpDeleteRequest)
BRIDGE_IMPL(sceHttpDeleteTemplate)
BRIDGE_IMPL(sceHttpDestroyEpoll)
BRIDGE_IMPL(sceHttpGetAcceptEncodingGZIPEnabled)
BRIDGE_IMPL(sceHttpGetAllResponseHeaders)
BRIDGE_IMPL(sceHttpGetAuthEnabled)
BRIDGE_IMPL(sceHttpGetAutoRedirect)
BRIDGE_IMPL(sceHttpGetCookie)
BRIDGE_IMPL(sceHttpGetCookieEnabled)
BRIDGE_IMPL(sceHttpGetCookieStats)
BRIDGE_IMPL(sceHttpGetEpoll)
BRIDGE_IMPL(sceHttpGetEpollId)
BRIDGE_IMPL(sceHttpGetIcmOption)
BRIDGE_IMPL(sceHttpGetLastErrno)
BRIDGE_IMPL(sceHttpGetMemoryPoolStats)
BRIDGE_IMPL(sceHttpGetNonblock)
BRIDGE_IMPL(sceHttpGetResponseContentLength)
BRIDGE_IMPL(sceHttpGetStatusCode)
BRIDGE_IMPL(sceHttpInit)
BRIDGE_IMPL(sceHttpParseResponseHeader)
BRIDGE_IMPL(sceHttpParseStatusLine)
BRIDGE_IMPL(sceHttpReadData)
BRIDGE_IMPL(sceHttpRedirectCacheFlush)
BRIDGE_IMPL(sceHttpRemoveRequestHeader)
BRIDGE_IMPL(sceHttpRequestGetAllHeaders)
BRIDGE_IMPL(sceHttpSendRequest)
BRIDGE_IMPL(sceHttpSetAcceptEncodingGZIPEnabled)
BRIDGE_IMPL(sceHttpSetAuthEnabled)
BRIDGE_IMPL(sceHttpSetAuthInfoCallback)
BRIDGE_IMPL(sceHttpSetAutoRedirect)
BRIDGE_IMPL(sceHttpSetConnectTimeOut)
BRIDGE_IMPL(sceHttpSetCookieEnabled)
BRIDGE_IMPL(sceHttpSetCookieMaxNum)
BRIDGE_IMPL(sceHttpSetCookieMaxNumPerDomain)
BRIDGE_IMPL(sceHttpSetCookieMaxSize)
BRIDGE_IMPL(sceHttpSetCookieRecvCallback)
BRIDGE_IMPL(sceHttpSetCookieSendCallback)
BRIDGE_IMPL(sceHttpSetCookieTotalMaxSize)
BRIDGE_IMPL(sceHttpSetDefaultAcceptEncodingGZIPEnabled)
BRIDGE_IMPL(sceHttpSetEpoll)
BRIDGE_IMPL(sceHttpSetEpollId)
BRIDGE_IMPL(sceHttpSetIcmOption)
BRIDGE_IMPL(sceHttpSetInflateGZIPEnabled)
BRIDGE_IMPL(sceHttpSetNonblock)
BRIDGE_IMPL(sceHttpSetRecvTimeOut)
BRIDGE_IMPL(sceHttpSetRedirectCallback)
BRIDGE_IMPL(sceHttpSetRequestContentLength)
BRIDGE_IMPL(sceHttpSetResolveRetry)
BRIDGE_IMPL(sceHttpSetResolveTimeOut)
BRIDGE_IMPL(sceHttpSetResponseHeaderMaxSize)
BRIDGE_IMPL(sceHttpSetSendTimeOut)
BRIDGE_IMPL(sceHttpSslIsCtxCreated)
BRIDGE_IMPL(sceHttpTerm)
BRIDGE_IMPL(sceHttpUnsetEpoll)
BRIDGE_IMPL(sceHttpUriBuild)
BRIDGE_IMPL(sceHttpUriEscape)
BRIDGE_IMPL(sceHttpUriMerge)
BRIDGE_IMPL(sceHttpUriParse)
BRIDGE_IMPL(sceHttpUriSweepPath)
BRIDGE_IMPL(sceHttpUriUnescape)
BRIDGE_IMPL(sceHttpWaitRequest)
BRIDGE_IMPL(sceHttpWaitRequestCB)
BRIDGE_IMPL(sceHttpsDisableOption)
BRIDGE_IMPL(sceHttpsDisableOption2)
BRIDGE_IMPL(sceHttpsDisableOptionPrivate)
BRIDGE_IMPL(sceHttpsEnableOption)
BRIDGE_IMPL(sceHttpsEnableOption2)
BRIDGE_IMPL(sceHttpsEnableOptionPrivate)
BRIDGE_IMPL(sceHttpsFreeCaList)
BRIDGE_IMPL(sceHttpsGetCaList)
BRIDGE_IMPL(sceHttpsGetSslError)
BRIDGE_IMPL(sceHttpsLoadCert)
BRIDGE_IMPL(sceHttpsSetSslCallback)
BRIDGE_IMPL(sceHttpsUnloadCert)
