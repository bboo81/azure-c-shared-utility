 // Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/uws.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio.h"

typedef struct UWS_INSTANCE_TAG
{
    SINGLYLINKEDLIST_HANDLE pending_sends;
    XIO_HANDLE underlying_io;
} UWS_INSTANCE;

UWS_HANDLE uws_create(const char* hostname, unsigned int port, bool use_ssl)
{
    UWS_HANDLE result;

    (void)hostname;
    (void)port;
    (void)use_ssl;

    /* Codes_SRS_UWS_01_001: [`uws_create` shall create an instance of uws and return a non-NULL handle to it.]*/
    result = malloc(sizeof(UWS_INSTANCE));
    if (result == NULL)
    {
        LogError("Could not allocate uWS instance");
    }
    else
    {
        /* Codes_SRS_UWS_01_017: [ `uws_create` shall create a pending send IO list that is to be used to queue send packets by calling `singlylinkedlist_create`. ]*/
        result->pending_sends = singlylinkedlist_create();
        if (result->pending_sends == NULL)
        {
            LogError("Could not allocate pending send frames list");
            free(result);
            result = NULL;
        }
        else
        {
            if (use_ssl == true)
            {
                TLSIO_CONFIG tlsio_config;
                const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
                if (tlsio_interface == NULL)
                {
                    LogError("NULL TLSIO interface description");
                }
                else
                {
                    tlsio_config.hostname = hostname;
                    tlsio_config.port = port;

                    result->underlying_io = xio_create(tlsio_interface, &tlsio_config);
                }
            }
            else
            {
                SOCKETIO_CONFIG socketio_config;
                const IO_INTERFACE_DESCRIPTION* socketio_interface = socketio_get_interface_description();
                if (socketio_interface == NULL)
                {
                    LogError("NULL socketio interface description");
                }
                else
                {
                    socketio_config.hostname = hostname;
                    socketio_config.port = port;
                    socketio_config.accepted_socket = NULL;

                    result->underlying_io = xio_create(socketio_interface, &socketio_config);
                }
            }

            if (result->underlying_io == NULL)
            {
                LogError("Cannot create underlying IO.");
                singlylinkedlist_destroy(result->pending_sends);
                free(result);
                result = NULL;
            }
        }
    }

    return result;
}

void uws_destroy(UWS_HANDLE uws)
{
    (void)uws;
}