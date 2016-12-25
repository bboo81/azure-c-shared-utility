 // Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdint.h>
#include <stddef.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/gb_rand.h"
#include "azure_c_shared_utility/uws_frame_encoder.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/uniqueid.h"

int uws_frame_encoder_encode(BUFFER_HANDLE encode_buffer, WS_FRAME_TYPE opcode, const void* payload, size_t length, bool is_masked, bool is_final, unsigned char reserved)
{
    int result;

    if (encode_buffer == NULL)
    {
        /* Codes_SRS_UWS_FRAME_ENCODER_01_045: [ If the argument `encode_buffer` is NULL then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
        LogError("NULL encode_buffer");
        result = __LINE__;
    }
    else if (reserved > 7)
    {
        /* Codes_SRS_UWS_FRAME_ENCODER_01_052: [ If `reserved` has any bits set except the lowest 3 then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
        LogError("Bad reserved value: 0x%02x", reserved);
        result = __LINE__;
    }
    else if (opcode > 0x0F)
    {
        /* Codes_SRS_UWS_FRAME_ENCODER_01_006: [ If an unknown opcode is received, the receiving endpoint MUST _Fail the WebSocket Connection_. ]*/
        LogError("Invalid opcode: 0x%02x", opcode);
        result = __LINE__;
    }
    else
    {
        size_t needed_bytes = 2;
        size_t header_bytes;

        /* Codes_SRS_UWS_FRAME_ENCODER_01_048: [ The buffer `encode_buffer` shall be reset by calling `BUFFER_unbuild`. ]*/
        if (BUFFER_unbuild(encode_buffer) != 0)
        {
            /* Codes_SRS_UWS_FRAME_ENCODER_01_049: [ If `BUFFER_unbuild` fails then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
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

            if (is_masked)
            {
                needed_bytes += 4;
            }

            header_bytes = needed_bytes;
            needed_bytes += length;

            /* Codes_SRS_UWS_FRAME_ENCODER_01_046: [ The buffer `encode_buffer` shall be resized accordingly using `BUFFER_enlarge`. ]*/
            if (BUFFER_enlarge(encode_buffer, needed_bytes) != 0)
            {
                /* Codes_SRS_UWS_FRAME_ENCODER_01_047: [ If `BUFFER_enlarge` fails then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
                LogError("Cannot allocate memory for encoded frame");
                result = __LINE__;
            }
            else
            {
                /* Codes_SRS_UWS_FRAME_ENCODER_01_050: [ The allocated memory shall be accessed by calling `BUFFER_u_char`. ]*/
                unsigned char* buffer = BUFFER_u_char(encode_buffer);
                if (buffer == NULL)
                {
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_051: [ If `BUFFER_u_char` fails then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
                    LogError("Cannot get encoded buffer pointer");
                    result = __LINE__;
                }
                else
                {
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_007: [ *  %x0 denotes a continuation frame ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_008: [ *  %x1 denotes a text frame ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_009: [ *  %x2 denotes a binary frame ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_011: [ *  %x8 denotes a connection close ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_012: [ *  %x9 denotes a ping ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_013: [ *  %xA denotes a pong ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
                    buffer[0] = opcode;

                    /* Codes_SRS_UWS_FRAME_ENCODER_01_002: [ Indicates that this is the final fragment in a message. ]*/
                    /* Codes_SRS_UWS_FRAME_ENCODER_01_003: [ The first fragment MAY also be the final fragment. ]*/
                    if (is_final)
                    {
                        buffer[0] |= 0x80;
                    }

                    /* Codes_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
                    buffer[0] |= reserved << 4;

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
                        /* Codes_SRS_UWS_FRAME_ENCODER_01_015: [ Defines whether the "Payload data" is masked. ]*/
                        buffer[1] |= 0x80;

                        /* Codes_SRS_UWS_FRAME_ENCODER_01_053: [ In order to obtain a 32 bit value for masking, `gb_rand` shall be used 4 times (for each byte). ]*/
                        buffer[header_bytes - 4] = (unsigned char)gb_rand();
                        buffer[header_bytes - 3] = (unsigned char)gb_rand();
                        buffer[header_bytes - 2] = (unsigned char)gb_rand();
                        buffer[header_bytes - 1] = (unsigned char)gb_rand();
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
    }

    return result;
}
