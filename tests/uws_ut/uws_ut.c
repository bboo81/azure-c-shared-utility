// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"

/* TODO:
- Implement a feature in umock_c that can say "compare this argument as a given type"
*/

#define ENABLE_MOCKS

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tlsio.h"

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

TEST_DEFINE_ENUM_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);

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
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_frame_received, void*, context, const unsigned char*, buffer, size_t, size);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_error, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_close_complete, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_send_frame_complete, void*, context, WS_SEND_FRAME_RESULT, ws_send_frame_result)
MOCK_FUNCTION_END()

static ON_IO_OPEN_COMPLETE g_on_io_open_complete;
static void* g_on_io_open_complete_context;
static ON_BYTES_RECEIVED g_on_bytes_received;
static void* g_on_bytes_received_context;
static ON_IO_ERROR g_on_io_error;
static void* g_on_io_error_context;

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

static ON_IO_CLOSE_COMPLETE g_on_io_close_complete;
static void* g_on_io_close_complete_context;

static int my_xio_close(XIO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    (void)xio;
    g_on_io_close_complete = on_io_close_complete;
    g_on_io_close_complete_context = callback_context;
    return 0;
}

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static const IO_INTERFACE_DESCRIPTION* TEST_SOCKET_IO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x4542;
static const IO_INTERFACE_DESCRIPTION* TEST_TLS_IO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x4543;

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
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, my_xio_close);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_SOCKET_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(platform_get_default_tlsio, TEST_TLS_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
    REGISTER_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT);
    REGISTER_TYPE(const SOCKETIO_CONFIG*, const_SOCKETIO_CONFIG_ptr);

    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(UWS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
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
TEST_FUNCTION(uws_create_with_valid_args_no_ssl_succeeds)
{
	// arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_HANDLE uws;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

	EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();

	// act
    uws = uws_create("test_host", 80, false);

	// assert
	ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_002: [ If the argument `hostname` is NULL then `uws_create` shall return NULL. ]*/
TEST_FUNCTION(uws_create_with_NULL_hostname_fails)
{
    // arrange

    // act
    UWS_HANDLE uws = uws_create(NULL, 80, false);

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
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();

    // act
    uws = uws_create("test_host", 81, false);

    // assert
    ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
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
    uws = uws_create("test_host", 80, false);

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_392: [ If allocating memory for the copy of the hostname argument fails, then `uws_create` shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_hostname_copy_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    UWS_HANDLE uws = uws_create("test_host", 80, false);

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

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws = uws_create("test_host", 80, false);

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_01_007: [ If obtaining the underlying IO interface fails, then `uws_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_getting_the_socket_interface_description_fails_then_uws_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    UWS_HANDLE uws = uws_create("test_host", 80, false);

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

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters()
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws = uws_create("test_host", 80, false);

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
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(xio_create(TEST_TLS_IO_INTERFACE_DESCRIPTION, &tlsio_config))
        .IgnoreArgument_io_create_parameters();

    // act
    uws = uws_create("test_host", 443, true);

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
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(xio_create(TEST_TLS_IO_INTERFACE_DESCRIPTION, &tlsio_config))
        .IgnoreArgument_io_create_parameters();

    // act
    uws = uws_create("test_host", 444, true);

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

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws = uws_create("test_host", 444, true);

    // assert
    ASSERT_IS_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* uws_destroy */

/* Tests_SRS_UWS_01_019: [ `uws_destroy` shall free all resources associated with the uws instance. ]*/
/* Tests_SRS_UWS_01_023: [ `uws_destroy` shall destroy the underlying IO created in `uws_create` by calling `xio_destroy`. ]*/
/* Tests_SRS_UWS_01_024: [ `uws_destroy` shall free the list used to track the pending sends by calling `singlylinkedlist_destroy`. ]*/
TEST_FUNCTION(uws_destroy_fress_the_resources)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, true);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

    uws = uws_create("test_host", 444, true);
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

/* Tests_SRS_UWS_01_369: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_ERROR` while uws is OPENING (`uws_open` was called), uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_UNDERLYING_IO_OPEN_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_ERROR_triggers_the_ws_open_complete_callback_with_WS_OPEN_UNDERLYING_IO_OPEN_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, true);
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_UNDERLYING_IO_OPEN_ERROR));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_ERROR);

    // assert
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

    uws = uws_create("test_host", 444, true);
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_io_open_complete(NULL, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_402: [ When `on_underlying_io_open_complete` is called with `IO_OPEN_CANCELLED` while uws is OPENING (`uws_open` was called), uws shall report that the open failed by calling the `on_ws_open_complete` callback passed to `uws_open` with `WS_OPEN_UNDERLYING_IO_OPEN_CANCELLED_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_CANCELLED_triggers_the_ws_open_complete_callback_with_WS_OPEN_UNDERLYING_IO_OPEN_CANCELLED_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, true);
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_UNDERLYING_IO_OPEN_CANCELLED_ERROR));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

END_TEST_SUITE(uws_ut)
