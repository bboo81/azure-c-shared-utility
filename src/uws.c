 // Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/uws.h"
#include "azure_c_shared_utility/xlogging.h"

UWS_HANDLE uws_create(const char* hostname, unsigned int port, bool use_ssl)
{
    (void)hostname;
    (void)port;
    (void)use_ssl;
    return NULL;
}

void uws_destroy(UWS_HANDLE uws)
{
    (void)uws;
}