// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UWS_FRAME_DECODER_H
#define UWS_FRAME_DECODER_H

#include "uws_frame.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif

    #include "azure_c_shared_utility/umock_c_prod.h"

    typedef struct UWS_FRAME_DECODER_TAG* UWS_FRAME_DECODER_HANDLE;

    typedef void (*ON_WS_FRAME_DECODED)(void* context, UWS_FRAME_HANDLE uws_frame);

    MOCKABLE_FUNCTION(, UWS_FRAME_DECODER_HANDLE, uws_frame_decoder_create, ON_WS_FRAME_DECODED, on_frame_decoded, void*, context);
    MOCKABLE_FUNCTION(, void, uws_frame_decoder_destroy, UWS_FRAME_DECODER_HANDLE, uws_frame_decoder);
    MOCKABLE_FUNCTION(, int, uws_frame_decoder_decode, UWS_FRAME_DECODER_HANDLE, uws_frame_decoder, const unsigned char*, bytes, size_t, size);

#ifdef __cplusplus
}
#endif

#endif /* UWS_FRAME_DECODER_H */
