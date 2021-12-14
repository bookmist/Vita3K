// Vita3K emulator project
// Copyright (C) 2021 Vita3K team
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

#include "SceNpBasic.h"

EXPORT(int, sceNpBasicCheckCallback) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicCheckIfPlayerIsBlocked) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetBlockListEntries) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetBlockListEntryCount) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetFriendContextState) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetFriendListEntries) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetFriendListEntryCount) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetFriendOnlineStatus) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetFriendRequestEntries) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetFriendRequestEntryCount) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetGameJoiningPresence) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetGamePresenceOfFriend) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetPlaySessionLog) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetPlaySessionLogSize) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetRequestedFriendRequestEntries) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicGetRequestedFriendRequestEntryCount) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicInit) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicJoinGameAckResponseSend) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicRecordPlaySessionLog) {
    return UNIMPLEMENTED();
}

#define SCE_NP_BASIC_FRIENDLIST_SIZE_MAX 100
#define SCE_NP_BASIC_BLOCKLIST_SIZE_MAX 50

#define SCE_NP_BASIC_TOKEN_ISSUE_INTERVAL (60 * 1000 * 1000) /* 60sec */
#define SCE_NP_BASIC_PRESENCE_MAX_NUM_TOKEN 6

/*  Maximum size of title */
#define SCE_NP_BASIC_TITLE_SIZE_MAX 128

#define SCE_NP_BASIC_IN_GAME_PRESENCE_STATUS_SIZE_MAX 192
#define SCE_NP_BASIC_IN_GAME_PRESENCE_DATA_SIZE_MAX 128

/*  Maximum size of InGame data messages */
#define SCE_NP_BASIC_IN_GAME_MESSAGE_SIZE_MAX 512

/*  Maximum size of Play Session Log description */
#define SCE_NP_BASIC_PLAY_SESSION_DESCRIPTION_SIZE_MAX 512

/*  The time for a Join Game Acknowledgment request to timeout */
#define SCE_NP_BASIC_JOIN_GAME_REQUEST_TIMEOUT_SEC 30 /* 30 sec */

/*  Maximum size of Join Game Acknowledgment deny reason messages */
#define SCE_NP_BASIC_JOIN_GAME_ACK_RESPONSE_DENY_REASON_SIZE_MAX 128

/*
 * @defgroup ErrorCodeList Error code list
 * @brief
 * Error codes returned by the NP Basic library
 * @{
 */
/*  Base error code of the NP Basic library */
#define SCE_NP_BASIC_ERROR_BASE 0x80551d00
/*  Unknown error */
#define SCE_NP_BASIC_ERROR_UNKNOWN 0x80551d01
/*  Invalid argument */
#define SCE_NP_BASIC_ERROR_INVALID_ARGUMENT 0x80551d02
/*  There is not enough memory */
#define SCE_NP_BASIC_ERROR_OUT_OF_MEMORY 0x80551d03
/*  Not initialized */
#define SCE_NP_BASIC_ERROR_NOT_INITIALIZED 0x80551d04
/*  Already initialized */
#define SCE_NP_BASIC_ERROR_ALREADY_INITIALIZED 0x80551d05
/*  Signed out */
#define SCE_NP_BASIC_ERROR_SIGNED_OUT 0x80551d06
/*  Not online */
#define SCE_NP_BASIC_ERROR_NOT_ONLINE 0x80551d07
/*  Searched data cannot be found */
#define SCE_NP_BASIC_ERROR_DATA_NOT_FOUND 0x80551d08
/*  Busy. Tokens have been used up */
#define SCE_NP_BASIC_ERROR_BUSY 0x80551d09
/*  Not ready for communication */
#define SCE_NP_BASIC_ERROR_NOT_READY_TO_COMMUNICATE 0x80551d0a
/*  NP Communication ID has not been supplied */
#define SCE_NP_BASIC_ERROR_NO_COMM_ID_SUPPLIED 0x80551d0b
/* Queue for communication between processes overflows. This error code returns through SceNpServiceStateCallback. */
#define SCE_NP_BASIC_ERROR_IPC_QUEUE_OVERFLOWED 0x80551d0c
/* Cache data of friend list or blocked user list has not been initialized */
#define SCE_NP_BASIC_ERROR_DATA_NOT_INITIALIZED 0x80551d0d
/*  NP Communication ID is not written in param.sfo */
#define SCE_NP_BASIC_ERROR_NO_NP_COMM_ID_IN_PARAM_SFO 0x80551d0e
/*  Response was sent on a timed out Join Game Acknowledgment request */
#define SCE_NP_BASIC_ERROR_JOIN_GAME_ACK_REQUEST_TIMEDOUT 0x80551d0f
/*  Response was sent on a canceled Join Game Acknowledgment request */
#define SCE_NP_BASIC_ERROR_JOIN_GAME_ACK_REQUEST_CANCELED 0x80551d10
/*  Response was sent on a invalid Join Game Acknowledgment request */
#define SCE_NP_BASIC_ERROR_JOIN_GAME_ACK_REQUEST_INVALID 0x80551d11
/*  Join Ggame Aacknowledgment was set without registering handler */
#define SCE_NP_BASIC_ERROR_NO_JOIN_GAME_ACK_HANDLER 0x80551d12

