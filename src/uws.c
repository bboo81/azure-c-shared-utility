 // Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/uws.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/buffer_.h"

typedef enum UWS_STATE_TAG
{
    UWS_STATE_CLOSED,
    UWS_STATE_OPENING_UNDERLYING_IO,
    UWS_STATE_WAITING_FOR_UPGRADE_RESPONSE,
    UWS_STATE_OPEN,
    UWS_STATE_CLOSING,
    UWS_STATE_ERROR
} UWS_STATE;

typedef enum UWS_FRAME_DECODER_STATE_TAG
{
    UWS_FRAME_DECODER_STATE_HEADER_AND_LENGTH,
    UWS_FRAME_DECODER_STATE_EXTENDED_LENGTH_16,
    UWS_FRAME_DECODER_STATE_EXTENDED_LENGTH_64,
    UWS_FRAME_DECODER_STATE_PAYLOAD_BYTES
} UWS_FRAME_DECODER_STATE;

typedef struct WS_INSTANCE_PROTOCOL_TAG
{
    char* protocol;
} WS_INSTANCE_PROTOCOL;

#define OPCODE_CONTINUATION_FRAME   0x0
#define OPCODE_TEXT_FRAME           0x1
#define OPCODE_BINARY_FRAME         0x2

typedef struct UWS_INSTANCE_TAG
{
    SINGLYLINKEDLIST_HANDLE pending_sends;
    XIO_HANDLE underlying_io;
    char* hostname;
    char* resource_name;
    WS_INSTANCE_PROTOCOL* protocols;
    size_t protocol_count;
    int port;
    UWS_STATE uws_state;
    ON_WS_OPEN_COMPLETE on_ws_open_complete;
    void* on_ws_open_complete_context;
    ON_WS_FRAME_RECEIVED on_ws_frame_received;
    void* on_ws_frame_received_context;
    ON_WS_ERROR on_ws_error;
    void* on_ws_error_context;
    ON_WS_CLOSE_COMPLETE on_ws_close_complete;
    void* on_ws_close_complete_context;
    unsigned char* received_bytes;
    size_t received_bytes_count;
    UWS_FRAME_DECODER_STATE frame_decoder_state;
    BUFFER_HANDLE encode_buffer;
} UWS_INSTANCE;

