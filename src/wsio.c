 // Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/wsio.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/uws.h"

typedef enum IO_STATE_TAG
{
    IO_STATE_NOT_OPEN,
    IO_STATE_OPENING,
    IO_STATE_OPEN,
    IO_STATE_CLOSING,
    IO_STATE_ERROR
} IO_STATE;

typedef struct PENDING_SOCKET_IO_TAG
{
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_SOCKET_IO;

typedef struct WSIO_INSTANCE_TAG
{
    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;
    ON_IO_ERROR on_io_error;
    void* on_io_error_context;
    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
    char* hostname;
    char* proxy_address;
    int proxy_port;
    UWS_HANDLE uws;
} WSIO_INSTANCE;

static void indicate_error(WSIO_INSTANCE* wsio_instance)
{
    wsio_instance->io_state = IO_STATE_ERROR;
    if (wsio_instance->on_io_error != NULL)
    {
        wsio_instance->on_io_error(wsio_instance->on_io_error_context);
    }
}

static void indicate_open_complete(WSIO_INSTANCE* ws_io_instance, IO_OPEN_RESULT open_result)
{
    if (ws_io_instance->on_io_open_complete != NULL)
    {
        ws_io_instance->on_io_open_complete(ws_io_instance->on_io_open_complete_context, open_result);
    }
}

static LIST_ITEM_HANDLE add_pending_io(WSIO_INSTANCE* ws_io_instance, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    LIST_ITEM_HANDLE result;
    PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)malloc(sizeof(PENDING_SOCKET_IO));
    if (pending_socket_io == NULL)
    {
        result = NULL;
    }
    else
    {
        pending_socket_io->on_send_complete = on_send_complete;
        pending_socket_io->callback_context = callback_context;
        pending_socket_io->pending_io_list = ws_io_instance->pending_io_list;

        if ((result = singlylinkedlist_add(ws_io_instance->pending_io_list, pending_socket_io)) == NULL)
        {
            free(pending_socket_io);
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

static int remove_pending_io(WSIO_INSTANCE* wsio_instance, LIST_ITEM_HANDLE item_handle, PENDING_SOCKET_IO* pending_socket_io)
{
    int result;

    free(pending_socket_io);
    if (singlylinkedlist_remove(wsio_instance->pending_io_list, item_handle) != 0)
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

static void on_underlying_ws_send_frame_complete(void* context, WS_SEND_FRAME_RESULT ws_send_frame_result)
{
    (void)context;
    (void)ws_send_frame_result;
}

CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters)
{
    WSIO_CONFIG* ws_io_config = io_create_parameters;
    WSIO_INSTANCE* result;

    if ((ws_io_config == NULL) ||
        (ws_io_config->underlying_io == NULL))
    {
        result = NULL;
    }
    else
    {
        result = (WSIO_INSTANCE*)malloc(sizeof(WSIO_INSTANCE));
        if (result != NULL)
        {
            size_t hostname_length;
            static const WS_PROTOCOL protocols[] = { { "AMQPWSB10" } };

            result->on_bytes_received = NULL;
            result->on_bytes_received_context = NULL;
            result->on_io_open_complete = NULL;
            result->on_io_open_complete_context = NULL;
            result->on_io_error = NULL;
            result->on_io_error_context = NULL;
            result->uws = uws_create(ws_io_config->hostname, 443, "/$iothub/websocket", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

            hostname_length = strlen(ws_io_config->hostname);
            result->hostname = malloc(hostname_length + 1);
            if (result->hostname == NULL)
            {
                free(result);
                result = NULL;
            }
            else
            {
                (void)memcpy(result->hostname, ws_io_config->hostname, hostname_length + 1);

                result->pending_io_list = singlylinkedlist_create();
                if (result->pending_io_list == NULL)
                {
                    free(result->hostname);
                    free(result);
                    result = NULL;
                }
                else
                {
                    result->io_state = IO_STATE_NOT_OPEN;
                }
            }
        }
    }

    return result;
}

static void on_underlying_ws_open_complete(void* context, WS_OPEN_RESULT open_result)
{
    WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)context;

    (void)open_result;
    wsio_instance->io_state = IO_STATE_OPEN;
    indicate_open_complete(wsio_instance, IO_OPEN_OK);
}

static void on_underlying_ws_frame_received(void* context, unsigned char frame_type, const unsigned char* buffer, size_t size)
{
    WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)context;

    (void)frame_type;
    LogInfo("Received %zu bytes", size);

    wsio_instance->on_bytes_received(wsio_instance->on_bytes_received_context, buffer, size);
}

static void on_underlying_ws_error(void* context, WS_ERROR ws_error)
{
    (void)context;
    (void)ws_error;
}

int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result = 0;

    if (ws_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)ws_io;

        if (wsio_instance->io_state != IO_STATE_NOT_OPEN)
        {
            result = __LINE__;
        }
        else
        {
            wsio_instance->on_bytes_received = on_bytes_received;
            wsio_instance->on_bytes_received_context = on_bytes_received_context;
            wsio_instance->on_io_open_complete = on_io_open_complete;
            wsio_instance->on_io_open_complete_context = on_io_open_complete_context;
            wsio_instance->on_io_error = on_io_error;
            wsio_instance->on_io_error_context = on_io_error_context;

            wsio_instance->io_state = IO_STATE_OPENING;

            /* connect here */
            if (uws_open(wsio_instance->uws, on_underlying_ws_open_complete, wsio_instance, on_underlying_ws_frame_received, wsio_instance, on_underlying_ws_error, wsio_instance) != 0)
            {
                /* Error */
                wsio_instance->io_state = IO_STATE_NOT_OPEN;
                result = __LINE__;
            }
            else
            {
                result = 0;
            }
        }
    }
    
    return result;
}

