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

int uws_frame_encoder_encode(BUFFER_HANDLE buffer, unsigned char opcode, const void* payload, size_t length, bool masked, bool final, unsigned char reserved)
{
}