UWS_HANDLE uws_create(const char* hostname, unsigned int port, const char* resource_name, bool use_ssl, const WS_PROTOCOL* protocols, size_t protocol_count)
{
    UWS_HANDLE result;

    /* Codes_SRS_UWS_01_002: [ If any of the arguments `hostname` and `resource_name` is NULL then `uws_create` shall return NULL. ]*/
    if ((hostname == NULL) ||
        (resource_name == NULL) ||
        /* Codes_SRS_UWS_01_411: [ If `protocol_count` is non zero and `protocols` is NULL then `uws_create` shall fail and return NULL. ]*/
        ((protocols == NULL) && (protocol_count > 0)))
    {
        LogError("Invalid arguments: hostname = %p, resource_name = %p, protocols = %p, protocol_count = %zu", hostname, resource_name, protocols, protocol_count);
        result = NULL;
    }
    else
    {
        /* Codes_SRS_UWS_01_412: [ If the `protocol` member of any of the items in the `protocols` argument is NULL, then `uws_create` shall fail and return NULL. ]*/
        size_t i;
        for (i = 0; i < protocol_count; i++)
        {
            if (protocols[i].protocol == NULL)
            {
                break;
            }
        }

        if (i < protocol_count)
        {
            LogError("Protocol index %zu has NULL name", i);
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
                /* Codes_SRS_UWS_01_422: [ `uws_create` shall create a buffer to be used for encoding outgoing frames by calling `BUFFER_new`. ]*/
                result->encode_buffer = BUFFER_new();
                if (result->encode_buffer == NULL)
                {
                    /* Codes_SRS_UWS_01_423: [ If `BUFFER_new` fails  then `uws_create` shall fail and return NULL. ]*/
                    LogError("Could not allocate encode buffer.");
                    free(result);
                    result = NULL;
                }
                else
                {
                    /* Codes_SRS_UWS_01_004: [ The argument `hostname` shall be copied for later use. ]*/
                    if (mallocAndStrcpy_s(&result->hostname, hostname) != 0)
                    {
                        /* Codes_SRS_UWS_01_392: [ If allocating memory for the copy of the `hostname` argument fails, then `uws_create` shall return NULL. ]*/
                        LogError("Could not copy hostname.");
                        BUFFER_delete(result->encode_buffer);
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        /* Codes_SRS_UWS_01_404: [ The argument `resource_name` shall be copied for later use. ]*/
                        if (mallocAndStrcpy_s(&result->resource_name, resource_name) != 0)
                        {
                            /* Codes_SRS_UWS_01_405: [ If allocating memory for the copy of the `resource` argument fails, then `uws_create` shall return NULL. ]*/
                            LogError("Could not copy resource.");
                            free(result->hostname);
                            BUFFER_delete(result->encode_buffer);
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
                                free(result->resource_name);
                                free(result->hostname);
                                BUFFER_delete(result->encode_buffer);
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
                                    free(result->resource_name);
                                    free(result->hostname);
                                    BUFFER_delete(result->encode_buffer);
                                    free(result);
                                    result = NULL;
                                }
                                else
                                {
                                    result->uws_state = UWS_STATE_CLOSED;
                                    /* Codes_SRS_UWS_01_403: [ The argument `port` shall be copied for later use. ]*/
                                    result->port = port;

                                    result->on_ws_open_complete = NULL;
                                    result->on_ws_open_complete_context = NULL;
                                    result->on_ws_frame_received = NULL;
                                    result->on_ws_frame_received_context = NULL;
                                    result->on_ws_error = NULL;
                                    result->on_ws_error_context = NULL;
                                    result->on_ws_close_complete = NULL;
                                    result->on_ws_close_complete_context = NULL;
                                    result->received_bytes = NULL;
                                    result->received_bytes_count = 0;

                                    result->protocol_count = protocol_count;

                                    /* Codes_SRS_UWS_01_410: [ The `protocols` argument shall be allowed to be NULL, in which case no protocol is to be specified by the client in the upgrade request. ]*/
                                    if (protocols != NULL)
                                    {
                                        result->protocols = (WS_INSTANCE_PROTOCOL*)malloc(sizeof(WS_INSTANCE_PROTOCOL) * protocol_count);
                                        if (result->protocols == NULL)
                                        {
                                            xio_destroy(result->underlying_io);
                                            singlylinkedlist_destroy(result->pending_sends);
                                            free(result->resource_name);
                                            free(result->hostname);
                                            BUFFER_delete(result->encode_buffer);
                                            free(result);
                                            result = NULL;
                                        }
                                        else
                                        {
                                            for (i = 0; i < protocol_count; i++)
                                            {
                                                if (mallocAndStrcpy_s(&result->protocols[i].protocol, protocols[i].protocol) != 0)
                                                {
                                                    break;
                                                }
                                            }

                                            if (i < protocol_count)
                                            {
                                                size_t j;

                                                for (j = 0; j < i; j++)
                                                {
                                                    free(result->protocols[j].protocol);
                                                }
                                                free(result->protocols);
                                                xio_destroy(result->underlying_io);
                                                singlylinkedlist_destroy(result->pending_sends);
                                                free(result->resource_name);
                                                free(result->hostname);
                                                BUFFER_delete(result->encode_buffer);
                                                free(result);
                                                result = NULL;
                                            }
                                            else
                                            {
                                                result->protocol_count = protocol_count;
                                            }
                                        }
                                    }
                                }
                            }
                        }
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
        size_t i;

        if (uws->protocol_count > 0)
        {
            for (i = 0; i < uws->protocol_count; i++)
            {
                free(uws->protocols[i].protocol);
            }
            free(uws->protocols);
        }

        /* Codes_SRS_UWS_01_019: [ `uws_destroy` shall free all resources associated with the uws instance. ]*/
        /* Codes_SRS_UWS_01_023: [ `uws_destroy` shall destroy the underlying IO created in `uws_create` by calling `xio_destroy`. ]*/
        xio_destroy(uws->underlying_io);
        /* Codes_SRS_UWS_01_024: [ `uws_destroy` shall free the list used to track the pending sends by calling `singlylinkedlist_destroy`. ]*/
        singlylinkedlist_destroy(uws->pending_sends);
        free(uws->resource_name);
        free(uws->hostname);

        /* Codes_SRS_UWS_01_424: [ `uws_destroy` shall free the buffer allocated in `uws_create` by calling `BUFFER_delete`. ]*/
        BUFFER_delete(uws->encode_buffer);
        free(uws);
    }
}

static void indicate_ws_open_complete_error(UWS_INSTANCE* uws, WS_OPEN_RESULT ws_open_result)
{
    /* Codes_SRS_UWS_01_409: [ After any error is indicated by `on_ws_open_complete`, a subsequent `uws_open` shall be possible. ]*/
    uws->on_ws_open_complete(uws->on_ws_open_complete_context, ws_open_result);
    uws->uws_state = UWS_STATE_CLOSED;
}

static void indicate_ws_open_complete_error_and_close(UWS_INSTANCE* uws, WS_OPEN_RESULT ws_open_result)
{
    indicate_ws_open_complete_error(uws, ws_open_result);
    xio_close(uws->underlying_io, NULL, NULL);
}

static void indicate_ws_error(UWS_INSTANCE* uws, WS_ERROR error_code)
{
    uws->uws_state = UWS_STATE_ERROR;
    uws->on_ws_error(uws->on_ws_error_context, error_code);
}

static void indicate_ws_error_and_close(UWS_INSTANCE* uws, WS_ERROR error_code, unsigned int close_error_code)
{
    unsigned char close_frame[4];
    close_frame[0] = 0x88;
    close_frame[1] = 0x02;
    close_frame[2] = (unsigned char)(close_error_code >> 8);
    close_frame[3] = (unsigned char)(close_error_code & 0xFF);

    uws->uws_state = UWS_STATE_ERROR;
    if (xio_send(uws->underlying_io, close_frame, sizeof(close_frame), NULL, NULL) != 0)
    {

    }
    else
    {
        uws->on_ws_error(uws->on_ws_error_context, error_code);
    }
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    UWS_HANDLE uws = context;
    /* Codes_SRS_UWS_01_401: [ If `on_underlying_io_open_complete` is called with a NULL context, `on_underlying_io_open_complete` shall do nothing. ]*/
    if (uws == NULL)
    {
        LogError("NULL context");
    }
    else
    {
        switch (uws->uws_state)
        {
        default:
        case UWS_STATE_WAITING_FOR_UPGRADE_RESPONSE:
            /* Codes_SRS_UWS_01_407: [ When `on_underlying_io_open_complete` is called when the uws instance has send the upgrade request but it is waiting for the response, an error shall be reported to the user by calling the `on_ws_open_complete` with `WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS`. ]*/
            LogError("underlying on_io_open_complete was called again after upgrade request was sent.");
            indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS);
            break;
        case UWS_STATE_OPENING_UNDERLYING_IO:
            switch (open_result)
            {
            default:
            case IO_OPEN_ERROR:
                /* Codes_SRS_UWS_01_369: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_ERROR` while uws is OPENING (`uws_open` was called), uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED`. ]*/
                indicate_ws_open_complete_error(uws, WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED);
                break;
            case IO_OPEN_CANCELLED:
                /* Codes_SRS_UWS_01_402: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_CANCELLED` while uws is OPENING (`uws_open` was called), uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED`. ]*/
                indicate_ws_open_complete_error(uws, WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED);
                break;
            case IO_OPEN_OK:
            {
                int upgrade_request_length;
                char* upgrade_request;

                /* Codes_SRS_UWS_01_371: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_OK` while uws is OPENING (`uws_open` was called), uws shall prepare the WebSockets upgrade request. ]*/
                const char upgrade_request_format[] = "GET %s HTTP/1.1\r\n"
                    "Host: %s:%d\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                    "Sec-WebSocket-Protocol: %s\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "\r\n";

                upgrade_request_length = snprintf(NULL, 0, upgrade_request_format,
                    uws->resource_name,
                    uws->hostname,
                    uws->port,
                    "");
                if (upgrade_request_length < 0)
                {
                    /* Codes_SRS_UWS_01_408: [ If constructing of the WebSocket upgrade request fails, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_CONSTRUCTING_UPGRADE_REQUEST`. ]*/
                    LogError("Cannot construct the WebSocket upgrade request");
                    indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_CONSTRUCTING_UPGRADE_REQUEST);
                }
                else
                {
                    upgrade_request = (char*)malloc(upgrade_request_length);
                    if (upgrade_request == NULL)
                    {
                        /* Codes_SRS_UWS_01_406: [ If not enough memory can be allocated to construct the WebSocket upgrade request, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_NOT_ENOUGH_MEMORY`. ]*/
                        LogError("Cannot allocate memory for the WebSocket upgrade request");
                        indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY);
                    }
                    else
                    {
                        /* No need to have any send complete here, as we are monitoring the received bytes */
                        /* Codes_SRS_UWS_01_372: [ Once prepared the WebSocket upgrade request shall be sent by calling `xio_send`. ]*/
                        if (xio_send(uws->underlying_io, upgrade_request, upgrade_request_length, NULL, NULL) != 0)
                        {
                            /* Codes_SRS_UWS_01_373: [ If `xio_send` fails then uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST`. ]*/
                            LogError("Cannot send upgrade request");
                            indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST);
                        }
                        else
                        {
                            uws->uws_state = UWS_STATE_WAITING_FOR_UPGRADE_RESPONSE;
                        }

                        free(upgrade_request);
                    }
                }

                break;
            }
            }
        }
    }
}

