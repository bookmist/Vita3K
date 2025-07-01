// Vita3K emulator project
// Copyright (C) 2025 Vita3K team
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

#include <cstdio>
#include <kernel/state.h>
#include <net/state.h>
#include <net/types.h>
#include <util/lock_and_find.h>

#include <chrono>
#include <thread>
#include <util/net_utils.h>

#include <util/tracy.h>
TRACY_MODULE_NAME(SceNetSyscall);

template <>
std::string to_debug_str<SceNetEpollControlFlag>(const MemState &mem, SceNetEpollControlFlag type);
template <>
std::string to_debug_str<SceNetProtocol>(const MemState &mem, SceNetProtocol type);
template <>
std::string to_debug_str<SceNetSocketOption>(const MemState &mem, SceNetSocketOption type);
template <>
std::string to_debug_str<SceNetSocketType>(const MemState &mem, SceNetSocketType type);

#define RET_NET_ERRNO(ret)                                        \
    do {                                                          \
        int _r = (ret);                                           \
        if (_r >= 0)                                              \
            return _r;                                            \
        _r = -(_r & 0xFF);                                        \
        LOG_ERROR_ONCE("{} returned errno {}", export_name, -_r); \
        return _r;                                                \
    } while (0)

EXPORT(int, sceNetSyscallAccept, int sid, SceNetSockaddr *addr, unsigned int *addrlen) {
    TRACY_FUNC(sceNetSyscallAccept, sid, addr, addrlen);
    auto sock = lock_and_find(sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock)
        RET_ERROR(-SCE_NET_EBADF);

    int err = 0;
    auto newsock = sock->accept(addr, addrlen, err);
    if (!newsock)
        RET_NET_ERRNO(err);

    auto id = ++emuenv.net.next_id;
    emuenv.net.socks.emplace(id, newsock);
    return id;
}

EXPORT(int, sceNetSyscallBind, int sid, const SceNetSockaddr *addr, unsigned int addrlen) {
    TRACY_FUNC(sceNetSyscallBind, sid, addr, addrlen);
    auto sock = lock_and_find(sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock)
        return RET_ERROR(-SCE_NET_EBADF);

    RET_NET_ERRNO(sock->bind(addr, addrlen));
}

EXPORT(int, sceNetSyscallClose, int sid) {
    TRACY_FUNC(sceNetSyscallClose, sid);
    auto sock = lock_and_find(sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock)
        return RET_ERROR(-SCE_NET_EBADF);

    RET_NET_ERRNO(sock->close());
}

EXPORT(int, sceNetSyscallConnect, int sid, const SceNetSockaddr *addr, unsigned int addrlen) {
    TRACY_FUNC(sceNetSyscallConnect, sid, addr, addrlen);
    auto sock = lock_and_find(sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock)
        return RET_ERROR(-SCE_NET_EBADF);

    RET_NET_ERRNO(sock->connect(addr, addrlen));
}

EXPORT(int, sceNetSyscallControl, int if_index, int code, int *ptr, int len) {
    TRACY_FUNC(sceNetSyscallControl, if_index, code, *ptr, len)
    // LOG_CONSOLE(sceNetSyscallControl, if_index, code, *ptr, len)
    if (if_index == -1) {
        if (code == 2) {
            // dns_addresses
            if (len >= 8) {
                ptr[0] = 0x01020304;
                ptr[1] = 0x02030405;
            } else {
                memset(ptr, 0, len);
            }
        } else if (code == 1) {
            // statistics
            memset(ptr, 0, len);
            /**
             * uint kernel: free_min
             * uint kernel: free_size
             * uint packet1
             * uint packet2
             */
            uint8_t *p_bype = reinterpret_cast<uint8_t *>(ptr);
            for (int i = 0; i < len; i++) {
                p_bype[i] = i + 1;
            }
        } else if (code == 0x10000012) {
            // get emulation info //0x200 bytes
            // works only for specified if_index
        }
    }
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallDescriptorClose, int id) {
    TRACY_FUNC(sceNetSyscallDescriptorClose, id)
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallDescriptorCreate, const char *name, int flags) {
    TRACY_FUNC(sceNetSyscallDescriptorCreate, name, flags)
    UNIMPLEMENTED();
    auto id = ++emuenv.net.next_id;
    return id;
}

