// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/singlylinkedlist.h"

TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);

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

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/uws.h"

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
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);

    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
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
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* uws_create */

TEST_FUNCTION(uws_create_with_valid_args_no_ssl_succeeds)
{
	// arrange
	EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

	// act
    CONCRETE_IO_HANDLE uws = uws_create("test_host", 443, false);

	// assert
	ASSERT_IS_NOT_NULL(uws);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_destroy(uws);
}

END_TEST_SUITE(uws_ut)
