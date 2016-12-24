// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "umock_c.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/uws_frame_encoder.h"

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

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/uws_frame_encoder.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

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
    REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_SOCKET_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(platform_get_default_tlsio, TEST_TLS_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(uws_frame_decoder_create, TEST_UWS_FRAME_DECODER);
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
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(UWS_FRAME_DECODER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_FRAME_DECODED, void*);
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

/* Tests_SRS_UWS_01_392: [ If allocating memory for the copy of the `hostname` argument fails, then `uws_create` shall return NULL. ]*/
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
    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
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

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
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
    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
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

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
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

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
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
TEST_FUNCTION(when_1_extra_byte_is_received_the_open_complete_is_properly_indicated_and_the_extra_byte_is_passed_to_frame_decoder)
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
    STRICT_EXPECTED_CALL(uws_frame_decoder_decode(TEST_UWS_FRAME_DECODER, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(2, test_upgrade_response + sizeof(test_upgrade_response) - 1, 1);

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

/* Tests_SRS_UWS_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames by passing them to `uws_frame_decoder_decode`. ]*/
TEST_FUNCTION(when_2_extra_byte_is_received_the_open_complete_is_properly_indicated_and_the_extra_byte_is_passed_to_frame_decoder)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_HANDLE uws;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\nAB";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws = uws_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_open(uws, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));
    STRICT_EXPECTED_CALL(uws_frame_decoder_decode(TEST_UWS_FRAME_DECODER, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(2, test_upgrade_response + sizeof(test_upgrade_response) - 2, 2);

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

END_TEST_SUITE(uws_ut)