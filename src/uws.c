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
#include "azure_c_shared_utility/crt_abstractions.h"

typedef struct UWS_INSTANCE_TAG
{
    SINGLYLINKEDLIST_HANDLE pending_sends;
    XIO_HANDLE underlying_io;
    char* hostname;
} UWS_INSTANCE;

UWS_HANDLE uws_create(const char* hostname, unsigned int port, bool use_ssl)
{
    UWS_HANDLE result;

    /* Codes_SRS_UWS_01_002: [ If the argument `hostname` is NULL then `uws_create` shall return NULL. ]*/
    if (hostname == NULL)
    {
        LogError("NULL hostname");
        result = NULL;
    }
    else
    {
        /* Codes_SRS_UWS_01_001: [`uws_create` shall create an instance of uws and return a non-NULL handle to it.]*/
        result = malloc(sizeof(UWS_INSTANCE));
        if (result == NULL)
        {
            /* Codes_SRS_UWS_01_003: [ If allocating memory for the new uws instance fails then `uws_create` shall return NULL. ]*/
            LogError("Could not allocate uWS instance");
        }
        else
        {
            /* Codes_SRS_UWS_01_004: [ The argument `hostname` shall be copied for later use. ]*/
            if (mallocAndStrcpy_s(&result->hostname, hostname) != 0)
            {
                /* Codes_SRS_UWS_01_392: [ If allocating memory for the copy of the hostname argument fails, then `uws_create` shall return NULL. ]*/
                LogError("Could not copy hostname.");
                free(result);
                result = NULL;
            }
            else
            {
                /* Codes_SRS_UWS_01_017: [ `uws_create` shall create a pending send IO list that is to be used to queue send packets by calling `singlylinkedlist_create`. ]*/
                result->pending_sends = singlylinkedlist_create();
                if (result->pending_sends == NULL)
                {
                    /* Codes_SRS_UWS_01_018: [ If `singlylinkedlist_create` fails then `uws_create` shall fail and return NULL. ]*/
                    LogError("Could not allocate pending send frames list");
                    free(result->hostname);
                    free(result);
                    result = NULL;
                }
                else
                {
                    if (use_ssl == true)
                    {
                        TLSIO_CONFIG tlsio_config;
                        /* Codes_SRS_UWS_01_006: [ If `use_ssl` is 1 then `uws_create` shall obtain the interface used to create a tlsio instance by calling `platform_get_default_tlsio`. ]*/
                        const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
                        if (tlsio_interface == NULL)
                        {
                            /* Codes_SRS_UWS_01_007: [ If obtaining the underlying IO interface fails, then `uws_create` shall fail and return NULL. ]*/
                            LogError("NULL TLSIO interface description");
                            result->underlying_io = NULL;
                        }
                        else
                        {
                            /* Codes_SRS_UWS_01_013: [ The create arguments for the tls IO (when `use_ssl` is 1) shall have: ]*/
                            /* Codes_SRS_UWS_01_014: [ - `hostname` set to the `hostname` argument passed to `uws_create`. ]*/
                            /* Codes_SRS_UWS_01_015: [ - `port` set to the `port` argument passed to `uws_create`. ]*/
                            tlsio_config.hostname = hostname;
                            tlsio_config.port = port;

                            result->underlying_io = xio_create(tlsio_interface, &tlsio_config);
                            if (result->underlying_io == NULL)
                            {
                                LogError("Cannot create underlying TLS IO.");
                            }
                        }
                    }
                    else
                    {
                        SOCKETIO_CONFIG socketio_config;
                        /* Codes_SRS_UWS_01_005: [ If `use_ssl` is 0 then `uws_create` shall obtain the interface used to create a socketio instance by calling `socketio_get_interface_description`. ]*/
                        const IO_INTERFACE_DESCRIPTION* socketio_interface = socketio_get_interface_description();
                        if (socketio_interface == NULL)
                        {
                            /* Codes_SRS_UWS_01_007: [ If obtaining the underlying IO interface fails, then `uws_create` shall fail and return NULL. ]*/
                            LogError("NULL socketio interface description");
                            result->underlying_io = NULL;
                        }
                        else
                        {
                            /* Codes_SRS_UWS_01_010: [ The create arguments for the socket IO (when `use_ssl` is 0) shall have: ]*/
                            /* Codes_SRS_UWS_01_011: [ - `hostname` set to the `hostname` argument passed to `uws_create`. ]*/
                            /* Codes_SRS_UWS_01_012: [ - `port` set to the `port` argument passed to `uws_create`. ]*/
                            socketio_config.hostname = hostname;
                            socketio_config.port = port;
                            socketio_config.accepted_socket = NULL;

                            /* Codes_SRS_UWS_01_008: [ The obtained interface shall be used to create the IO used as underlying IO by the newly created uws instance. ]*/
                            /* Codes_SRS_UWS_01_009: [ The underlying IO shall be created by calling `xio_create`. ]*/
                            result->underlying_io = xio_create(socketio_interface, &socketio_config);
                            if (result->underlying_io == NULL)
                            {
                                LogError("Cannot create underlying socket IO.");
                            }
                        }
                    }

                    if (result->underlying_io == NULL)
                    {
                        /* Tests_SRS_UWS_01_016: [ If `xio_create` fails, then `uws_create` shall fail and return NULL. ]*/
                        singlylinkedlist_destroy(result->pending_sends);
                        free(result->hostname);
                        free(result);
                        result = NULL;
                    }
                }
            }
        }
    }

    return result;
}

void uws_destroy(UWS_HANDLE uws)
{
    /* Codes_SRS_UWS_01_020: [ If `uws` is NULL, `uws_destroy` shall do nothing. ]*/
    if (uws == NULL)
    {
        LogError("NULL uws handle");
    }
    else
    {
        /* Codes_SRS_UWS_01_019: [ `uws_destroy` shall free all resources associated with the uws instance. ]*/
        /* Codes_SRS_UWS_01_023: [ `uws_destroy` shall destroy the underlying IO created in `uws_create` by calling `xio_destroy`. ]*/
        xio_destroy(uws->underlying_io);
        /* Codes_SRS_UWS_01_024: [ `uws_destroy` shall free the list used to track the pending sends by calling `singlylinkedlist_destroy`. ]*/
        singlylinkedlist_destroy(uws->pending_sends);
        free(uws->hostname);
        free(uws);
    }
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
}

static void on_underlying_io_error(void* context)
{
    (void)context;
}

int uws_open(UWS_HANDLE uws, ON_WS_OPEN_COMPLETE on_ws_open_complete, void* on_ws_open_complete_context, ON_WS_FRAME_RECEIVED on_ws_frame_received, void* on_ws_frame_received_context, ON_WS_ERROR on_ws_error, void* on_ws_error_context)
{
    (void)uws;
    (void)on_ws_open_complete;
    (void)on_ws_open_complete_context;
    (void)on_ws_frame_received;
    (void)on_ws_frame_received_context;
    (void)on_ws_error;
    (void)on_ws_error_context;

    /* Codes_SRS_UWS_01_025: [ `uws_open` shall open the underlying IO by calling `xio_open` and providing the IO handle created in `uws_create` as argument. ]*/
    xio_open(uws->underlying_io, on_underlying_io_open_complete, uws, on_underlying_io_bytes_received, uws, on_underlying_io_error, uws);

    return 0;
}