// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef WSIO_H
#define WSIO_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#include <cstdbool>
#else
#include <stddef.h>
#include <stdbool.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"

typedef struct WSIO_CONFIG_TAG
{
    const char* hostname;
    XIO_HANDLE underlying_io;
    const char* resource_name;
    const char* protocol;
} WSIO_CONFIG;

extern CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters);
extern void wsio_destroy(CONCRETE_IO_HANDLE ws_io);
extern int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
extern int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
extern int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void wsio_dowork(CONCRETE_IO_HANDLE ws_io);
extern int wsio_setoption(CONCRETE_IO_HANDLE ws_io, const char* option_name, const void* value);
extern void* wsio_clone_option(const char* name, const void* value);
extern void wsio_destroy_option(const char* name, const void* value);
extern OPTIONHANDLER_HANDLE wsio_retrieveoptions(CONCRETE_IO_HANDLE handle);

extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WSIO_H */
