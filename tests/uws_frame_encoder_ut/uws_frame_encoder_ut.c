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
#include "azure_c_shared_utility/gb_rand.h"
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
    result = uws_frame_encoder_encode(NULL, WS_TEXT_FRAME, test_payload, sizeof(test_payload), false, true, 0);

	// assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_001: [ `uws_frame_encoder_encode` shall encode the information given in `opcode`, `payload`, `length`, `is_masked`, `is_final` and `reserved` according to the RFC6455 into the `encode_buffer` argument.]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_044: [ On success `uws_frame_encoder_encode` shall return 0. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_048: [ The buffer `encode_buffer` shall be reset by calling `BUFFER_unbuild`. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_046: [ The buffer `encode_buffer` shall be resized accordingly using `BUFFER_enlarge`. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_050: [ The allocated memory shall be accessed by calling `BUFFER_u_char`. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_002: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_003: [ The first fragment MAY also be the final fragment. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_015: [ Defines whether the "Payload data" is masked. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x82, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_049: [ If `BUFFER_unbuild` fails then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_BUFFER_unbuild_fails_then_uws_frame_encoder_encode_fails)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer))
        .SetReturn(1);

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_047: [ If `BUFFER_enlarge` fails then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_BUFFER_enlarge_fails_then_uws_frame_encoder_encode_fails)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2))
        .SetReturn(1);

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_051: [ If `BUFFER_u_char` fails then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_BUFFER_u_char_fails_then_uws_frame_encoder_encode_fails)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer))
        .SetReturn(NULL);

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_002: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_003: [ The first fragment MAY also be the final fragment. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_that_is_not_final)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x02, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, false, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_reserved_bits_set)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0xF2, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, 7);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_052: [ If `reserved` has any bits set except the lowest 3 then `uws_frame_encoder_encode` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_reserved_bits_having_all_bits_set_fails)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, 0xFF);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_RSV1_set)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0xC2, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, RESERVED_1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_RSV2_set)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0xA2, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, RESERVED_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_RSV3_set)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x92, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, RESERVED_3);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_006: [ If an unknown opcode is received, the receiving endpoint MUST _Fail the WebSocket Connection_. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_with_opcode_16_fails)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();

    // act
    result = uws_frame_encoder_encode(encode_buffer, 0x10, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_007: [ *  %x0 denotes a continuation frame ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_continuation_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x80, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_CONTINUATION_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_008: [ *  %x1 denotes a text frame ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_text_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x81, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_TEXT_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_009: [ *  %x2 denotes a binary frame ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_binary_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x82, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_3)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x83, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_NON_CONTROL_FRAME_3, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_4)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x84, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_NON_CONTROL_FRAME_4, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_5)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x85, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_NON_CONTROL_FRAME_5, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_6)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x86, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_NON_CONTROL_FRAME_6, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_7)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x87, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_NON_CONTROL_FRAME_7, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_011: [ *  %x8 denotes a connection close ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_close_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x88, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_CLOSE_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_012: [ *  %x9 denotes a ping ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_ping_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x89, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_PING_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_013: [ *  %xA denotes a pong ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_pong_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x8A, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_PONG_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_B)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x8B, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_CONTROL_FRAME_B, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_C)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x8C, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_CONTROL_FRAME_C, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_D)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x8D, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_CONTROL_FRAME_D, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_E)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x8E, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_CONTROL_FRAME_E, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_F)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x8F, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, 2));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_RESERVED_CONTROL_FRAME_F, NULL, 0, false, true, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str);
    stringify_bytes(real_BUFFER_u_char(encode_buffer), real_BUFFER_length(encode_buffer), actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(encode_buffer);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_015: [ Defines whether the "Payload data" is masked. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_053: [ In order to obtain a 32 bit value for masking, `gb_rand` shall be used 4 times (for each byte). ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_masked_zero_length_binary_frame)
{
    // arrange
    int result;
    BUFFER_HANDLE encode_buffer = real_BUFFER_new();
    unsigned char expected_bytes[] = { 0x82, 0x80, 0xFF, 0xFF, 0xFF, 0xFF };

    STRICT_EXPECTED_CALL(BUFFER_unbuild(encode_buffer));
    STRICT_EXPECTED_CALL(BUFFER_enlarge(encode_buffer, sizeof(expected_bytes)));
    STRICT_EXPECTED_CALL(BUFFER_u_char(encode_buffer));
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);

    // act
    result = uws_frame_encoder_encode(encode_buffer, WS_BINARY_FRAME, NULL, 0, true, true, 0);

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