EXPORT(int, sceNetSyscallDescriptorCtl, int id, int op, int s, void *info) {
    TRACY_FUNC(sceNetSyscallDescriptorCtl, id, op, s, info)
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallDumpAbort, int id, int flags) {
    TRACY_FUNC(sceNetSyscallDumpAbort, id, flags);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallDumpClose, int id) {
    TRACY_FUNC(sceNetSyscallDumpClose, id)
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallDumpCreate, const char *name, int len, int flags) {
    TRACY_FUNC(sceNetSyscallDumpCreate, name, len, flags);
    UNIMPLEMENTED();
    auto id = ++emuenv.net.next_id;
    return id;
}

EXPORT(int, sceNetSyscallDumpRead, int id, void *buf, int len, void *pflags) {
    TRACY_FUNC(sceNetSyscallDumpRead, id, buf, len, pflags);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallEpollAbort, int eid, int flags) {
    TRACY_FUNC(sceNetSyscallEpollAbort, eid, flags);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallEpollClose, int eid) {
    TRACY_FUNC(sceNetSyscallEpollClose, eid);

    const std::lock_guard<std::mutex> lock(emuenv.kernel.mutex);
    if (emuenv.net.epolls.erase(eid) == 0) {
        return RET_ERROR(-SCE_NET_EBADF);
    }

    return 0;
}

EXPORT(int, sceNetSyscallEpollCreate, const char *name, int flags) {
    TRACY_FUNC(sceNetSyscallEpollCreate, name, flags);
    auto id = ++emuenv.net.next_epoll_id;
    auto epoll = std::make_shared<Epoll>();
    const std::lock_guard<std::mutex> lock(emuenv.kernel.mutex);
    emuenv.net.epolls.emplace(id, epoll);
    return id;
}

EXPORT(int, sceNetSyscallEpollCtl, int eid, SceNetEpollControlFlag op, int id, SceNetEpollEvent *ev) {
    TRACY_FUNC(sceNetSyscallEpollCtl, eid, op, id, ev);
    auto epoll = lock_and_find(eid, emuenv.net.epolls, emuenv.kernel.mutex);
    if (!epoll)
        return RET_ERROR(-SCE_NET_EBADF);

    switch (op) {
    case SCE_NET_EPOLL_CTL_ADD: {
        const auto sock = lock_and_find(id, emuenv.net.socks, emuenv.kernel.mutex);
        if (!sock)
            return RET_ERROR(-SCE_NET_EBADF);
        RET_NET_ERRNO(epoll->add(id, sock, ev));
    }
    case SCE_NET_EPOLL_CTL_DEL:
        RET_NET_ERRNO(epoll->del(id));
    case SCE_NET_EPOLL_CTL_MOD:
        RET_NET_ERRNO(epoll->mod(id, ev));
    default:
        RET_NET_ERRNO(SCE_NET_ERROR_EINVAL);
    }
}

struct SceNetSyscallEpollWaitParameter {
    int eid;
    Ptr<SceNetEpollEvent> events;
    int maxevents;
    Ptr<int> timeout;
    int do_callbacks;
};

EXPORT(int, sceNetSyscallEpollWait, SceNetSyscallEpollWaitParameter *param) {
    TRACY_FUNC(sceNetSyscallEpollWait, param->eid, param->events, param->maxevents, param->timeout, param->do_callbacks);
    auto epoll = lock_and_find(param->eid, emuenv.net.epolls, emuenv.kernel.mutex);
    if (!epoll) {
        return RET_ERROR(-SCE_NET_EBADF);
    }

    RET_NET_ERRNO(epoll->wait(param->events.get(emuenv.mem), param->maxevents, *param->timeout.get(emuenv.mem)));
}

