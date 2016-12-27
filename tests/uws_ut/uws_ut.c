// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>
#include <stdbool.h>

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"

/* TODO:
- Implement a feature in umock_c that can say "compare this argument as a given type" for the config structures
- Check the Websocket Upgrade request by breaking it into lines and parsing each line. Test on_underlying_io_open_complete_with_OK_prepares_and_sends_the_WebSocket_upgrade_request
*/

#define ENABLE_MOCKS

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/uws_frame_encoder.h"

TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);

static const void** list_items = NULL;
static size_t list_item_count = 0;
static const SINGLYLINKEDLIST_HANDLE TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE = (SINGLYLINKEDLIST_HANDLE)0x4242;
static const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x4243;
static const XIO_HANDLE TEST_IO_HANDLE = (XIO_HANDLE)0x4244;

static size_t currentmalloc_call;
static size_t whenShallmalloc_fail;
static size_t currentrealloc_call;
static size_t whenShallrealloc_fail;

static void* my_gballoc_malloc(size_t size)
{
    void* result;
    currentmalloc_call++;
    if (whenShallmalloc_fail > 0)
    {
        if (currentmalloc_call == whenShallmalloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
    }
    else
    {
        result = malloc(size);
    }
    return result;
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    void* result;
    currentrealloc_call++;
    if (whenShallrealloc_fail > 0)
    {
        if (currentrealloc_call == whenShallrealloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = realloc(ptr, size);
        }
    }
    else
    {
        result = realloc(ptr, size);
    }
    return result;
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    return 0;
}

static LIST_ITEM_HANDLE add_to_list(const void* item)
{
    const void** items = (const void**)realloc((void*)list_items, (list_item_count + 1) * sizeof(item));
    if (items != NULL)
    {
        list_items = items;
        list_items[list_item_count++] = item;
    }
    return (LIST_ITEM_HANDLE)list_item_count;
}

static int singlylinkedlist_remove_result;

static int my_singlylinkedlist_remove(SINGLYLINKEDLIST_HANDLE list, LIST_ITEM_HANDLE item)
{
    size_t index = (size_t)item - 1;
    (void)list;
    (void)memmove((void*)&list_items[index], &list_items[index + 1], sizeof(const void*) * (list_item_count - index - 1));
    list_item_count--;
    if (list_item_count == 0)
    {
        free((void*)list_items);
        list_items = NULL;
    }
    return singlylinkedlist_remove_result;
}

static LIST_ITEM_HANDLE my_singlylinkedlist_get_head_item(SINGLYLINKEDLIST_HANDLE list)
{
    LIST_ITEM_HANDLE list_item_handle = NULL;
    (void)list;
    if (list_item_count > 0)
    {
        list_item_handle = (LIST_ITEM_HANDLE)1;
    }
    else
    {
        list_item_handle = NULL;
    }
    return list_item_handle;
}

LIST_ITEM_HANDLE my_singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item)
{
    (void)list;
    return add_to_list(item);
}

const void* my_singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle)
{
    return (const void*)list_items[(size_t)item_handle - 1];
}

LIST_ITEM_HANDLE my_singlylinkedlist_find(SINGLYLINKEDLIST_HANDLE handle, LIST_MATCH_FUNCTION match_function, const void* match_context)
{
    size_t i;
    const void* found_item = NULL;
    (void)handle;
    for (i = 0; i < list_item_count; i++)
    {
        if (match_function((LIST_ITEM_HANDLE)list_items[i], match_context))
        {
            found_item = list_items[i];
            break;
        }
    }
    return (LIST_ITEM_HANDLE)found_item;
}

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/uws.h"

static const WS_PROTOCOL protocols[] = { { "test_protocol" } };

TEST_DEFINE_ENUM_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(WS_ERROR, WS_ERROR_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_ERROR, WS_ERROR_VALUES);
TEST_DEFINE_ENUM_TYPE(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT_VALUES);

static char* umocktypes_stringify_const_SOCKETIO_CONFIG_ptr(const SOCKETIO_CONFIG** value)
{
    char* result = NULL;
    char temp_buffer[1024];
    int length;
    length = sprintf(temp_buffer, "{ hostname = %s, port = %d, accepted_socket = %p }",
        (*value)->hostname,
        (*value)->port,
        (*value)->accepted_socket);

    if (length > 0)
    {
        result = (char*)malloc(strlen(temp_buffer) + 1);
        if (result != NULL)
        {
            (void)memcpy(result, temp_buffer, strlen(temp_buffer) + 1);
        }
    }

    return result;
}

static int umocktypes_are_equal_const_SOCKETIO_CONFIG_ptr(const SOCKETIO_CONFIG** left, const SOCKETIO_CONFIG** right)
{
    int result;

    if ((left == NULL) ||
        (right == NULL))
    {
        result = -1;
    }
    else
    {
        result = ((*left)->port == (*right)->port);
        result = result && ((*left)->accepted_socket == (*right)->accepted_socket);
        if (strcmp((*left)->hostname, (*right)->hostname) != 0)
        {
            result = 0;
        }
    }

    return result;
}

static char* copy_string(const char* source)
{
    char* result;

    if (source == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t length = strlen(source);
        result = (char*)malloc(length + 1);
        (void)memcpy(result, source, length + 1);
    }

    return result;
}