static void consume_received_bytes(UWS_INSTANCE* uws, size_t consumed_bytes)
{
    if (consumed_bytes < uws->received_bytes_count)
    {
        (void)memmove(uws->received_bytes, uws->received_bytes + consumed_bytes, uws->received_bytes_count - consumed_bytes);
    }

    uws->received_bytes_count -= consumed_bytes;
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    /* Codes_SRS_UWS_01_415: [ If called with a NULL `context` argument, `on_underlying_io_bytes_received` shall do nothing. ]*/
    if (context != NULL)
    {
        UWS_HANDLE uws = context;

        if ((buffer == NULL) ||
            (size == 0))
        {
            /* Codes_SRS_UWS_01_416: [ If called with NULL `buffer` or zero `size` and the state of the iws is OPENING, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS`. ]*/
            indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS);
        }
        else
        {
            unsigned char decode_stream = 1;

            switch (uws->uws_state)
            {
            default:
            case UWS_STATE_CLOSED:
                decode_stream = 0;
                break;

            case UWS_STATE_OPENING_UNDERLYING_IO:
                /* Codes_SRS_UWS_01_417: [ When `on_underlying_io_bytes_received` is called while OPENING but before the `on_underlying_io_open_complete` has been called, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN`. ]*/
                indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN);
                decode_stream = 0;
                break;

            case UWS_STATE_WAITING_FOR_UPGRADE_RESPONSE:
            {
                /* Codes_SRS_UWS_01_378: [ When `on_underlying_io_bytes_received` is called while the uws is OPENING, the received bytes shall be accumulated in order to attempt parsing the WebSocket Upgrade response. ]*/
                unsigned char* new_received_bytes = (unsigned char*)realloc(uws->received_bytes, uws->received_bytes_count + size);
                if (new_received_bytes == NULL)
                {
                    /* Codes_SRS_UWS_01_379: [ If allocating memory for accumulating the bytes fails, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_NOT_ENOUGH_MEMORY`. ]*/
                    indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY);
                    decode_stream = 0;
                }
                else
                {
                    uws->received_bytes = new_received_bytes;
                    (void)memcpy(uws->received_bytes + uws->received_bytes_count, buffer, size);
                    uws->received_bytes_count += size;

                    decode_stream = 1;
                }

                break;
            }

            case UWS_STATE_OPEN:
            {
                /* Codes_SRS_UWS_01_385: [ If the state of the uws instance is OPEN, the received bytes shall be used for decoding WebSocket frames. ]*/
                unsigned char* new_received_bytes = (unsigned char*)realloc(uws->received_bytes, uws->received_bytes_count + size);
                if (new_received_bytes == NULL)
                {
                    /* Codes_SRS_UWS_01_418: [ If allocating memory for the bytes accumulated for decoding WebSocket frames fails, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_NOT_ENOUGH_MEMORY`. ]*/
                    LogError("Cannot allocate memory for received data");
                    indicate_ws_error(uws, WS_ERROR_NOT_ENOUGH_MEMORY);

                    decode_stream = 0;
                }
                else
                {
                    uws->received_bytes = new_received_bytes;
                    (void)memcpy(uws->received_bytes + uws->received_bytes_count, buffer, size);
                    uws->received_bytes_count += size;
                
                    decode_stream = 1;
                }

                break;
            }
            }

            while (decode_stream)
            {
                decode_stream = 0;

                switch (uws->uws_state)
                {
                default:
                case UWS_STATE_CLOSED:
                    break;

                case UWS_STATE_OPENING_UNDERLYING_IO:
                    /* Codes_SRS_UWS_01_417: [ When `on_underlying_io_bytes_received` is called while OPENING but before the `on_underlying_io_open_complete` has been called, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN`. ]*/
                    indicate_ws_open_complete_error_and_close(uws, WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN);
                    break;

                case UWS_STATE_WAITING_FOR_UPGRADE_RESPONSE:
                {
                    char* request_end_ptr;

                    /* Codes_SRS_UWS_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
                    /* Codes_SRS_UWS_01_381: [ If the status is 101, uws shall be considered OPEN and this shall be indicated by calling the `on_ws_open_complete` callback passed to `uws_open` with `IO_OPEN_OK`. ]*/
                    if ((uws->received_bytes_count >= 4) &&
                        ((request_end_ptr = strstr((const char*)uws->received_bytes, "\r\n\r\n")) != NULL))
                    {
                        /* Codes_SRS_UWS_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
                        consume_received_bytes(uws, request_end_ptr - (char*)uws->received_bytes + 4);
                        uws->uws_state = UWS_STATE_OPEN;
                        uws->on_ws_open_complete(uws->on_ws_open_complete_context, WS_OPEN_OK);

                        decode_stream = 1;
                    }

                    break;
                }

                case UWS_STATE_OPEN:
                {
                    size_t needed_bytes = 2;
                    size_t length;

                    if (uws->received_bytes_count >= needed_bytes)
                    {
                        unsigned char has_error = 0;

                        /* Codes_SRS_UWS_01_160: [ Defines whether the "Payload data" is masked. ]*/
                        if ((uws->received_bytes[1] & 0x80) != 0)
                        {
                            /* Codes_SRS_UWS_01_144: [ A client MUST close a connection if it detects a masked frame. ]*/
                            /* Codes_SRS_UWS_01_145: [ In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) ]*/
                            indicate_ws_error_and_close(uws, WS_ERROR_BAD_FRAME_RECEIVED, 1002);
                        }

                        /* Codes_SRS_UWS_01_163: [ The length of the "Payload data", in bytes: ]*/
                        /* Codes_SRS_UWS_01_164: [ if 0-125, that is the payload length. ]*/
                        length = uws->received_bytes[1];

                        if (length == 126)
                        {
                            /* Codes_SRS_UWS_01_165: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
                            needed_bytes += 2;
                            if (uws->received_bytes_count >= needed_bytes)
                            {
                                /* Codes_SRS_UWS_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
                                length = ((uint64_t)(uws->received_bytes[2]) << 8) + uws->received_bytes[3];

                                if (length < 126)
                                {
                                    /* Codes_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
                                    LogError("Bad frame: received a %u length on the 16 bit length", (unsigned int)length);

                                    /* Codes_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
                                    indicate_ws_error(uws, WS_ERROR_BAD_FRAME_RECEIVED);
                                    has_error = 1;
                                }
                                else
                                {
                                    needed_bytes += (size_t)length;
                                }
                            }
                        }
                        else if (length == 127)
                        {
                            /* Codes_SRS_UWS_01_166: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
                            needed_bytes += 8;
                            if (uws->received_bytes_count >= needed_bytes)
                            {
                                if ((uws->received_bytes[2] & 0x80) != 0)
                                {
                                    LogError("Bad frame: received a 64 bit length frame with the highest bit set");

                                    /* Codes_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
                                    indicate_ws_error(uws, WS_ERROR_BAD_FRAME_RECEIVED);
                                    has_error = 1;
                                }
                                else
                                {
                                    /* Codes_SRS_UWS_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
                                    length = (size_t)(((uint64_t)(uws->received_bytes[2]) << 56) +
                                        (((uint64_t)uws->received_bytes[3]) << 48) +
                                        (((uint64_t)uws->received_bytes[4]) << 40) +
                                        (((uint64_t)uws->received_bytes[5]) << 32) +
                                        (((uint64_t)uws->received_bytes[6]) << 24) +
                                        (((uint64_t)uws->received_bytes[7]) << 16) +
                                        (((uint64_t)uws->received_bytes[8]) << 8) +
                                        (uint64_t)(uws->received_bytes[9]));

                                    if (length < 65536)
                                    {
                                        /* Codes_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
                                        LogError("Bad frame: received a %u length on the 64 bit length", (unsigned int)length);

                                        /* Codes_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
                                        indicate_ws_error(uws, WS_ERROR_BAD_FRAME_RECEIVED);
                                        has_error = 1;
                                    }
                                    else
                                    {
                                        needed_bytes += length;
                                    }
                                }
                            }
                        }
                        else
                        {
                            needed_bytes += length;
                        }

                        if ((has_error == 0) &&
                            (uws->received_bytes_count >= needed_bytes))
                        {
                            unsigned char opcode = uws->received_bytes[0] & 0xF;

                            switch (opcode)
                            {
                            default:
                                break;

                                /* Codes_SRS_UWS_01_153: [ *  %x1 denotes a text frame ]*/
                            case OPCODE_TEXT_FRAME:
                                /* Codes_SRS_UWS_01_386: [ When a WebSocket data frame is decoded succesfully it shall be indicated via the callback `on_ws_frame_received`. ]*/
                                /* Codes_SRS_UWS_01_169: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
                                /* Codes_SRS_UWS_01_173: [ The "Payload data" is defined as "Extension data" concatenated with "Application data". ]*/
                                uws->on_ws_frame_received(uws->on_ws_frame_received_context, WS_FRAME_TYPE_TEXT, uws->received_bytes + needed_bytes - length, length);
                                decode_stream = 1;
                                break;

                                /* Codes_SRS_UWS_01_154: [ *  %x2 denotes a binary frame ]*/
                            case OPCODE_BINARY_FRAME:
                                /* Codes_SRS_UWS_01_386: [ When a WebSocket data frame is decoded succesfully it shall be indicated via the callback `on_ws_frame_received`. ]*/
                                /* Codes_SRS_UWS_01_169: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
                                /* Codes_SRS_UWS_01_173: [ The "Payload data" is defined as "Extension data" concatenated with "Application data". ]*/
                                uws->on_ws_frame_received(uws->on_ws_frame_received_context, WS_FRAME_TYPE_BINARY, uws->received_bytes + needed_bytes - length, length);
                                decode_stream = 1;
                                break;
                            }

                            consume_received_bytes(uws, needed_bytes);
                        }
                    }

                    break;
                }
                }
            }
        }
    }
}