struct SceIfItem {
    char name[16];
    char name2[16];
    uint32_t index;
    SceUInt stub_01;
    SceUInt ip_address;
    SceUInt stub_02;
    SceUInt broadcast;
    SceUInt mask;
    SceUInt ip_address_2;
    SceUInt stub_03;
    SceUInt broadcast_2;
    SceUInt mask_2;
    SceUInt status;
    SceUInt connect_type; // 1-ethernet,2-wlan,3-bluetooth,4-phone
    SceNetEtherAddr ether_addr;
    char stub[2];
    SceUInt mtu;
    SceUInt mtu_max;
    SceUInt tx_packet_bytes;
    SceUInt tx_packets;
    SceUInt tx_broadcast_bytes;
    SceUInt tx_broadcast_packets;
    SceUInt tx_multicast_bytes;
    SceUInt tx_multicast_packets;
    SceUInt tx_dropped;
    SceUInt tx_errors;
    SceUInt rx_packet_bytes;
    SceUInt rx_packets;
    SceUInt rx_broadcast_bytes;
    SceUInt rx_broadcast_packets;
    SceUInt rx_multicast_bytes;
    SceUInt rx_multicast_packets;
    SceUInt rx_dropped;
    SceUInt rx_errors;
    SceUInt data[16];
    SceUInt icm_status;
    SceUInt icm_1;
    SceUInt stub_10;
    SceUInt stub_11;
    SceUInt icm_2;
    SceUInt icm_proto;
    SceUInt stub_12;
    SceUInt stub_13;
    SceUInt emulation;
    SceUInt data1[15];
};
static_assert(sizeof(SceIfItem) <= 0x140, "SceIfItem size mismatch");

EXPORT(int, sceNetSyscallGetIfList, SceIfItem *list, int n) {
    TRACY_FUNC(sceNetSyscallGetIfList, list, n)
    // LOG_CONSOLE(sceNetSyscallGetIfList, list, n)
    UNIMPLEMENTED();
    if (n == 0)
        return 1;
    else {
        memset(list, 1, sizeof(SceIfItem) * n);
        strncpy(list->name, "test_interface", 16);
        strncpy(list->name2, "test_if_2", 16);
        list->index = 42;
        list->connect_type = 2;
        list->status = 0xFFFFE7;
        list->ip_address = 0x0100007f;
        list->stub_01 = 0x01010801;
        list->stub_02 = 0x01020802;
        list->stub_03 = 0x01030803;
        list->broadcast_2 = 0x01040804;
        list->mask = 0x0000FFFF;
        list->broadcast = 0x05060708;
        list->ip_address_2 = 0x0200007f;
        list->mask_2 = 0x000000FF;
        list->mtu = 1420;
        list->mtu_max = 1500;
        list->tx_packet_bytes = 10;
        list->tx_packets = 20;
        list->tx_broadcast_bytes = 30;
        list->tx_broadcast_packets = 40;
        list->tx_multicast_bytes = 50;
        list->tx_multicast_packets = 60;
        list->tx_dropped = 70;
        list->tx_errors = 80;
        list->rx_packet_bytes = 110;
        list->rx_packets = 120;
        list->rx_broadcast_bytes = 130;
        list->rx_broadcast_packets = 140;
        list->rx_multicast_bytes = 150;
        list->rx_multicast_packets = 160;
        list->rx_dropped = 170;
        list->rx_errors = 180;
        //
        list->icm_status = 1;
        list->icm_1 = 100;
        list->icm_2 = 200;
        list->icm_proto = 2;
        list->emulation = 1;

        memcpy(list->ether_addr.data, "abcdef", 6);
        for (int i = 0; i < 16; i++) {
            list->data[i] = i + 32;
        }
        for (int i = 0; i < 15; i++) {
            list->data1[i] = i + 64;
        }
        list->data1[2] = 600;
    }
    return n;
}

typedef SceUInt16 SceNetInPort_t;
typedef SceUID SceNetId;