static int umocktypes_copy_const_SOCKETIO_CONFIG_ptr(SOCKETIO_CONFIG** destination, const SOCKETIO_CONFIG** source)
{
    int result;

    *destination = (SOCKETIO_CONFIG*)malloc(sizeof(SOCKETIO_CONFIG));
    if (*destination == NULL)
    {
        result = __LINE__;
    }
    else
    {
        if ((*source)->hostname == NULL)
        {
            (*destination)->hostname = NULL;
        }
        else
        {
            (*destination)->hostname = copy_string((*source)->hostname);
            (*destination)->port = (*source)->port;
            (*destination)->accepted_socket = (*source)->accepted_socket;
        }

        result = 0;
    }

    return result;
}

static void umocktypes_free_const_SOCKETIO_CONFIG_ptr(SOCKETIO_CONFIG** value)
{
    free((void*)(*value)->hostname);
    free(*value);
}

// consumer mocks
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_open_complete, void*, context, WS_OPEN_RESULT, ws_open_result);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_frame_received, void*, context, unsigned char, frame_type, const unsigned char*, buffer, size_t, size);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_error, void*, context, WS_ERROR, error_code);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_close_complete, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_send_frame_complete, void*, context, WS_SEND_FRAME_RESULT, ws_send_frame_result)
MOCK_FUNCTION_END()

static ON_IO_OPEN_COMPLETE g_on_io_open_complete;
static void* g_on_io_open_complete_context;
static ON_SEND_COMPLETE g_on_io_send_complete;
static void* g_on_io_send_complete_context;
static ON_BYTES_RECEIVED g_on_bytes_received;
static void* g_on_bytes_received_context;
static ON_IO_ERROR g_on_io_error;
static void* g_on_io_error_context;
static ON_IO_CLOSE_COMPLETE g_on_io_close_complete;
static void* g_on_io_close_complete_context;

static int my_xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    (void)xio;
    g_on_io_open_complete = on_io_open_complete;
    g_on_io_open_complete_context = on_io_open_complete_context;
    g_on_bytes_received = on_bytes_received;
    g_on_bytes_received_context = on_bytes_received_context;
    g_on_io_error = on_io_error;
    g_on_io_error_context = on_io_error_context;
    return 0;
}

static int my_xio_close(XIO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    (void)xio;
    g_on_io_close_complete = on_io_close_complete;
    g_on_io_close_complete_context = callback_context;
    return 0;
}

static int my_xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    (void)xio;
    (void)buffer;
    (void)size;
    g_on_io_send_complete = on_send_complete;
    g_on_io_send_complete_context = callback_context;
    return 0;
}

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static const IO_INTERFACE_DESCRIPTION* TEST_SOCKET_IO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x4542;
static const IO_INTERFACE_DESCRIPTION* TEST_TLS_IO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x4543;

