// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UWS_FRAME_ENCODER_H
#define UWS_FRAME_ENCODER_H

#ifdef __cplusplus
#include <cstdbool>
extern "C" {
#else
#include <stdbool.h>
#endif

extern int uws_frame_encoder_encode(BUFFER_HANDLE buffer, unsigned char opcode, const void* payload, size_t length, bool masked, bool final, unsigned char reserved);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UWS_FRAME_ENCODER_H */
