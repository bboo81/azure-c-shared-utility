// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UWS_H
#define UWS_H

#ifdef __cplusplus
#include <cstdbool>
extern "C" {
#else
#include <stdbool.h>
#endif

typedef struct UWS_INSTANCE_TAG* UWS_HANDLE;

#define WS_SEND_FRAME_RESULT_VALUES \
    WS_SEND_FRAME_OK, \
    WS_SEND_FRAME_ERROR, \
    WS_SEND_FRAME_CANCELLED

DEFINE_ENUM(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT_VALUES);

#define WS_OPEN_RESULT_VALUES \
    WS_OPEN_OK, \
    WS_OPEN_ERROR, \
    WS_OPEN_CANCELLED

DEFINE_ENUM(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);

typedef void(*ON_WS_FRAME_RECEIVED)(void* context, const unsigned char* buffer, size_t size);
typedef void(*ON_WS_SEND_FRAME_COMPLETE)(void* context, WS_SEND_FRAME_RESULT ws_send_frame_result);
typedef void(*ON_WS_OPEN_COMPLETE)(void* context, WS_OPEN_RESULT ws_open_result);
typedef void(*ON_WS_ERROR)(void* context);

extern UWS_HANDLE uws_create(const char* hostname, unsigned int port, bool use_ssl);
extern void uws_destroy(UWS_HANDLE uws);
extern int uws_open(UWS_HANDLE uws, ON_WS_OPEN_COMPLETE on_uws_open_complete, ON_WS_FRAME_RECEIVED on_ws_frame_received, ON_WS_ERROR on_ws_error, void* callback_context);
extern int uws_close(UWS_HANDLE uws);
extern int uws_send_frame(UWS_HANDLE uws, const unsigned char* buffer, size_t size, ON_WS_SEND_FRAME_COMPLETE on_ws_send_frame_complete, void* callback_context);
extern void uws_dowork(UWS_HANDLE uws);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UWS_H */
