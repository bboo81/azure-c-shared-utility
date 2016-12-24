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

#include "azure_c_shared_utility/buffer_.h"

extern int uws_frame_encoder_encode(BUFFER_HANDLE encode_buffer, unsigned char opcode, const void* payload, size_t length, bool is_masked, bool is_final, unsigned char reserved);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UWS_FRAME_ENCODER_H */
