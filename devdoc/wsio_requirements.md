# wsio requirements
 
## Overview

`ws_io` is module that implements a concrete IO that implements the WebSockets protocol by using the uws library.

## References

RFC6455 - The WebSocket Protocol.

## Exposed API

```c
typedef struct WSIO_CONFIG_TAG
{
	const char* hostname;
	XIO_HANDLE underlying_io;
	const char* resource_name;
	const char* protocol;
} WSIO_CONFIG;

extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

### wsio_create

```c
extern CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters);
```

X**SRS_WSIO_01_001: [**`wsio_create` shall create an instance of wsio and return a non-NULL handle to it.**]**
**SRS_WSIO_01_065: [** If the argument `io_create_parameters` is NULL then `wsio_create` shall return NULL. **]**
X**SRS_WSIO_01_066: [** `io_create_parameters` shall be used as a `WSIO_CONFIG*` . **]**
**SRS_WSIO_01_067: [** If any of the members `hostname`, `resource_name` or `protocol` is NULL in `WSIO_CONFIG` then `wsio_create` shall return NULL. **]**
**SRS_WSIO_01_068: [** If allocating memory for the new wsio instance fails then `wsio_create` shall return NULL. **]**
X**SRS_WSIO_01_070: [** The underlying uws instance shall be created by calling `uws_create_with_io`. **]**
X**SRS_WSIO_01_071: [** The arguments for `uws_create_with_io` shall be: **]**
X**SRS_WSIO_01_072: [** - `hostname` set to the `hostname` field in the `io_create_parameters` passed to `wsio_create`. **]**
- `resource_name` set to the `resource_name` field in the `io_create_parameters` passed to `wsio_create`.
- `protocols` shall be filled with only one structure, that shall have the `protocol` set to the value of the `protocol` field in the `io_create_parameters` passed to `wsio_create`.
X**SRS_WSIO_01_073: [** - `underlying_io` shall be set to the `underlying_io` member in `io_create_parameters`. **]**
**SRS_WSIO_01_075: [** If `uws_create_with_io` fails, then `wsio_create` shall fail and return NULL. **]**
X**SRS_WSIO_01_076: [** `wsio_create` shall create a pending send IO list that is to be used to queue send packets by calling `singlylinkedlist_create`. **]**
**SRS_WSIO_01_077: [** If `singlylinkedlist_create` fails then `wsio_create` shall fail and return NULL. **]**

### wsio_destroy

```c
extern void wsio_destroy(CONCRETE_IO_HANDLE ws_io);
```

**SRS_WSIO_01_078: [** `wsio_destroy` shall free all resources associated with the wsio instance. **]**
**SRS_WSIO_01_079: [** If `ws_io` is NULL, `wsio_destroy` shall do nothing.  **]**
**SRS_WSIO_01_080: [** `wsio_destroy` shall destroy the uws instance created in `wsio_create` by calling `uws_destroy`. **]**
**SRS_WSIO_01_081: [** `wsio_destroy` shall free the list used to track the pending send IOs by calling `singlylinkedlist_destroy`. **]**

### wsio_open

```c
extern int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
```

**SRS_WSIO_01_082: [** `wsio_open` shall open the underlying uws instance by calling `uws_open` and providing the uws handle created in `wsio_create` as argument. **]**
**SRS_WSIO_01_083: [** On success, `wsio_open` shall return 0. **]**
**SRS_WSIO_01_084: [** If opening the underlying uws instance fails then `wsio_open` shall fail and return a non-zero value. **]**

### wsio_close

```c
extern int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_WSIO_01_085: [** `wsio_close` shall close the websockets IO if an open action is either pending or has completed successfully (if the IO is open).  **]**
**SRS_WSIO_01_086: [** if `ws_io` is NULL, `wsio_close` shall return a non-zero value.  **]**
**SRS_WSIO_01_087: [** `wsio_close` shall call `uws_close` while passing as argument the IO handle created in `wsio_create`.  **]**
**SRS_WSIO_01_088: [** `wsio_close` when no open action has been issued shall fail and return a non-zero value. **]**
**SRS_WSIO_01_089: [** `wsio_close` after a `wsio_close` shall fail and return a non-zero value.  **]**
**SRS_WSIO_01_090: [** The argument `on_io_close_complete` shall be optional, if NULL is passed by the caller then no close complete callback shall be triggered.  **]**
**SRS_WSIO_01_091: [** `wsio_close` shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item. **]**
**SRS_WSIO_01_092: [** Obtaining the head of the pending IO list shall be done by calling `singlylinkedlist_get_head_item`. **]**
**SRS_WSIO_01_093: [** For each pending item the send complete callback shall be called with `IO_SEND_CANCELLED`.**\]** **]**
**SRS_WSIO_01_094: [** The callback context passed to the `on_send_complete` callback shall be the context given to `wsio_send`.  **]**

### wsio_send