enum SCE_NET_SOCKINFO_STATE {
    SCE_NET_SOCKINFO_STATE_UNKNOWN = 0,
    SCE_NET_SOCKINFO_STATE_CLOSED = 1,
    SCE_NET_SOCKINFO_STATE_OPENED = 2,
    SCE_NET_SOCKINFO_STATE_LISTEN = 3,
    SCE_NET_SOCKINFO_STATE_SYN_SENT = 4,
    SCE_NET_SOCKINFO_STATE_SYN_RECEIVED = 5,
    SCE_NET_SOCKINFO_STATE_ESTABLISHED = 6,
    SCE_NET_SOCKINFO_STATE_FIN_WAIT_1 = 7,
    SCE_NET_SOCKINFO_STATE_FIN_WAIT_2 = 8,
    SCE_NET_SOCKINFO_STATE_CLOSE_WAIT = 9,
    SCE_NET_SOCKINFO_STATE_CLOSING = 10,
    SCE_NET_SOCKINFO_STATE_LAST_ACK = 11,
    SCE_NET_SOCKINFO_STATE_TIME_WAIT = 12
};

enum SCE_NET_SOCKINFO_FLAGS {
    SCE_NET_SOCKINFO_F_SELF = 0x00000001,
    SCE_NET_SOCKINFO_F_KERNEL = 0x00000002,
    SCE_NET_SOCKINFO_F_OTHERS = 0x00000004,
    SCE_NET_SOCKINFO_F_RECV_WAIT = 0x00010000,
    SCE_NET_SOCKINFO_F_SEND_WAIT = 0x00020000,
    SCE_NET_SOCKINFO_F_RECV_EWAIT = 0x00040000,
    SCE_NET_SOCKINFO_F_SEND_EWAIT = 0x00080000,
    SCE_NET_SOCKINFO_F_WAKEUP_SIGNAL = 0x00100000,
    SCE_NET_SOCKINFO_F_ALL = 0x001F0007
};

struct SceNetSockInfo {
    char name[32];
    SceUID pid;
    SceNetId s;
    SceInt8 socket_type;
    SceInt8 policy;
    SceInt16 reserved16;
    int recv_queue_length;
    int send_queue_length;
    SceNetInAddr local_adr;
    SceNetInAddr remote_adr;
    SceNetInPort_t local_port;
    SceNetInPort_t remote_port;
    SceNetInPort_t local_vport;
    SceNetInPort_t remote_vport;
    SCE_NET_SOCKINFO_STATE state;
    SCE_NET_SOCKINFO_FLAGS flags;
    int reserved[8];
};

static_assert(sizeof(SceNetSockInfo) == 108, "SceNetSockInfo size mismatch");

EXPORT(int, sceNetSyscallGetSockinfo, int s, void *ptr, int n, int flags) {
    TRACY_FUNC(sceNetSyscallGetSockinfo, s, ptr, n, flags);
    // LOG_CONSOLE(sceNetSyscallGetSockinfo, s, ptr, n, flags);
    int max_count = std::min<int>(emuenv.net.socks.size(), n);
    if (ptr)
        for (int i = 0; i < max_count; i++) {
            auto sock = emuenv.net.socks[i];
            SceNetSockInfo info = {}; /*
             strncpy(info.name, sock->name.c_str(), sizeof(info.name) - 1);
             info.pid = sock->pid;
             info.s = sock->id;
             info.socket_type = sock->type;
             info.policy = sock->policy;
             info.recv_queue_length = sock->recv_queue_length;
             info.send_queue_length = sock->send_queue_length;
             info.local_adr = sock->local_addr.sin_addr.s_addr;
             info.remote_adr = sock->remote_addr.sin_addr.s_addr;
             info.local_port = ntohs(sock->local_addr.sin_port);
             info.remote_port = ntohs(sock->remote_addr.sin_port);
             info.local_vport = ntohs(sock->local_vport);
             info.remote_vport = ntohs(sock->remote_vport);
             info.state = sock->state;
             info.flags = sock->flags;
             memcpy((uint8_t *)ptr + i * sizeof(SceNetSockInfo), &info, sizeof(SceNetSockInfo));*/
        }
    return max_count;
}

