# wsio requirements
 
## Overview

`ws_io` is module that implements a concrete IO that implements the WebSockets protocol by using the uws library.

## References

RFC6455 - The WebSocket Protocol.

## Exposed API

```c
typedef struct WSIO_CONFIG_TAG
{
	const char* host;
	int use_ssl : 1;
} WSIO_CONFIG;

extern CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters);
extern void wsio_destroy(CONCRETE_IO_HANDLE ws_io);
extern int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, ON_BYTES_RECEIVED on_bytes_received, ON_IO_ERROR on_io_error, void* callback_context);
extern int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
extern int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void wsio_dowork(CONCRETE_IO_HANDLE ws_io);
extern int wsio_setoption(CONCRETE_IO_HANDLE ws_io, const char* optionName, const void* value);
extern void* wsio_clone_option(const char* name, const void* value);
extern void wsio_destroy_option(const char* name, const void* value);
extern OPTIONHANDLER_HANDLE wsio_retrieveoptions(CONCRETE_IO_HANDLE handle);

extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

### wsio_create

```c
extern CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters);
```

**SRS_WSIO_01_001: \[**`wsio_create` shall create an instance of wsio and return a non-NULL handle to it.**\]**
If the argument `io_create_parameters` is NULL then `wsio_create` shall return NULL.
`io_create_parameters` shall be used as a `WSIO_CONFIG*` .
If the member `host` is NULL in `WSIO_CONFIG` then `wsio_create` shall return NULL.
If allocating memory for the new wsio instance fails then `wsio_create` shall return NULL.
The member `host` shall be copied for later use (they are needed when the IO is opened).
If `use_ssl` is 0 then `wsio_create` shall obtain the interface used to create a socketio instance by calling `socketio_get_interface_description`.
If `use_ssl` is 1 then `wsio_create` shall obtain the interface used to create a tlsio instance by calling `platform_get_default_tlsio`.
The underlying IO shall be created by calling `uws_create`.
The create arguments for `uws_create` shall be:
- `hostname` set to the `host` field in the `io_create_parameters` passed to `wsio_create`.
- `port` set to 80 if `use_ssl` is 0.
- `port` set to 443 if `use_ssl` is 1.
If `uws_create` fails, then `wsio_create` shall fail and return NULL.
`wsio_create` shall create a pending send IO list that is to be used to queue send packets by calling `singlylinkedlist_create`.
If `singlylinkedlist_create` fails then `wsio_create` shall fail and return NULL.

### wsio_destroy

```c
extern void wsio_destroy(CONCRETE_IO_HANDLE ws_io);
```

`wsio_destroy` shall free all resources associated with the wsio instance.
If `ws_io` is NULL, `wsio_destroy` shall do nothing. 
`wsio_destroy` shall destroy the uws instance created in `wsio_create` by calling `uws_destroy`.
`wsio_destroy` shall free the list used to track the pending send IOs by calling `singlylinkedlist_destroy`.

### wsio_open

```c
extern int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, ON_BYTES_RECEIVED on_bytes_received, ON_IO_ERROR on_io_error, void* callback_context);
```

`wsio_open` shall open the underlying uws instance by calling `uws_open` and providing the uws handle created in `wsio_create` as argument.
On success, `wsio_open` shall return 0.
If opening the underlying uws instance fails then `wsio_open` shall fail and return a non-zero value.

### wsio_close

```c
extern int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

`wsio_close` shall close the websockets IO if an open action is either pending or has completed successfully (if the IO is open). 
if `ws_io` is NULL, `wsio_close` shall return a non-zero value. 
`wsio_close` shall close the connection by calling `xio_close` while passing as argument the IO handle created in `wsio_create`. 
`wsio_close` when no open action has been issued shall fail and return a non-zero value.
`wsio_close` after a `wsio_close` shall fail and return a non-zero value. 
The callback `on_io_close_complete` shall be called after the close action has been completed in the context of `wsio_close` (`wsio_close` is effectively blocking).
The `callback_context` argument shall be passed to `on_io_close_complete` as is. 
The argument `on_io_close_complete` shall be optional, if NULL is passed by the caller then no close complete callback shall be triggered. 
`wsio_close` shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item.
Obtaining the head of the pending IO list shall be done by calling `singlylinkedlist_get_head_item`.
For each pending item the send complete callback shall be called with `IO_SEND_CANCELLED`.**\]**
The callback context passed to the `on_send_complete` callback shall be the context given to `wsio_send`. 