/** @} */

typedef SceUInt SceNpId;

typedef enum {
    SCE_NP_BASIC_FRIEND_LIST_EVENT_TYPE_SYNC_START = 0,
    SCE_NP_BASIC_FRIEND_LIST_EVENT_TYPE_SYNC = 1,
    SCE_NP_BASIC_FRIEND_LIST_EVENT_TYPE_SYNC_DONE = 2,
    SCE_NP_BASIC_FRIEND_LIST_EVENT_TYPE_ADDED = 3,
    SCE_NP_BASIC_FRIEND_LIST_EVENT_TYPE_DELETED = 4
} SceNpBasicFriendListEventType;

typedef void(SceNpBasicFriendListEventHandler)(
    SceNpBasicFriendListEventType eventType,
    Ptr<const SceNpId> friendId,
    Ptr<void> userdata);

typedef enum {
    SCE_NP_BASIC_ONLINE_STATUS_UNKNOWN = 0,
    SCE_NP_BASIC_ONLINE_STATUS_OFFLINE = 1,
    SCE_NP_BASIC_ONLINE_STATUS_AFK = 2,
    SCE_NP_BASIC_ONLINE_STATUS_ONLINE = 3
} SceNpBasicOnlineStatus;

typedef void(SceNpBasicFriendOnlineStatusEventHandler)(
    Ptr<const SceNpId> friendId,
    SceNpBasicOnlineStatus status,
    Ptr<void> userdata);

typedef enum {
    SCE_NP_BASIC_FRIEND_CONTEXT_STATE_UNKNOWN = 0,
    SCE_NP_BASIC_FRIEND_CONTEXT_STATE_OUT_OF_CONTEXT = 1,
    SCE_NP_BASIC_FRIEND_CONTEXT_STATE_IN_CONTEXT = 2
} SceNpBasicFriendContextState;

typedef void(SceNpBasicFriendContextEventHandler)(
    Ptr<const SceNpId> friendId,
    SceNpBasicFriendContextState state,
    Ptr<void> userdata);

typedef enum {
    SCE_NP_BASIC_BLOCK_LIST_EVENT_TYPE_SYNC_START = 0,
    SCE_NP_BASIC_BLOCK_LIST_EVENT_TYPE_SYNC = 1,
    SCE_NP_BASIC_BLOCK_LIST_EVENT_TYPE_SYNC_DONE = 2,
    SCE_NP_BASIC_BLOCK_LIST_EVENT_TYPE_ADDED = 3,
    SCE_NP_BASIC_BLOCK_LIST_EVENT_TYPE_DELETED = 4
} SceNpBasicBlockListEventType;

typedef void(SceNpBasicBlockListEventHandler)(
    SceNpBasicBlockListEventType eventType,
    Ptr<const SceNpId> playerId,
    Ptr<void> userdata);

typedef enum {
    SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_UNKNOWN = -1,
    SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_NONE = 0,
    SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_DEFAULT = 1,
    SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_GAME_JOINING = 2,
    SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_GAME_JOINING_ONLY_FOR_PARTY = 3,
    SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_JOIN_GAME_ACK = 4,
    SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_MAX = 5
} SceNpBasicInGamePresenceType;

typedef struct SceNpBasicInGamePresence {
    SceUInt32 sdkVersion;
    SceNpBasicInGamePresenceType type;
    char status[SCE_NP_BASIC_IN_GAME_PRESENCE_STATUS_SIZE_MAX];
    SceUInt8 data[SCE_NP_BASIC_IN_GAME_PRESENCE_DATA_SIZE_MAX];
    SceSize dataSize;
} SceNpBasicInGamePresence;

typedef struct SceNpBasicGamePresence {
    SceSize size;
    char title[SCE_NP_BASIC_TITLE_SIZE_MAX];
    SceNpBasicInGamePresence inGamePresence;
} SceNpBasicGamePresence;

typedef void(SceNpBasicFriendGamePresenceEventHandler)(
    Ptr<const SceNpId> friendId,
    Ptr<const SceNpBasicGamePresence> presence,
    Ptr<void> userdata);

/*
 * @brief InGame data message
 *
 * Represents the InGame data message sent by the game.
 */
typedef struct SceNpBasicInGameDataMessage {
    /* Arbitrary data */
    SceUInt8 data[SCE_NP_BASIC_IN_GAME_MESSAGE_SIZE_MAX];
    /* Actual data size */
    SceSize dataSize;
} SceNpBasicInGameDataMessage;

