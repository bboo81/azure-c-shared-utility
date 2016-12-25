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
        size_t needed_bytes = 2;
        size_t header_bytes;

        /* Codes_SRS_UWS_FRAME_ENCODER_01_048: [ The buffer `encode_buffer` shall be reset by calling `BUFFER_unbuild`. ]*/
        if (BUFFER_unbuild(encode_buffer) != 0)
        {
            LogError("Cannot reset buffer size for encoded frame");
            result = __LINE__;
        }
        else
        {
            /* Codes_SRS_UWS_FRAME_ENCODER_01_001: [ `uws_frame_encoder_encode` shall encode the information given in `opcode`, `payload`, `length`, `is_masked`, `is_final` and `reserved` according to the RFC6455 into the `encode_buffer` argument.]*/

            if (length > 65535)
            {

            }
            else if (length > 125)
            {

            }

            header_bytes = needed_bytes;
            needed_bytes += length;

            /* Codes_SRS_UWS_FRAME_ENCODER_01_046: [ The buffer `encode_buffer` shall be resized accordingly using `BUFFER_resize`. ]*/
            if (BUFFER_enlarge(encode_buffer, needed_bytes) != 0)
            {
                LogError("Cannot allocate memory for encoded frame");
                result = __LINE__;
            }
            else
            {
                /* Codes_SRS_UWS_FRAME_ENCODER_01_050: [ The allocated memory shall be accessed by calling `BUFFER_u_char`. ]*/
                unsigned char* buffer = BUFFER_u_char(encode_buffer);

                buffer[0] = opcode;

                if (is_final)
                {
                    buffer[0] |= 0x80;
                }

                if (length > 65535)
                {

                }
                else if (length > 125)
                {

                }
                else
                {
                    buffer[1] = (unsigned char)length;
                }

                if (is_masked)
                {
                    buffer[1] |= 0x80;
                }

                if (length > 0)
                {
                    (void)memcpy(buffer + header_bytes, payload, length);
                }

                /* Codes_SRS_UWS_FRAME_ENCODER_01_044: [ On success `uws_frame_encoder_encode` shall return 0. ]*/
                result = 0;
            }
        }
    }

    return result;
}