static void on_underlying_io_close_complete(void* context)
{
    (void)context;
}

static void on_underlying_io_error(void* context)
{
    (void)context;
}

int uws_open(UWS_HANDLE uws, ON_WS_OPEN_COMPLETE on_ws_open_complete, void* on_ws_open_complete_context, ON_WS_FRAME_RECEIVED on_ws_frame_received, void* on_ws_frame_received_context, ON_WS_ERROR on_ws_error, void* on_ws_error_context)
{
    int result;

    (void)on_ws_error_context;

    /* Codes_SRS_UWS_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
    if ((uws == NULL) ||
        (on_ws_open_complete == NULL) ||
        (on_ws_frame_received == NULL) ||
        (on_ws_error == NULL))
    {
        /* Codes_SRS_UWS_01_027: [ If `uws`, `on_ws_open_complete`, `on_ws_frame_received` or `on_ws_error` is NULL, `uws_open` shall fail and return a non-zero value. ]*/
        LogError("Invalid arguments: uws=%p, on_ws_open_complete=%p, on_ws_frame_received=%p, on_ws_error=%p",
            uws, on_ws_open_complete, on_ws_frame_received, on_ws_error);
        result = __LINE__;
    }
    else
    {
        if (uws->uws_state != UWS_STATE_CLOSED)
        {
            /* Codes_SRS_UWS_01_400: [ `uws_open` while CLOSING shall fail and return a non-zero value. ]*/
            /* Codes_SRS_UWS_01_394: [ `uws_open` while the uws instance is already OPEN or OPENING shall fail and return a non-zero value. ]*/
            LogError("Invalid uWS state while trying to open: %d", (int)uws->uws_state);
            result = __LINE__;
        }
        else
        {
            /* Codes_SRS_UWS_01_025: [ `uws_open` shall open the underlying IO by calling `xio_open` and providing the IO handle created in `uws_create` as argument. ]*/
            /* Codes_SRS_UWS_01_367: [ The callbacks `on_underlying_io_open_complete`, `on_underlying_io_bytes_received` and `on_underlying_io_error` shall be passed as arguments to `xio_open`. ]*/
            if (xio_open(uws->underlying_io, on_underlying_io_open_complete, uws, on_underlying_io_bytes_received, uws, on_underlying_io_error, uws) != 0)
            {
                /* Codes_SRS_UWS_01_028: [ If opening the underlying IO fails then `uws_open` shall fail and return a non-zero value. ]*/
                LogError("Opening the underlying IO failed");
                result = __LINE__;
            }
            else
            {
                uws->uws_state = UWS_STATE_OPENING_UNDERLYING_IO;
                uws->on_ws_open_complete = on_ws_open_complete;
                uws->on_ws_open_complete_context = on_ws_open_complete_context;
                uws->on_ws_frame_received = on_ws_frame_received;
                uws->on_ws_frame_received_context = on_ws_frame_received_context;
                uws->on_ws_error = on_ws_error;
                uws->on_ws_error_context = on_ws_error_context;

                /* Codes_SRS_UWS_01_026: [ On success, `uws_open` shall return 0. ]*/
                result = 0;
            }
        }
    }

    return result;
}