int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
    int result = 0;

    if (ws_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)ws_io;

        if (wsio_instance->io_state == IO_STATE_NOT_OPEN)
        {
            result = __LINE__;
        }
        else
        {
            if (wsio_instance->io_state == IO_STATE_OPENING)
            {
                indicate_open_complete(wsio_instance, IO_OPEN_CANCELLED);
            }
            else
            {
                /* cancel all pending IOs */
                LIST_ITEM_HANDLE first_pending_io;

                while ((first_pending_io = singlylinkedlist_get_head_item(wsio_instance->pending_io_list)) != NULL)
                {
                    PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);

                    if (pending_socket_io != NULL)
                    {
                        if (pending_socket_io->on_send_complete != NULL)
                        {
                            pending_socket_io->on_send_complete(pending_socket_io->callback_context, IO_SEND_CANCELLED);
                        }

                        if (pending_socket_io != NULL)
                        {
                            free(pending_socket_io);
                        }
                    }

                    (void)singlylinkedlist_remove(wsio_instance->pending_io_list, first_pending_io);
                }
            }

            uws_close(wsio_instance->uws, NULL, NULL);
            wsio_instance->io_state = IO_STATE_NOT_OPEN;

            if (on_io_close_complete != NULL)
            {
                on_io_close_complete(on_io_close_complete_context);
            }

            result = 0;
        }
    }

    return result;
}

void wsio_destroy(CONCRETE_IO_HANDLE ws_io)
{
    if (ws_io != NULL)
    {
        WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)ws_io;

        (void)wsio_close(wsio_instance, NULL, NULL);

        singlylinkedlist_destroy(wsio_instance->pending_io_list);

        if (wsio_instance->hostname != NULL)
        {
            free(wsio_instance->hostname);
        }

        free(ws_io);
    }
}

int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if ((ws_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        result = __LINE__;
    }
    else
    {
        WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)ws_io;

        if (wsio_instance->io_state != IO_STATE_OPEN)
        {
            result = __LINE__;
        }
        else
        {
            LIST_ITEM_HANDLE new_item;
            if ((new_item = add_pending_io(wsio_instance, on_send_complete, callback_context)) != 0)
            {
                result = __LINE__;
            }
            else
            {
                if (uws_send_frame(wsio_instance->uws, buffer, size, true, on_underlying_ws_send_frame_complete, new_item) != 0)
                {
                    indicate_error(wsio_instance);
                }
                else
                {
                }

                result = 0;
            }
        }
    }

    return result;
}

void wsio_dowork(CONCRETE_IO_HANDLE ws_io)
{
    if (ws_io != NULL)
    {
        WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)ws_io;

        if ((wsio_instance->io_state == IO_STATE_OPEN) ||
            (wsio_instance->io_state == IO_STATE_OPENING))
        {
            uws_dowork(wsio_instance->uws);
        }
    }
}

int wsio_setoption(CONCRETE_IO_HANDLE ws_io, const char* optionName, const void* value)
{
    int result;
    if (
        (ws_io == NULL) ||
        (optionName == NULL)
        )
    {
        result = __LINE__;
        LogError("invalid parameter (NULL) passed to HTTPAPI_SetOption");
    }
    else
    {
        WSIO_INSTANCE* wsio_instance = (WSIO_INSTANCE*)ws_io;
        
        if (uws_set_option(wsio_instance->uws, optionName, value) != 0)
        {
            result = __LINE__;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

void* wsio_clone_option(const char* name, const void* value)
{
    void* result;
    if (
        (name == NULL) || (value == NULL)
       )
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
        result = NULL;
    }
    else
    {
        result = NULL;
    }

    return result;
}

/*this function destroys an option previously created*/
void wsio_destroy_option(const char* name, const void* value)
{
    if (
        (name == NULL) || (value == NULL)
       )
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
    }
}

OPTIONHANDLER_HANDLE wsio_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result;
    if (handle == NULL)
    {
        LogError("parameter handle is NULL");
        result = NULL;
    }
    else
    {
        result = OptionHandler_Create(wsio_clone_option, wsio_destroy_option, wsio_setoption);
        if (result == NULL)
        {
            LogError("unable to OptionHandler_Create");
            /*return as is*/
        }
        else
        {
		}
    }

    return result;
}

static const IO_INTERFACE_DESCRIPTION ws_io_interface_description =
{
    wsio_retrieveoptions,
    wsio_create,
    wsio_destroy,
    wsio_open,
    wsio_close,
    wsio_send,
    wsio_dowork,
    wsio_setoption
};

const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void)
{
    return &ws_io_interface_description;
}
