// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UWS_H
#define UWS_H

#ifdef __cplusplus
#include <cstdbool>
#include <cstddef>
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#endif

typedef struct UWS_INSTANCE_TAG* UWS_HANDLE;

#define WS_SEND_FRAME_RESULT_VALUES \
    WS_SEND_FRAME_OK, \
    WS_SEND_FRAME_ERROR, \
    WS_SEND_FRAME_CANCELLED

DEFINE_ENUM(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT_VALUES);

#define WS_OPEN_RESULT_VALUES \
    WS_OPEN_OK, \
    WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED, \
    WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED, \
    WS_OPEN_ERROR_NOT_ENOUGH_MEMORY, \
    WS_OPEN_ERROR_CANNOT_CONSTRUCT_UPGRADE_REQUEST, \
    WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST, \
    WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS, \
    WS_OPEN_ERROR_CONSTRUCTING_UPGRADE_REQUEST, \
    WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS, \
    WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN, \
    WS_OPEN_CANCELLED

DEFINE_ENUM(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);

#define WS_ERROR_VALUES \
    WS_ERROR_NOT_ENOUGH_MEMORY, \
    WS_ERROR_BAD_FRAME_RECEIVED

DEFINE_ENUM(WS_ERROR, WS_ERROR_VALUES);

#define WS_FRAME_TYPE_TEXT      0x01
#define WS_FRAME_TYPE_BINARY    0x02

typedef void(*ON_WS_FRAME_RECEIVED)(void* context, unsigned char frame_type, const unsigned char* buffer, size_t size);
typedef void(*ON_WS_SEND_FRAME_COMPLETE)(void* context, WS_SEND_FRAME_RESULT ws_send_frame_result);
typedef void(*ON_WS_OPEN_COMPLETE)(void* context, WS_OPEN_RESULT ws_open_result);
typedef void(*ON_WS_CLOSE_COMPLETE)(void* context);
typedef void(*ON_WS_ERROR)(void* context, WS_ERROR error_code);

typedef struct WS_PROTOCOL_TAG
{
    const char* protocol;
} WS_PROTOCOL;

extern UWS_HANDLE uws_create(const char* hostname, unsigned int port, const char* resource_name, bool use_ssl, const WS_PROTOCOL* protocols, size_t protocol_count);
extern void uws_destroy(UWS_HANDLE uws);
extern int uws_open(UWS_HANDLE uws, ON_WS_OPEN_COMPLETE on_ws_open_complete, void* on_ws_open_complete_context, ON_WS_FRAME_RECEIVED on_ws_frame_received, void* on_ws_frame_received_context, ON_WS_ERROR on_ws_error, void* on_ws_error_context);
extern int uws_close(UWS_HANDLE uws, ON_WS_CLOSE_COMPLETE on_ws_close_complete, void* on_ws_close_complete_context);
extern int uws_send_frame(UWS_HANDLE uws, const unsigned char* buffer, size_t size, bool is_final, ON_WS_SEND_FRAME_COMPLETE on_ws_send_frame_complete, void* callback_context);
extern void uws_dowork(UWS_HANDLE uws);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UWS_H */