/* Codes_SRS_UWS_01_029: [ `uws_close` shall close the uws instance connection if an open action is either pending or has completed successfully (if the IO is open). ]*/
int uws_close(UWS_HANDLE uws, ON_WS_CLOSE_COMPLETE on_ws_close_complete, void* on_ws_close_complete_context)
{
    int result;

    /* Codes_SRS_UWS_01_397: [ The `on_ws_close_complete` argument shall be allowed to be NULL, in which case no callback shall be called when the close is complete. ]*/
    /* Codes_SRS_UWS_01_398: [ `on_ws_close_complete_context` shall also be allows to be NULL. ]*/
    if (uws == NULL)
    {
        /* Codes_SRS_UWS_01_030: [ if `uws` is NULL, `uws_close` shall return a non-zero value. ]*/
        LogError("NULL uWS handle.");
        result = __LINE__;
    }
    else
    {
        if ((uws->uws_state == UWS_STATE_CLOSED) ||
            (uws->uws_state == UWS_STATE_CLOSING))
        {
            /* Codes_SRS_UWS_01_032: [ `uws_close` when no open action has been issued shall fail and return a non-zero value. ]*/
            LogError("close has been called when already CLOSED");
            result = __LINE__;
        }
        else
        {
            /* Codes_SRS_UWS_01_399: [ `on_ws_close_complete` and `on_ws_close_complete_context` shall be saved and the callback `on_ws_close_complete` shall be triggered when the close is complete. ]*/
            uws->on_ws_close_complete = on_ws_close_complete;
            uws->on_ws_close_complete_context = on_ws_close_complete_context;
            
            uws->uws_state = UWS_STATE_CLOSING;

            /* Codes_SRS_UWS_01_031: [ `uws_close` shall close the connection by calling `xio_close` while passing as argument the IO handle created in `uws_create`. ]*/
            /* Codes_SRS_UWS_01_368: [ The callback `on_underlying_io_close` shall be passed as argument to `xio_close`. ]*/
            if (xio_close(uws->underlying_io, on_underlying_io_close_complete, uws) != 0)
            {
                /* Codes_SRS_UWS_01_395: [ If `xio_close` fails, `uws_close` shall fail and return a non-zero value. ]*/
                LogError("Closing the underlying IO failed.");
                result = __LINE__;
            }
            else
            {
                /* Codes_SRS_UWS_01_396: [ On success `uws_close` shall return 0. ]*/
                result = 0;
            }
        }
    }

    return result;
}
