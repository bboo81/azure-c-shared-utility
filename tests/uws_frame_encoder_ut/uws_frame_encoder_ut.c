// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c.h"

#define ENABLE_MOCKS

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

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/buffer_.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/uws_frame_encoder.h"

extern BUFFER_HANDLE real_BUFFER_new(void);
extern void real_BUFFER_delete(BUFFER_HANDLE handle);
extern int real_BUFFER_enlarge(BUFFER_HANDLE handle, size_t enlargeSize);
extern int real_BUFFER_size(BUFFER_HANDLE handle, size_t* size);
extern int real_BUFFER_content(BUFFER_HANDLE handle, const unsigned char** content);
extern unsigned char* real_BUFFER_u_char(BUFFER_HANDLE handle);
extern size_t real_BUFFER_length(BUFFER_HANDLE handle);

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static char expected_encoded_str[256];
static char actual_encoded_str[256];

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static void stringify_bytes(const unsigned char* bytes, size_t byte_count, char* output_string)
{
    size_t i;
    size_t pos = 0;

    output_string[pos++] = '[';
    for (i = 0; i < byte_count; i++)
    {
        (void)sprintf(&output_string[pos], "0x%02X", bytes[i]);
        if (i < byte_count - 1)
        {
            strcat(output_string, ",");
        }
        pos = strlen(output_string);
    }
    output_string[pos++] = ']';
    output_string[pos++] = '\0';
}

BEGIN_TEST_SUITE(uws_frame_encoder_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_u_char, real_BUFFER_u_char);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_enlarge, real_BUFFER_enlarge);
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
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* uws_frame_encoder_encode */

/* Tests_SRS_UWS_FRAME_ENCODER_01_045: [ If the argument `encode_buffer` is NULL then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_with_NULL_buffer_fails)
{
	// arrange
    int result;
    unsigned char test_payload[] = { 0x42 };

	// act
    result = uws_frame_encoder_encode(NULL, 0x01, test_payload, sizeof(test_payload), false, true, 0);

	// assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_001: [ `uws_frame_encoder_encode` shall encode the information given in `opcode`, `payload`, `length`, `is_masked`, `is_final` and `reserved` according to the RFC6455 into the `encode_buffer` argument.]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_044: [ On success `uws_frame_encoder_encode` shall return 0. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_046: [ The buffer `encode_buffer` shall be resized accordingly using `BUFFER_resize`. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x82, 0x00 };

    // act
    result = uws_frame_encoder_encode(encode_buffer, 0x02, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

END_TEST_SUITE(uws_frame_encoder_ut)
