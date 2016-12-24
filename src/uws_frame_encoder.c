 // Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/uws_frame_encoder.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/buffer_.h"

int uws_frame_encoder_encode(BUFFER_HANDLE encode_buffer, unsigned char opcode, const void* payload, size_t length, bool is_masked, bool is_final, unsigned char reserved)
{
    int result;

    (void)opcode;
    (void)payload;
    (void)length;
    (void)is_masked;
    (void)is_final;
    (void)reserved;

    if (encode_buffer == NULL)
    {
        /* Codes_SRS_UWS_FRAME_ENCODER_01_045: [ If the argument `encode_buffer` is NULL then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
        LogError("NULL encode_buffer");
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}