### wsio_send

```c
extern int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

`wsio_send` shall create and queue a structure that contains:
- the websocket frame containing the `size` bytes pointed by `buffer`, so that the frame can be later sent when `wsio_dowork` is called
- the send complete callback `on_send_complete`
- the send complete callback context `on_send_complete_context`
On success, `wsio_send` shall return 0.
If the wsio is not OPEN (open has not been called or is still in progress) then `wsio_send` shall fail and return a non-zero value.
If any of the arguments `ws_io` or `buffer` are NULL, `wsio_send` shall fail and return a non-zero value.
If `size` is zero then `wsio_send` shall fail and return a non-zero value.
`wsio_send` shall allocate enough memory to hold the websocket frame that contains `size` bytes.
If allocating memory for the newly queued item fails, `wsio_send` shall fail and return a non-zero value.
Queueing shall be done by calling `singlylinkedlist_add`.
If `singlylinkedlist_add` fails, `wsio_send` shall fail and return a non-zero value.
The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered.

### wsio_dowork

```c
extern void wsio_dowork(CONCRETE_IO_HANDLE ws_io);
```

`wsio_dowork` shall iterate through all the pending sends and for each one of them:
`wsio_dowork` shall call `xio_send` to send the websocket frame with the following arguments:
- the io handle shall be the underlyiong IO handle created in `wsio_create`.
- the `buffer` argument shall point to the complete websocket frame to be sent.
- the `size` argument shall indicate the websocket frame length.
- the `send_complete` callback shall be the `wsio_send_complete` function.
- the `send_complete_context` argument shall identify the pending send.
If `xio_send` fails, `wsio_dowork` shall indicate that by calling the `on_send_complete` callback associated with the pending send with the `IO_SEND_ERROR` code. 
If the `ws_io` argument is NULL, `wsio_dowork` shall do nothing.
If the IO is not yet open, `wsio_dowork` shall do nothing.

### wsio_setoption

```c
extern int wsio_setoption(CONCRETE_IO_HANDLE ws_io, const char* option_name, const void* value);
```

If any of the arguments `ws_io` or `option_name` is NULL `wsio_setoption` shall return a non-zero value.
If the `option_name` argument indicates an option that is not handled by wsio, then `wsio_setoption` shall return a non-zero value.
If the option was handled by wsio, then `wsio_setoption` shall return 0.

Options that shall be handled by wsio:

### wsio_clone_option

```c
extern void* wsio_clone_option(const char* name, const void* value);
```

If the `name` or `value` arguments are NULL, `wsio_clone_option` shall return NULL.

### wsio_destroy_option

```c
extern void wsio_destroy_option(const char* name, const void* value);
```

If any of the arguments is NULL, `wsio_destroy_option` shall do nothing.

### wsio_retrieveoptions

```c
OPTIONHANDLER_HANDLE wsio_retrieveoptions(CONCRETE_IO_HANDLE handle)
```

`wsio_retrieveoptions` produces an `OPTIONHANDLER_HANDLE`. 

If parameter `handle` is `NULL` then `wsio_retrieveoptions` shall fail and return NULL.
`wsio_retrieveoptions` shall produce an `OPTIONHANDLER_HANDLE`.
If producing the `OPTIONHANDLER_HANDLE` fails then `wsio_retrieveoptions` shall fail and return NULL. 

### wsio_get_interface_description

```c
extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

**SRS_WSIO_01_064: \[**wsio_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: wsio_retrieveoptions, wsio_create, wsio_destroy, wsio_open, wsio_close, wsio_send and wsio_dowork.**\]** 

### on_underlying_io_error

### on_underlying_io_bytes_received

### on_underlying_io_open_complete

### on_underlying_io_send_complete