EXPORT(int, sceNetSyscallGetpeername, int sid, SceNetSockaddr *name, unsigned int *namelen) {
    TRACY_FUNC(sceNetSyscallGetpeername, sid, name, namelen);
    auto sock = lock_and_find(sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    RET_NET_ERRNO(sock->get_peer_address(name, namelen));
}

EXPORT(int, sceNetSyscallGetsockname, int sid, SceNetSockaddr *name, unsigned int *namelen) {
    TRACY_FUNC(sceNetSyscallGetsockname, sid, name, namelen);
    auto sock = lock_and_find(sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    RET_NET_ERRNO(sock->get_socket_address(name, namelen));
}

struct SceNetSyscallGetsockoptParam {
    int sid;
    SceNetProtocol level;
    SceNetSocketOption optname;
    Ptr<void> optval;
    Ptr<unsigned int> optlen;
};

EXPORT(int, sceNetSyscallGetsockopt, SceNetSyscallGetsockoptParam *param) {
    TRACY_FUNC(sceNetSyscallGetsockopt, param->sid, param->level, param->optname, param->optval, param->optlen);

    auto sock = lock_and_find(param->sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    RET_NET_ERRNO(sock->get_socket_options(param->level, param->optname, param->optval.get(emuenv.mem), param->optlen.get(emuenv.mem)));
}

EXPORT(int, sceNetSyscallIcmConnect, int s, int flags) {
    TRACY_FUNC(sceNetSyscallIcmConnect, s, flags)
    // LOG_CONSOLE(sceNetSyscallIcmConnect, s, flags)
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallIoctl, int s, uint32_t com, void *data) {
    TRACY_FUNC(sceNetSyscallIoctl, s, com, data)
    // LOG_CONSOLE(sceNetSyscallIoctl, s, com, data)
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallListen, int sid, int backlog) {
    TRACY_FUNC(sceNetSyscallListen, sid, backlog);
    auto sock = lock_and_find(sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    RET_NET_ERRNO(sock->listen(backlog));
}

struct SceNetSyscallRecvfromParam {
    int sid;
    Ptr<void> buf;
    unsigned int len;
    int flags;
    Ptr<SceNetSockaddr> from;
    Ptr<unsigned int> fromlen;
};

EXPORT(int, sceNetSyscallRecvfrom, SceNetSyscallRecvfromParam *param) {
    TRACY_FUNC(sceNetSyscallRecvfrom, param->sid, param->buf, param->len, param->flags, param->from, param->fromlen);
    auto sock = lock_and_find(param->sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    RET_NET_ERRNO(sock->recv_packet(param->buf.get(emuenv.mem), param->len, param->flags, param->from.get(emuenv.mem), param->fromlen.get(emuenv.mem)));
}

typedef SceUInt32 SceNetInAddr_t;
typedef SceUShort16 SceNetInPort_t;
typedef SceUChar8 SceNetSaFamily_t;
typedef SceUInt32 SceNetSocklen_t;

typedef struct SceNetIovec {
    Ptr<void> iov_base; // Base address (pointer)
    SceSize iov_len; // Size of area (in bytes) indicated by iov_base
} SceNetIovec;

typedef struct SceNetMsghdr {
    Ptr<SceNetSockaddr> msg_name; // Pointer to address structure
    SceNetSocklen_t msg_namelen; // Size of address structure
    Ptr<SceNetIovec> msg_iov; // Pointer to scatter/gather array
    int msg_iovlen; // Number of elements in msg_iov array
    Ptr<void> msg_control; // (unsupported)
    SceNetSocklen_t msg_controllen; // (unsupported)
    int msg_flags; // (unsupported)
} SceNetMsghdr;

EXPORT(int, sceNetSyscallRecvmsg, int s, SceNetMsghdr *msg, int flags) {
    TRACY_FUNC(sceNetSyscallRecvmsg, s, msg, flags);
    // LOG_CONSOLE(sceNetSyscallRecvmsg, s, msg, flags);
    return UNIMPLEMENTED();
}

EXPORT(int, sceNetSyscallSendmsg, int s, const SceNetMsghdr *msg, int flags) {
    TRACY_FUNC(sceNetSyscallSendmsg, s, msg, flags);
    // LOG_CONSOLE(sceNetSyscallSendmsg, s, msg, flags);
    auto sock = lock_and_find(s, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    SceNetSockaddrIn to_in;
    SceNetSockaddrIn *to_in_ptr = &to_in;
    if (msg->msg_name && msg->msg_namelen) {
        std::memcpy(&to_in, msg->msg_name.get(emuenv.mem), std::min<uint32_t>(sizeof(SceNetSockaddrIn), msg->msg_namelen));
        if (to_in.sin_addr.s_addr == INADDR_BROADCAST)
            to_in.sin_addr.s_addr = emuenv.net.broadcastAddr;
    } else
        to_in_ptr = nullptr;
    if (msg->msg_iovlen == 1) {
        RET_NET_ERRNO(sock->send_packet(msg->msg_iov.get(emuenv.mem)->iov_base.get(emuenv.mem), msg->msg_iov.get(emuenv.mem)->iov_len, flags, (SceNetSockaddr *)to_in_ptr, msg->msg_namelen));
    } else {
        std::vector<char> data;
        SceSize total_data_size = 0;
        for (int i = 0; i < msg->msg_iovlen; i++) {
            total_data_size += msg->msg_iov.get(emuenv.mem)[i].iov_len;
        }
        data.resize(total_data_size);
        for (int i = 0; i < msg->msg_iovlen; i++) {
            data.insert(data.end(), msg->msg_iov.get(emuenv.mem)[i].iov_base.cast<char>().get(emuenv.mem), msg->msg_iov.get(emuenv.mem)[i].iov_base.cast<char>().get(emuenv.mem) + msg->msg_iov.get(emuenv.mem)[i].iov_len);
        }
        RET_NET_ERRNO(sock->send_packet(data.data(), total_data_size, flags, (SceNetSockaddr *)to_in_ptr, msg->msg_namelen));
    }
}

struct SceNetSyscallSendtoParam {
    int sid;
    Ptr<const void> msg;
    unsigned int len;
    int flags;
    Ptr<const SceNetSockaddr> to;
    unsigned int tolen;
};

EXPORT(int, sceNetSyscallSendto, SceNetSyscallSendtoParam *param) {
    TRACY_FUNC(sceNetSyscallSendto, param->sid, param->msg, param->len, param->flags, param->to, param->tolen);
    // LOG_CONSOLE(sceNetSyscallSendto, param->sid, param->msg, param->len, param->flags, param->to, param->tolen);
    // std::string data(param->msg.cast<const char>().get(emuenv.mem), param->len);
    // LOG_TRACE("sceNetSyscallSendto {}", data);
    auto sock = lock_and_find(param->sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }

    SceNetSockaddrIn to_in;
    SceNetSockaddrIn *to_in_ptr = &to_in;
    if (param->to && param->tolen) {
        std::memcpy(&to_in, param->to.get(emuenv.mem), std::min<uint32_t>(sizeof(SceNetSockaddrIn), param->tolen));
        if (to_in.sin_addr.s_addr == INADDR_BROADCAST)
            to_in.sin_addr.s_addr = emuenv.net.broadcastAddr;
    } else
        to_in_ptr = nullptr;
    RET_NET_ERRNO(sock->send_packet(param->msg.get(emuenv.mem), param->len, param->flags, (SceNetSockaddr *)to_in_ptr, param->tolen));
}

struct SceNetSyscallSetsockoptParam {
    int sid;
    SceNetProtocol level;
    SceNetSocketOption optname;
    Ptr<const int> optval;
    unsigned int optlen;
};

EXPORT(int, sceNetSyscallSetsockopt, SceNetSyscallSetsockoptParam *param) {
    TRACY_FUNC(sceNetSyscallSetsockopt, param->sid, param->level, param->optname, param->optval, param->optlen);
    // LOG_CONSOLE(sceNetSyscallSetsockopt, param->sid, param->level, param->optname, *param->optval.get(emuenv.mem), param->optlen);
    auto sock = lock_and_find(param->sid, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    if (param->optname == 0x40000) {
        LOG_ERROR("Unknown socket option {}", log_hex(param->optname));
        return 0;
    }
    RET_NET_ERRNO(sock->set_socket_options(param->level, param->optname, param->optval.get(emuenv.mem), param->optlen));
}

enum SceNetShutdownMethod {
    SCE_NET_SHUT_RD = 0, // Shuts down reading
    SCE_NET_SHUT_WR = 1, // Shuts down writing
    SCE_NET_SHUT_RDWR = 2 // Shuts down reading and writing
};

EXPORT(int, sceNetSyscallShutdown, int s, SceNetShutdownMethod how) {
    TRACY_FUNC(sceNetSyscallShutdown, s, how);
    // LOG_CONSOLE(sceNetSyscallShutdown, s, how);
    auto sock = lock_and_find(s, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock) {
        return RET_ERROR(-SCE_NET_EBADF);
    }
    RET_NET_ERRNO(sock->shutdown_socket(how));
}

EXPORT(int, sceNetSyscallSocket, const char *name, int domain, SceNetSocketType type, SceNetProtocol protocol) {
    TRACY_FUNC(sceNetSyscallSocket, name, domain, type, protocol);
    // LOG_CONSOLE(sceNetSyscallSocket, name, domain, type, protocol);
    SocketPtr sock = (type == SCE_NET_SOCK_DGRAM_P2P || type == SCE_NET_SOCK_STREAM_P2P) ? std::make_shared<P2PSocket>(domain, type, protocol) : std::make_shared<PosixSocket>(domain, type, protocol);
    auto id = ++emuenv.net.next_id;
    emuenv.net.socks.emplace(id, sock);
    return id;
}

EXPORT(int, sceNetSyscallSocketAbort, int s, int flags) {
    TRACY_FUNC(sceNetSyscallSocketAbort, s, flags);
    // LOG_CONSOLE(sceNetSyscallSocketAbort, s, flags);
    if ((s < 0) || (flags < 0) || (flags > (SCE_NET_SOCKET_ABORT_FLAG_RCV_PRESERVATION | SCE_NET_SOCKET_ABORT_FLAG_SND_PRESERVATION)))
        return RET_ERROR(-SCE_NET_EINVAL);

    auto sock = lock_and_find(s, emuenv.net.socks, emuenv.kernel.mutex);
    if (!sock)
        return RET_ERROR(-SCE_NET_EBADF);

    RET_NET_ERRNO(sock->abort(flags));
}

struct SceNetSyscallSysctlParam {
    Ptr<uint32_t> param_1;
    int param_1_len;
    Ptr<uint32_t> param_3;
    Ptr<uint32_t> param_4;
    int param_5;
    int param_6;
};

EXPORT(int, sceNetSyscallSysctl, SceNetSyscallSysctlParam *param) {
    TRACY_FUNC(sceNetSyscallSysctl, param->param_1, param->param_1_len, param->param_3, param->param_4, param->param_5, param->param_6)
    // LOG_CONSOLE(sceNetSyscallSysctl, param->param_1, param->param_1_len, param->param_3, param->param_4, param->param_5, param->param_6)
    for (int i = 0; i < param->param_1_len; i++) {
        LOG_TRACE("param1[{}]: {:X}", i, param->param_1.get(emuenv.mem)[i]);
    }
    if (param->param_4) {
        LOG_TRACE("param4[0]: {:X}", param->param_4.get(emuenv.mem)[0]);
        *param->param_4.get(emuenv.mem) = 64;
    }
    if (param->param_3) {
        LOG_TRACE("param3[0]: {:X}", param->param_3.get(emuenv.mem)[0]);
        auto p = param->param_3.get(emuenv.mem);
        p[0] = 0x10203040;
        p[1] = 0x10203041;
        // p[2] = 0x10203042;
        //  p[3] = 0x10203043;
    }
    // return 1;
    return UNIMPLEMENTED();
}