/*
 * @brief Event handler of InGame data message
 *
 * Event handler for notifying incomming InGame data messages.
 *
 * @note
 * Copy data received with this handler to a game-side memory area while this handler is being called,
 * as they cannot be accessed after leaving this handler.
 *
 * @param from     Message sender
 * @param message  Message
 * @param userdata User data at the time of handler registration
 */
typedef void(SceNpBasicInGameDataMessageEventHandler)(
    Ptr<const SceNpId> from,
    Ptr<const SceNpBasicInGameDataMessage> message,
    Ptr<void> userdata);

typedef enum {
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_SYNC_START = 0,
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_SYNC_REQUESTING = 1,
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_SYNC_REQUESTED = 2,
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_SYNC_DONE = 3,
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_REQUESTING = 4,
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_REQUESTED = 5,
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_ACCEPTED = 6,
    SCE_NP_BASIC_FRIEND_REQUEST_EVENT_TYPE_DELETED = 7
} SceNpBasicFriendRequestEventType;

typedef void(SceNpBasicFriendRequestEventHandler)(
    SceNpBasicFriendRequestEventType eventType,
    const SceNpId *player,
    void *userdata);

typedef struct SceNpBasicEventHandlers {
    SceUInt32 sdkVersion;
    Ptr<SceNpBasicFriendListEventHandler> friendListEventHandler;
    Ptr<SceNpBasicFriendOnlineStatusEventHandler> friendOnlineStatusEventHandler;
    Ptr<SceNpBasicBlockListEventHandler> blockListEventHandler;
    Ptr<SceNpBasicFriendGamePresenceEventHandler> friendGamePresenceEventHandler;
    Ptr<SceNpBasicInGameDataMessageEventHandler> inGameDataMessageEventHandler;
    Ptr<SceNpBasicFriendContextEventHandler> friendContextEventHandler;
    Ptr<SceNpBasicFriendRequestEventHandler> friendRequestEventHandler;
} SceNpBasicEventHandlers;

typedef struct SceNpCommunicationId {
    char data[9];
    char term;
    SceUChar8 num;
    char dummy;
} SceNpCommunicationId;

EXPORT(int, sceNpBasicRegisterHandler, const SceNpBasicEventHandlers *handlers, const SceNpCommunicationId *context, void *userdata) {

    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicRegisterInGameDataMessageHandler) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicRegisterJoinGameAckHandler) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicSendInGameDataMessage) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicSetInGamePresence) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicTerm) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicUnregisterHandler) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicUnregisterInGameDataMessageHandler) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicUnregisterJoinGameAckHandler) {
    return UNIMPLEMENTED();
}

EXPORT(int, sceNpBasicUnsetInGamePresence) {
    return UNIMPLEMENTED();
}

BRIDGE_IMPL(sceNpBasicCheckCallback)
BRIDGE_IMPL(sceNpBasicCheckIfPlayerIsBlocked)
BRIDGE_IMPL(sceNpBasicGetBlockListEntries)
BRIDGE_IMPL(sceNpBasicGetBlockListEntryCount)
BRIDGE_IMPL(sceNpBasicGetFriendContextState)
BRIDGE_IMPL(sceNpBasicGetFriendListEntries)
BRIDGE_IMPL(sceNpBasicGetFriendListEntryCount)
BRIDGE_IMPL(sceNpBasicGetFriendOnlineStatus)
BRIDGE_IMPL(sceNpBasicGetFriendRequestEntries)
BRIDGE_IMPL(sceNpBasicGetFriendRequestEntryCount)
BRIDGE_IMPL(sceNpBasicGetGameJoiningPresence)
BRIDGE_IMPL(sceNpBasicGetGamePresenceOfFriend)
BRIDGE_IMPL(sceNpBasicGetPlaySessionLog)
BRIDGE_IMPL(sceNpBasicGetPlaySessionLogSize)
BRIDGE_IMPL(sceNpBasicGetRequestedFriendRequestEntries)
BRIDGE_IMPL(sceNpBasicGetRequestedFriendRequestEntryCount)
BRIDGE_IMPL(sceNpBasicInit)
BRIDGE_IMPL(sceNpBasicJoinGameAckResponseSend)
BRIDGE_IMPL(sceNpBasicRecordPlaySessionLog)
BRIDGE_IMPL(sceNpBasicRegisterHandler)
BRIDGE_IMPL(sceNpBasicRegisterInGameDataMessageHandler)
BRIDGE_IMPL(sceNpBasicRegisterJoinGameAckHandler)
BRIDGE_IMPL(sceNpBasicSendInGameDataMessage)
BRIDGE_IMPL(sceNpBasicSetInGamePresence)
BRIDGE_IMPL(sceNpBasicTerm)
BRIDGE_IMPL(sceNpBasicUnregisterHandler)
BRIDGE_IMPL(sceNpBasicUnregisterInGameDataMessageHandler)
BRIDGE_IMPL(sceNpBasicUnregisterJoinGameAckHandler)
BRIDGE_IMPL(sceNpBasicUnsetInGamePresence)