extern BUFFER_HANDLE real_BUFFER_new(void);
extern void real_BUFFER_delete(BUFFER_HANDLE handle);
extern unsigned char* real_BUFFER_u_char(BUFFER_HANDLE handle);
extern size_t real_BUFFER_length(BUFFER_HANDLE handle);

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(uws_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, my_xio_close);
    REGISTER_GLOBAL_MOCK_HOOK(xio_send, my_xio_send);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_SOCKET_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(platform_get_default_tlsio, TEST_TLS_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_new, real_BUFFER_new);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_delete, real_BUFFER_delete);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_u_char, real_BUFFER_u_char);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_length, real_BUFFER_length);
    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
    REGISTER_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT);
    REGISTER_TYPE(WS_ERROR, WS_ERROR);
    REGISTER_TYPE(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT);
    REGISTER_TYPE(const SOCKETIO_CONFIG*, const_SOCKETIO_CONFIG_ptr);

    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(UWS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(UWS_FRAME_DECODER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_FRAME_DECODED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }

    umock_c_reset_all_calls();

    currentmalloc_call = 0;
    whenShallmalloc_fail = 0;
    currentrealloc_call = 0;
    whenShallrealloc_fail = 0;
    singlylinkedlist_remove_result = 0;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* uws_create */

/* Tests_SRS_UWS_01_001: [`uws_create` shall create an instance of uws and return a non-NULL handle to it.]*/
/* Tests_SRS_UWS_01_017: [ `uws_create` shall create a pending send IO list that is to be used to queue send packets by calling `singlylinkedlist_create`. ]*/
/* Tests_SRS_UWS_01_005: [ If `use_ssl` is 0 then `uws_create` shall obtain the interface used to create a socketio instance by calling `socketio_get_interface_description`. ]*/
/* Tests_SRS_UWS_01_008: [ The obtained interface shall be used to create the IO used as underlying IO by the newly created uws instance. ]*/
/* Tests_SRS_UWS_01_009: [ The underlying IO shall be created by calling `xio_create`. ]*/
/* Tests_SRS_UWS_01_010: [ The create arguments for the socket IO (when `use_ssl` is 0) shall have: ]*/
/* Tests_SRS_UWS_01_011: [ - `hostname` set to the `hostname` argument passed to `uws_create`. ]*/
/* Tests_SRS_UWS_01_012: [ - `port` set to the `port` argument passed to `uws_create`. ]*/
/* Tests_SRS_UWS_01_004: [ The argument `hostname` shall be copied for later use. ]*/
/* Tests_SRS_UWS_01_403: [ The argument `port` shall be copied for later use. ]*/
/* Tests_SRS_UWS_01_404: [ The argument `resource_name` shall be copied for later use. ]*/
/* Tests_SRS_UWS_01_422: [ `uws_create` shall create a buffer to be used for encoding outgoing frames by calling `BUFFER_new`. ]*/
TEST_FUNCTION(uws_create_with_valid_args_no_ssl_succeeds)
{
	// arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_HANDLE uws;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

	EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new());
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "111"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

	// act
    uws = uws_create("test_host", 80, "111", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

	// assert
	ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_002: [ If any of the arguments `hostname` and `resource_name` is NULL then `uws_create` shall return NULL. ]*/
TEST_FUNCTION(uws_create_with_NULL_hostname_fails)
{
    // arrange

    // act
    UWS_HANDLE uws = uws_create(NULL, 80, "222", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_002: [ If any of the arguments `hostname` and `resource_name` is NULL then `uws_create` shall return NULL. ]*/
TEST_FUNCTION(uws_create_with_NULL_resource_name_fails)
{
    // arrange

    // act
    UWS_HANDLE uws = uws_create("testhost", 80, NULL, false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_012: [ - `port` set to the `port` argument passed to `uws_create`. ]*/
TEST_FUNCTION(uws_create_with_valid_args_no_ssl_port_different_than_80_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_HANDLE uws;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 81;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new());
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "333"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws = uws_create("test_host", 81, "333", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_410: [ The `protocols` argument shall be allowed to be NULL, in which case no protocol is to be specified by the client in the upgrade request. ]*/
TEST_FUNCTION(uws_create_with_NULL_protocols_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_HANDLE uws;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 81;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new());
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "333"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();

    // act
    uws = uws_create("test_host", 81, "333", false, NULL, 0);

    // assert
    ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_411: [ If `protocol_count` is non zero and `protocols` is NULL then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(uws_create_with_non_zero_protocol_count_and_NULL_protocols_fails)
{
    // arrange
    UWS_HANDLE uws;

    // act
    uws = uws_create("test_host", 81, "333", false, NULL, 1);

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_412: [ If the `protocol` member of any of the items in the `protocols` argument is NULL, then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(uws_create_with_the_first_protocol_name_NULL_fails)
{
    // arrange
    UWS_HANDLE uws;
    WS_PROTOCOL NULL_test_protocol[] = { { NULL } };

    // act
    uws = uws_create("test_host", 81, "333", false, NULL_test_protocol, sizeof(NULL_test_protocol) / sizeof(NULL_test_protocol[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_412: [ If the `protocol` member of any of the items in the `protocols` argument is NULL, then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(uws_create_with_the_second_protocol_name_NULL_fails)
{
    // arrange
    UWS_HANDLE uws;
    WS_PROTOCOL NULL_test_protocol[] = { { "aaa" }, { NULL } };

    // act
    uws = uws_create("test_host", 81, "333", false, NULL_test_protocol, sizeof(NULL_test_protocol) / sizeof(NULL_test_protocol[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_003: [ If allocating memory for the new uws instance fails then `uws_create` shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_new_uws_instance_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_HANDLE uws;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    uws = uws_create("test_host", 80, "aaa", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_423: [ If `BUFFER_new` fails  then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_BUFFER_new_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    UWS_HANDLE uws = uws_create("test_host", 80, "bbb", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_392: [ If allocating memory for the copy of the `hostname` argument fails, then `uws_create` shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_hostname_copy_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    BUFFER_HANDLE buffer_handle;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    UWS_HANDLE uws = uws_create("test_host", 80, "bbb", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_405: [ If allocating memory for the copy of the `resource_name` argument fails, then `uws_create` shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_resource_name_copy_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    BUFFER_HANDLE buffer_handle;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    UWS_HANDLE uws = uws_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_018: [ If `singlylinkedlist_create` fails then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_creating_the_pending_sends_list_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_HANDLE uws;
    BUFFER_HANDLE buffer_handle;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws = uws_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_007: [ If obtaining the underlying IO interface fails, then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_getting_the_socket_interface_description_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    BUFFER_HANDLE buffer_handle;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    UWS_HANDLE uws = uws_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_016: [ If `xio_create` fails, then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_creating_the_io_handle_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_HANDLE uws;
    BUFFER_HANDLE buffer_handle;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters()
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws = uws_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_006: [ If `use_ssl` is 1 then `uws_create` shall obtain the interface used to create a tlsio instance by calling `platform_get_default_tlsio`. ]*/
/* Tests_SRS_UWS_01_013: [ The create arguments for the tls IO (when `use_ssl` is 1) shall have: ]*/
/* Tests_SRS_UWS_01_014: [ - `hostname` set to the `hostname` argument passed to `uws_create`. ]*/
/* Tests_SRS_UWS_01_015: [ - `port` set to the `port` argument passed to `uws_create`. ]*/
TEST_FUNCTION(uws_create_with_valid_args_ssl_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 443;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new());
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/23"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(xio_create(TEST_TLS_IO_INTERFACE_DESCRIPTION, &tlsio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws = uws_create("test_host", 443, "test_resource/23", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_006: [ If `use_ssl` is 1 then `uws_create` shall obtain the interface used to create a tlsio instance by calling `platform_get_default_tlsio`. ]*/
/* Tests_SRS_UWS_01_013: [ The create arguments for the tls IO (when `use_ssl` is 1) shall have: ]*/
/* Tests_SRS_UWS_01_014: [ - `hostname` set to the `hostname` argument passed to `uws_create`. ]*/
/* Tests_SRS_UWS_01_015: [ - `port` set to the `port` argument passed to `uws_create`. ]*/
TEST_FUNCTION(uws_create_with_valid_args_ssl_port_different_than_443_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new());
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/23"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(xio_create(TEST_TLS_IO_INTERFACE_DESCRIPTION, &tlsio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws = uws_create("test_host", 444, "test_resource/23", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_007: [ If obtaining the underlying IO interface fails, then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_getting_the_tlsio_interface_fails_then_uws_create_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/23"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws = uws_create("test_host", 444, "test_resource/23", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* uws_destroy */

/* Tests_SRS_UWS_01_019: [ `uws_destroy` shall free all resources associated with the uws instance. ]*/
/* Tests_SRS_UWS_01_023: [ `uws_destroy` shall destroy the underlying IO created in `uws_create` by calling `xio_destroy`. ]*/
/* Tests_SRS_UWS_01_024: [ `uws_destroy` shall free the list used to track the pending sends by calling `singlylinkedlist_destroy`. ]*/
/* Tests_SRS_UWS_01_424: [ `uws_destroy` shall free the buffer allocated in `uws_create` by calling `BUFFER_delete`. ]*/
TEST_FUNCTION(uws_destroy_fress_the_resources)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_destroy(uws);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_020: [ If `uws` is NULL, `uws_destroy` shall do nothing. ]*/
TEST_FUNCTION(uws_destroy_with_NULL_does_nothing)
{
    // arrange

    // act
    uws_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* uws_open */

/* Tests_SRS_UWS_01_025: [ `uws_open` shall open the underlying IO by calling `xio_open` and providing the IO handle created in `uws_create` as argument. ]*/
/* Tests_SRS_UWS_01_367: [ The callbacks `on_underlying_io_open_complete`, `on_underlying_io_bytes_received` and `on_underlying_io_error` shall be passed as arguments to `xio_open`. ]*/
/* Tests_SRS_UWS_01_026: [ On success, `uws_open` shall return 0. ]*/
TEST_FUNCTION(uws_open_opens_the_underlying_IO)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_027: [ If `uws`, `on_ws_open_complete`, `on_ws_frame_received` or `on_ws_error` is NULL, `uws_open` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_open_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = uws_open(NULL, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_027: [ If `uws`, `on_ws_open_complete`, `on_ws_frame_received` or `on_ws_error` is NULL, `uws_open` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_open_with_NULL_on_ws_open_complete_callback_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_open(uws, NULL, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_027: [ If `uws`, `on_ws_open_complete`, `on_ws_frame_received` or `on_ws_error` is NULL, `uws_open` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_open_with_NULL_on_ws_frame_received_callback_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, NULL, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_027: [ If `uws`, `on_ws_open_complete`, `on_ws_frame_received` or `on_ws_error` is NULL, `uws_open` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_open_with_NULL_on_ws_error_callback_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
TEST_FUNCTION(uws_open_with_NULL_on_ws_open_complete_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, NULL, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
TEST_FUNCTION(uws_open_with_NULL_on_ws_frame_received_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, NULL, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
TEST_FUNCTION(uws_open_with_NULL_on_ws_error_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_028: [ If opening the underlying IO fails then `uws_open` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_opening_the_underlying_io_fails_uws_open_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context()
        .SetReturn(1);

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_394: [ `uws_open` while the uws instance is already OPEN or OPENING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_open_after_uws_open_without_a_close_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_400: [ `uws_open` while CLOSING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_open_while_closing_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    (void)uws_close(uws, test_on_ws_close_complete, NULL);
    umock_c_reset_all_calls();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* uws_close */

/* Tests_SRS_UWS_01_029: [ `uws_close` shall close the uws instance connection if an open action is either pending or has completed successfully (if the IO is open). ]*/
/* Tests_SRS_UWS_01_031: [ `uws_close` shall close the connection by calling `xio_close` while passing as argument the IO handle created in `uws_create`. ]*/
/* Tests_SRS_UWS_01_368: [ The callback `on_underlying_io_close` shall be passed as argument to `xio_close`. ]*/
/* Tests_SRS_UWS_01_396: [ On success `uws_close` shall return 0. ]*/
/* Tests_SRS_UWS_01_399: [ `on_ws_close_complete` and `on_ws_close_complete_context` shall be saved and the callback `on_ws_close_complete` shall be triggered when the close is complete. ]*/
TEST_FUNCTION(uws_close_closes_the_underlying_IO)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();

    // act
    result = uws_close(uws, test_on_ws_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_030: [ if `uws` is NULL, `uws_close` shall return a non-zero value. ]*/
TEST_FUNCTION(uws_close_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = uws_close(NULL, test_on_ws_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_397: [ The `on_ws_close_complete` argument shall be allowed to be NULL, in which case no callback shall be called when the close is complete. ]*/
TEST_FUNCTION(uws_close_with_NULL_close_complete_callback_is_allowed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();

    // act
    result = uws_close(uws, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_398: [ `on_ws_close_complete_context` shall also be allows to be NULL. ]*/
TEST_FUNCTION(uws_close_with_NULL_close_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();

    // act
    result = uws_close(uws, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_395: [ If `xio_close` fails, `uws_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_xio_close_fails_then_uws_close_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context()
        .SetReturn(1);

    // act
    result = uws_close(uws, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_032: [ `uws_close` when no open action has been issued shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_close_without_open_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_close(uws, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_033: [ `uws_close` after a `uws_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_close_while_closing_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    (void)uws_close(uws, test_on_ws_close_complete, NULL);
    umock_c_reset_all_calls();

    // act
    result = uws_close(uws, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_033: [ `uws_close` after a `uws_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_close_after_close_complete_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    (void)uws_close(uws, test_on_ws_close_complete, NULL);
    g_on_io_close_complete(g_on_io_close_complete_context);
    umock_c_reset_all_calls();

    // act
    result = uws_close(uws, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* on_underlying_io_open_complete */

/* Tests_SRS_UWS_01_369: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_ERROR` while uws is OPENING (`uws_open` was called), uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED`. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_ERROR_triggers_the_ws_open_complete_callback_with_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_409: [ After any error is indicated by `on_ws_open_complete`, a subsequent `uws_open` shall be possible. ]*/
TEST_FUNCTION(uws_open_after_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_ERROR);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_401: [ If `on_underlying_io_open_complete` is called with a NULL context, `on_underlying_io_open_complete` shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_io_open_complete(NULL, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_402: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_CANCELLED` while uws is OPENING (`uws_open` was called), uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED`. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_CANCELLED_triggers_the_ws_open_complete_callback_with_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_409: [ After any error is indicated by `on_ws_open_complete`, a subsequent `uws_open` shall be possible. ]*/
TEST_FUNCTION(uws_open_after_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_CANCELLED);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_371: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_OK` while uws is OPENING (`uws_open` was called), uws shall prepare the WebSockets upgrade request. ]*/
/* Tests_SRS_UWS_01_372: [ Once prepared the WebSocket upgrade request shall be sent by calling `xio_send`. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_OK_prepares_and_sends_the_WebSocket_upgrade_request)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .IgnoreArgument_buffer()
        .IgnoreArgument_size();
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_406: [ If not enough memory can be allocated to construct the WebSocket upgrade request, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_NOT_ENOUGH_MEMORY`. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_websocket_upgrade_request_fails_the_error_WS_OPEN_ERROR_NOT_ENOUGH_MEMORY_is_indicated_via_the_open_complete_callback)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_409: [ After any error is indicated by `on_ws_open_complete`, a subsequent `uws_open` shall be possible. ]*/
TEST_FUNCTION(uws_open_after_WS_OPEN_ERROR_NOT_ENOUGH_MEMORY_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));

    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_373: [ If `xio_send` fails then uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST`. ]*/
TEST_FUNCTION(when_sending_the_upgrade_request_fails_the_error_WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .IgnoreArgument_buffer()
        .IgnoreArgument_size()
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_409: [ After any error is indicated by `on_ws_open_complete`, a subsequent `uws_open` shall be possible. ]*/
TEST_FUNCTION(uws_open_after_WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .IgnoreArgument_buffer()
        .IgnoreArgument_size()
        .SetReturn(1);

    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_407: [ When `on_underlying_io_open_complete` is called when the uws instance has send the upgrade request but it is waiting for the response, an error shall be reported to the user by calling the `on_ws_open_complete` with `WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS`. ]*/
TEST_FUNCTION(when_sending_the_upgrade_request_fails_the_error_WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_409: [ After any error is indicated by `on_ws_open_complete`, a subsequent `uws_open` shall be possible. ]*/
TEST_FUNCTION(uws_open_after_WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS));
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* on_underlying_io_bytes_received */

/* Tests_SRS_UWS_01_378: [ When `on_underlying_io_bytes_received` is called while the uws is OPENING, the received bytes shall be accumulated in order to attempt parsing the WebSocket Upgrade response. ]*/
/* Tests_SRS_UWS_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
/* Tests_SRS_UWS_01_381: [ If the status is 101, uws shall be considered OPEN and this shall be indicated by calling the `on_ws_open_complete` callback passed to `uws_open` with `IO_OPEN_OK`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_full_reply_after_the_upgrade_request_was_sent_indicates_open_complete)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_379: [ If allocating memory for accumulating the bytes fails, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_NOT_ENOUGH_MEMORY`. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_received_bytes_fails_on_underlying_io_bytes_received_indicates_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_378: [ When `on_underlying_io_bytes_received` is called while the uws is OPENING, the received bytes shall be accumulated in order to attempt parsing the WebSocket Upgrade response. ]*/
/* Tests_SRS_UWS_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
TEST_FUNCTION(when_only_a_byte_is_received_no_open_complete_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "H";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_415: [ If called with a NULL `context` argument, `on_underlying_io_bytes_received` shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_bytes_received(NULL, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_416: [ If called with NULL `buffer` or zero `size` and the state of the iws is OPENING, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_NULL_buffer_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_bytes_received(g_on_bytes_received_context, NULL, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_416: [ If called with NULL `buffer` or zero `size` and the state of the iws is OPENING, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_zero_size_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_417: [ When `on_underlying_io_bytes_received` is called while OPENING but before the `on_underlying_io_open_complete` has been called, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_before_underlying_io_open_complete_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_379: [ If allocating memory for accumulating the bytes fails, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_NOT_ENOUGH_MEMORY`. ]*/
TEST_FUNCTION(when_allocating_memory_for_a_second_byte_fails_open_complete_is_indicated_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response + 1, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

void when_only_n_bytes_are_received_from_the_response_no_open_complete_is_indicated(const char* test_upgrade_response, size_t n)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    char temp_str[32];

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, n);

    // assert
    (void)sprintf(temp_str, "Bytes = %u", (unsigned int)n);
    ASSERT_ARE_EQUAL_WITH_MSG(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls(), temp_str);

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
TEST_FUNCTION(when_all_but_1_bytes_are_received_from_the_response_no_open_complete_is_indicated)
{
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r";
    size_t i;

    for (i = 1; i < sizeof(test_upgrade_response); i++)
    {
        when_only_n_bytes_are_received_from_the_response_no_open_complete_is_indicated(test_upgrade_response, i);
    }
}

/* Tests_SRS_UWS_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames by passing them to `uws_frame_decoder_decode`. ]*/
TEST_FUNCTION(when_1_extra_byte_is_received_the_open_complete_is_properly_indicated_and_the_extra_byte_is_saved_for_decoding_frames)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n\0";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_386: [ When a WebSocket data frame is decoded succesfully it shall be indicated via the callback `on_ws_frame_received`. ]*/
/* Tests_SRS_UWS_01_385: [ If the state of the uws instance is OPEN, the received bytes shall be used for decoding WebSocket frames. ]*/
/* Tests_SRS_UWS_01_154: [ *  %x2 denotes a binary frame ]*/
/* Tests_SRS_UWS_01_163: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_01_164: [ if 0-125, that is the payload length. ]*/
/* Tests_SRS_UWS_01_169: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
/* Tests_SRS_UWS_01_173: [ The "Payload data" is defined as "Extension data" concatenated with "Application data". ]*/
/* Tests_SRS_UWS_01_147: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_01_148: [ The first fragment MAY also be the final fragment. ]*/
TEST_FUNCTION(when_a_1_byte_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = { 0x82, 0x01, 0x42 };
    const unsigned char expected_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(3, expected_payload, sizeof(expected_payload));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_153: [ *  %x1 denotes a text frame ]*/
TEST_FUNCTION(when_a_1_byte_text_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = { 0x81, 0x01, 0x42 };
    const unsigned char expected_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_TEXT, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(3, expected_payload, sizeof(expected_payload));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_163: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_01_164: [ if 0-125, that is the payload length. ]*/
TEST_FUNCTION(when_a_0_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = { 0x82, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_163: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_01_164: [ if 0-125, that is the payload length. ]*/
TEST_FUNCTION(when_a_125_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[125 + 2] = { 0x82, 0x7D };
    size_t i;

    for (i = 0; i < 125; i++)
    {
        test_frame[2 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 125))
        .ValidateArgumentBuffer(3, &test_frame[2], 125);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_165: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_126_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[126 + 4] = { 0x82, 0x7E, 0x00, 0x7E };
    size_t i;

    for (i = 0; i < 126; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 126))
        .ValidateArgumentBuffer(3, &test_frame[4], 126);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_165: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_127_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[127 + 4] = { 0x82, 0x7E, 0x00, 0x7F };
    size_t i;

    for (i = 0; i < 127; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 127))
        .ValidateArgumentBuffer(3, &test_frame[4], 127);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_165: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_65535_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65535 + 4);
    test_frame[0] = 0x82;
    test_frame[1] = 0x7E;
    test_frame[2] = 0xFF;
    test_frame[3] = 0xFF;
    size_t i;

    for (i = 0; i < 65535; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 65535))
        .ValidateArgumentBuffer(3, &test_frame[4], 65535);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65535 + 4);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(test_frame);
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_166: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
/* Tests_SRS_UWS_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_65536_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65536 + 10);
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x00;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x01;
    test_frame[8] = 0x00;
    test_frame[9] = 0x00;
    size_t i;

    for (i = 0; i < 65536; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 65536))
        .ValidateArgumentBuffer(3, &test_frame[10], 65536);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65536 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(test_frame);
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_166: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
/* Tests_SRS_UWS_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_65537_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65537 + 10);
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x00;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x01;
    test_frame[8] = 0x00;
    test_frame[9] = 0x01;
    size_t i;

    for (i = 0; i < 65537; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 65537))
        .ValidateArgumentBuffer(3, &test_frame[10], 65537);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65537 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(test_frame);
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
TEST_FUNCTION(when_a_0_byte_binary_frame_is_received_with_16_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7E, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
TEST_FUNCTION(when_a_125_byte_binary_frame_is_received_with_16_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[125 + 4] = { 0x82, 0x7E, 0x00, 0x7D };
    size_t i;

    for (i = 0; i < 125; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
TEST_FUNCTION(when_a_0_byte_binary_frame_is_received_with_64_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
TEST_FUNCTION(when_a_65535_byte_binary_frame_is_received_with_64_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65535 + 10);
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x00;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x00;
    test_frame[8] = 0xFF;
    test_frame[9] = 0xFF;
    size_t i;

    for (i = 0; i < 65535; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65535 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
TEST_FUNCTION(check_for_16_bit_length_too_low_is_done_as_soon_as_length_is_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7E, 0x00, 0x7D };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
TEST_FUNCTION(check_for_64_bit_length_too_low_is_done_as_soon_as_length_is_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_166: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
/* Tests_SRS_UWS_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_BAD_FRAME_RECEIVED`. ]*/
TEST_FUNCTION(when_the_highest_bit_is_set_in_a_64_bit_length_frame_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65536 + 10);
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x80;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x01;
    test_frame[8] = 0x00;
    test_frame[9] = 0x00;
    size_t i;

    for (i = 0; i < 65536; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65536 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_418: [ If allocating memory for the bytes accumulated for decoding WebSocket frames fails, an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_NOT_ENOUGH_MEMORY`. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_received_frame_bytes_fails_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_NOT_ENOUGH_MEMORY));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
TEST_FUNCTION(when_1_byte_is_received_together_with_the_upgrade_request_and_one_byte_with_a_separate_call_decoding_frame_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* upgrade_response_frame = (unsigned char*)malloc(sizeof(test_upgrade_response));
    unsigned char test_frame[] = { 0x00 };

    (void)memcpy(upgrade_response_frame, test_upgrade_response, sizeof(test_upgrade_response) - 1);
    upgrade_response_frame[sizeof(test_upgrade_response) - 1] = 0x82;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)upgrade_response_frame, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
TEST_FUNCTION(when_a_complete_frame_is_received_together_with_the_upgrade_request_the_frame_is_indicated_as_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    size_t received_data_length = sizeof(test_upgrade_response) + 1;
    unsigned char* received_data = (unsigned char*)malloc(received_data_length);

    (void)memcpy(received_data, test_upgrade_response, sizeof(test_upgrade_response) - 1);
    received_data[received_data_length - 2] = 0x82;
    received_data[received_data_length - 1] = 0x00;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)received_data, received_data_length);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
TEST_FUNCTION(when_2_complete_frames_are_received_together_with_the_upgrade_request_the_frames_are_indicated_as_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    size_t received_data_length = sizeof(test_upgrade_response) + 4;
    unsigned char* received_data = (unsigned char*)malloc(received_data_length);

    (void)memcpy(received_data, test_upgrade_response, sizeof(test_upgrade_response) - 1);
    received_data[received_data_length - 5] = 0x81;
    received_data[received_data_length - 4] = 0x01;
    received_data[received_data_length - 3] = 'a';
    received_data[received_data_length - 2] = 0x82;
    received_data[received_data_length - 1] = 0x00;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_TEXT, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(3, "a", 1);
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)received_data, received_data_length);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_144: [ A client MUST close a connection if it detects a masked frame. ]*/
/* Tests_SRS_UWS_01_145: [ In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) ]*/
/* Tests_SRS_UWS_01_160: [ Defines whether the "Payload data" is masked. ]*/
TEST_FUNCTION(when_a_masked_frame_is_received_an_error_is_indicated_and_connection_is_closed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x80 };
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(buffer_handle, WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(3, close_frame_payload, sizeof(close_frame_payload));
    STRICT_EXPECTED_CALL(BUFFER_u_char(buffer_handle)).SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(buffer_handle)).SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), NULL, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_144: [ A client MUST close a connection if it detects a masked frame. ]*/
/* Tests_SRS_UWS_01_145: [ In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) ]*/
/* Tests_SRS_UWS_01_160: [ Defines whether the "Payload data" is masked. ]*/
TEST_FUNCTION(when_a_masked_frame_is_received_and_encoding_the_close_frame_fails_an_error_is_indicated_anyhow)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x80 };
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(buffer_handle, WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(3, close_frame_payload, sizeof(close_frame_payload))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_144: [ A client MUST close a connection if it detects a masked frame. ]*/
/* Tests_SRS_UWS_01_145: [ In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) ]*/
/* Tests_SRS_UWS_01_160: [ Defines whether the "Payload data" is masked. ]*/
TEST_FUNCTION(when_a_masked_frame_is_received_and_sending_the_encoded_CLOSE_frame_fails_an_error_is_indicated_anyhow)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x80 };
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(buffer_handle, WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(3, close_frame_payload, sizeof(close_frame_payload));
    STRICT_EXPECTED_CALL(BUFFER_u_char(buffer_handle)).SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(buffer_handle)).SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), NULL, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* uws_send_frame */

/* Tests_SRS_UWS_01_044: [ If any the arguments `uws` is NULL, `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_send_frame_with_NULL_handle_fails)
{
    // arrange
    unsigned char test_payload[] = { 0x42 };
    int result;

    // act
    result = uws_send_frame(NULL, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_045: [ If `size` is non-zero and `buffer` is NULL then `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_send_frame_with_NULL_buffer_and_non_zero_size_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    // act
    result = uws_send_frame(uws, NULL, 1, true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_043: [ If the uws instance is not OPEN (open has not been called or is still in progress) then `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_send_frame_when_not_open_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_043: [ If the uws instance is not OPEN (open has not been called or is still in progress) then `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_send_frame_when_opening_underlying_io_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_043: [ If the uws instance is not OPEN (open has not been called or is still in progress) then `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_send_frame_when_waiting_for_upgrade_response_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    int result;
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_038: [ `uws_send_frame` shall create and queue a structure that contains: ]*/
/* Tests_SRS_UWS_01_039: [ - the encoded websocket frame, so that the frame can be later sent when `uws_dowork` is called ]*/
/* Tests_SRS_UWS_01_040: [ - the send complete callback `on_ws_send_frame_complete` ]*/
/* Tests_SRS_UWS_01_056: [ - the `send_complete` callback shall be the `on_underlying_io_send_complete` function. ]*/
/* Tests_SRS_UWS_01_042: [ On success, `uws_send_frame` shall return 0. ]*/
/* Tests_SRS_UWS_01_425: [ Encoding shall be done by calling `uws_frame_encoder_encode` and passing to it the `buffer` and `size` argument for payload, the `is_final` flag and setting `is_masked` to true. ]*/
/* Tests_SRS_UWS_01_427: [ The encoded frame buffer that shall be used is the buffer created in `uws_create`. ]*/
/* Tests_SRS_UWS_01_428: [ The encoded frame buffer memory shall be obtained by calling `BUFFER_u_char` on the encode buffer. ]*/
/* Tests_SRS_UWS_01_429: [ The encoded frame size shall be obtained by calling `BUFFER_length` on the encode buffer. ]*/
/* Tests_SRS_UWS_01_048: [ Queueing shall be done by calling `singlylinkedlist_add`. ]*/
/* Tests_SRS_UWS_01_038: [ `uws_send_frame` shall create and queue a structure that contains: ]*/
/* Tests_SRS_UWS_01_040: [ - the send complete callback `on_ws_send_frame_complete` ]*/
/* Tests_SRS_UWS_01_041: [ - the send complete callback context `on_ws_send_frame_complete_context` ]*/
TEST_FUNCTION(uws_send_frame_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(IGNORED_PTR_ARG, WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .ValidateArgumentValue_encode_buffer(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item();
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame));

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_047: [ If allocating memory for the newly queued item fails, `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_new_sent_item_fails_uws_send_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_426: [ If `uws_frame_encoder_encode` fails, `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_encoding_the_frame_fails_uws_send_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(IGNORED_PTR_ARG, WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .ValidateArgumentValue_encode_buffer(&buffer_handle)
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_058: [ If `xio_send` fails, `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_xio_send_fails_uws_send_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;
    LIST_ITEM_HANDLE new_item_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(IGNORED_PTR_ARG, WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .ValidateArgumentValue_encode_buffer(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item()
        .CaptureReturn(&new_item_handle);
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .ValidateArgumentValue_item_handle(&new_item_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_049: [ If `singlylinkedlist_add` fails, `uws_send_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_adding_the_item_to_the_list_fails_uws_send_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(IGNORED_PTR_ARG, WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .ValidateArgumentValue_encode_buffer(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item()
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_050: [ The argument `on_ws_send_frame_complete` shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered. ]*/
TEST_FUNCTION(uws_send_frame_with_NULL_complete_callback_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(IGNORED_PTR_ARG, WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .ValidateArgumentValue_encode_buffer(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item();
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame));

    // act
    result = uws_send_frame(uws, test_payload, sizeof(test_payload), true, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* on_underlying_io_send_complete */

/* Tests_SRS_UWS_01_389: [ When `on_underlying_io_send_complete` is called with `IO_SEND_OK` as a result of sending a WebSocket frame to the underlying IO, the send shall be indicated to the uws user by calling `on_ws_send_frame_complete` with `WS_SEND_FRAME_OK`. ]*/
/* Tests_SRS_UWS_01_432: [ The indicated sent frame shall be removed from the list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_UWS_01_434: [ The memory associated with the sent frame shall be freed. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_OK_indicates_the_frame_as_sent_OK)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4245, WS_SEND_FRAME_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_433: [ If `singlylinkedlist_remove` fails an error shall be indicated by calling the `on_ws_error` callback with `WS_ERROR_CANNOT_REMOVE_SENT_ITEM_FROM_LIST`. ]*/
TEST_FUNCTION(when_removing_the_sent_framefrom_the_list_fails_then_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle()
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_CANNOT_REMOVE_SENT_ITEM_FROM_LIST));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_390: [ When `on_underlying_io_send_complete` is called with `IO_SEND_ERROR` as a result of sending a WebSocket frame to the underlying IO, the send shall be indicated to the uws user by calling `on_ws_send_frame_complete` with `WS_SEND_FRAME_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_ERROR_indicates_the_frame_with_WS_SEND_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4245, WS_SEND_FRAME_ERROR));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_391: [ When `on_underlying_io_send_complete` is called with `IO_SEND_CANCELLED` as a result of sending a WebSocket frame to the underlying IO, the send shall be indicated to the uws user by calling `on_ws_send_frame_complete` with `WS_SEND_FRAME_CANCELLED`. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_CANCELLED_indicates_the_frame_with_WS_SEND_CANCELLED)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4245, WS_SEND_FRAME_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_435: [ When `on_underlying_io_send_complete` is called with a NULL `context`, it shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_io_send_complete(NULL, IO_SEND_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_436: [ When `on_underlying_io_send_complete` is called with any other error code, it shall indicate an error by calling the `on_ws_error` callback with `WS_ERROR_INVALID_IO_SEND_RESULT`. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_an_unknown_result_indicates_an_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    uws_send_frame(uws, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_INVALID_IO_SEND_RESULT));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, 0x42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* uws_dowork */

/* Tests_SRS_UWS_01_059: [ If the `uws` argument is NULL, `uws_dowork` shall do nothing. ]*/
TEST_FUNCTION(uws_dowork_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    uws_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_430: [ `uws_dowork` shall call `xio_dowork` with the IO handle argument set to the underlying IO created in `uws_create`. ]*/
TEST_FUNCTION(uws_dowork_calls_the_underlying_io_dowork)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_dowork(TEST_IO_HANDLE));

    // act
    uws_dowork(uws);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_060: [ If the IO is not yet open, `uws_dowork` shall do nothing. ]*/
TEST_FUNCTION(uws_dowork_when_closed_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    uws_dowork(uws);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* on_underlying_io_error */

/* Tests_SRS_UWS_01_375: [ When `on_underlying_io_error` is called while uws is OPENING, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_UNDERLYING_IO_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_error_while_opening_underlying_io_indicates_an_open_complete_with_WS_OPEN_ERROR_UNDERLYING_IO_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_ERROR));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_375: [ When `on_underlying_io_error` is called while uws is OPENING, uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_ERROR_UNDERLYING_IO_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_error_while_waiting_for_upgrade_response_indicates_an_open_complete_with_WS_OPEN_ERROR_UNDERLYING_IO_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_ERROR));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_376: [ When `on_underlying_io_error` is called while the uws instance is OPEN, an error shall be reported to the user by calling the `on_ws_error` callback that was passed to `uws_open` with the argument `WS_ERROR_UNDERLYING_IO_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_error_while_OPEN_indicates_an_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_UNDERLYING_IO_ERROR));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

END_TEST_SUITE(uws_ut)