```c
extern int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

**SRS_WSIO_01_095: [** `wsio_send` shall call `uws_send_frame`, passing the `buffer` and `size` arguments as they are: **]**
**SRS_WSIO_01_096: [** The frame type used shall be `WS_FRAME_TYPE_BINARY`. **]**
**SRS_WSIO_01_097: [** The `is_final` argument shall be set to true. **]**
**SRS_WSIO_01_098: [** On success, `wsio_send` shall return 0. **]**
**SRS_WSIO_01_099: [** If the wsio is not OPEN (open has not been called or is still in progress) then `wsio_send` shall fail and return a non-zero value. **]**
**SRS_WSIO_01_100: [** If any of the arguments `ws_io` or `buffer` are NULL, `wsio_send` shall fail and return a non-zero value. **]**
**SRS_WSIO_01_101: [** If `size` is zero then `wsio_send` shall fail and return a non-zero value. **]**
**SRS_WSIO_01_102: [** An entry shall be queued in the singly linked list by calling `singlylinkedlist_add`. **]**
**SRS_WSIO_01_103: [** The entry shall contain the `on_send_complete` callback and its context. **]**
**SRS_WSIO_01_104: [** If `singlylinkedlist_add` fails, `wsio_send` shall fail and return a non-zero value. **]**
**SRS_WSIO_01_105: [** The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered. **]**

### wsio_dowork

```c
extern void wsio_dowork(CONCRETE_IO_HANDLE ws_io);
```

**SRS_WSIO_01_106: [** `wsio_dowork` shall call `uws_dowork` with the uws handle created in `wsio_create`. **]**
**SRS_WSIO_01_107: [** If the `ws_io` argument is NULL, `wsio_dowork` shall do nothing. **]**
**SRS_WSIO_01_108: [** If the IO is not yet open, `wsio_dowork` shall do nothing. **]**

### wsio_setoption

```c
extern int wsio_setoption(CONCRETE_IO_HANDLE ws_io, const char* option_name, const void* value);
```

**SRS_WSIO_01_109: [** If any of the arguments `ws_io` or `option_name` is NULL `wsio_setoption` shall return a non-zero value. **]**
**SRS_WSIO_01_110: [** If the `option_name` argument indicates an option that is not handled by wsio, then `wsio_setoption` shall return a non-zero value. **]**
**SRS_WSIO_01_111: [** If the option was handled by wsio, then `wsio_setoption` shall return 0. **]**

Options that shall be handled by wsio:
**SRS_WSIO_01_112: [** None **]**

### wsio_clone_option

```c
extern void* wsio_clone_option(const char* name, const void* value);
```

**SRS_WSIO_01_113: [** If the `name` or `value` arguments are NULL, `wsio_clone_option` shall return NULL. **]**
**SRS_WSIO_01_115: [** `wsio_clone_option` shall not handle any options and thus return NULL. **]**

### wsio_destroy_option

```c
extern void wsio_destroy_option(const char* name, const void* value);
```

**SRS_WSIO_01_114: [** If any of the arguments is NULL, `wsio_destroy_option` shall do nothing. **]**
**SRS_WSIO_01_116: [** `wsio_destroy_option` shall not handle any options. **]**

### wsio_retrieveoptions

```c
OPTIONHANDLER_HANDLE wsio_retrieveoptions(CONCRETE_IO_HANDLE handle)
```

**SRS_WSIO_01_117: [** `wsio_retrieveoptions` produces an `OPTIONHANDLER_HANDLE`.  **]**

**SRS_WSIO_01_118: [** If parameter `handle` is `NULL` then `wsio_retrieveoptions` shall fail and return NULL. **]**
**SRS_WSIO_01_119: [** `wsio_retrieveoptions` shall produce an `OPTIONHANDLER_HANDLE`. **]**
**SRS_WSIO_01_120: [** If producing the `OPTIONHANDLER_HANDLE` fails then `wsio_retrieveoptions` shall fail and return NULL.  **]**

### wsio_get_interface_description

```c
extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

**SRS_WSIO_01_064: [**wsio_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: wsio_retrieveoptions, wsio_create, wsio_destroy, wsio_open, wsio_close, wsio_send and wsio_dowork.**]** 

### on_uws_error

**SRS_WSIO_01_121: [** When `on_uws_error` is called while the IO is OPEN the wsio instance shall be set to ERROR and an error shall be indicated via the `on_io_error` callback passed to `wsio_open`. **]**
**SRS_WSIO_01_123: [** When calling `on_io_error`, the `on_io_error_context` argument given in `wsio_open` shall be passed to the callback `on_io_error`. **]**
**SRS_WSIO_01_122: [** When `on_uws_error` is called while the IO is OPENING, the `on_io_open_complete` callback passed to `wsio_open` shall be called with `IO_OPEN_ERROR`. **]**

### on_ws_frame_received

**SRS_WSIO_01_124: [** When `on_ws_frame_received` is called the bytes in the frame shall be indicated by calling the `on_bytes_received` callback passed to `wsio_open`. **]**
**SRS_WSIO_01_125: [** When calling `on_bytes_received`, the `on_bytes_received_context` argument given in `wsio_open` shall be passed to the callback `on_bytes_received`. **]**
**SRS_WSIO_01_126: [** If `on_ws_frame_received` is called while the IO is OPENING, an error shall be indicated via the `on_io_error` callback passed to `wsio_open`. **]**
**SRS_WSIO_01_127: [** When calling `on_io_error`, the `on_io_error_context` argument given in `wsio_open` shall be passed to the callback `on_io_error`. **]**

### on_ws_open_complete

### on_underlying_io_send_complete

