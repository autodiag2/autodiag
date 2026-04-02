#include "libTest.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t uint8;

double ad_expr_reduce(const uint8 *bytes, int sz, const char *expr, char **errorReturn);

static void expect_ok_double(const uint8 *bytes, int sz, const char *expr, double expected) {
    char *err = NULL;
    double v = ad_expr_reduce(bytes, sz, expr, &err);
    assert(err == NULL);
    assert(!isnan(v));
    assert(fabs(v - expected) < 1e-9);
}

static void expect_ok_int(const uint8 *bytes, int sz, const char *expr, long long expected) {
    char *err = NULL;
    double v = ad_expr_reduce(bytes, sz, expr, &err);
    assert(err == NULL);
    assert(!isnan(v));
    assert((long long)v == expected);
}

static void expect_error_contains(const uint8 *bytes, int sz, const char *expr, const char *needle) {
    char *err = NULL;
    double v = ad_expr_reduce(bytes, sz, expr, &err);
    assert(isnan(v));
    assert(err != NULL);
    assert(strstr(err, needle) != NULL);
    free(err);
}

static void test_basic_arithmetic_literals(void) {
    uint8 bytes[] = {0};

    expect_ok_double(bytes, 1, "1", 1.0);
    expect_ok_double(bytes, 1, "1 + 2", 3.0);
    expect_ok_double(bytes, 1, "1 + 2 * 3", 7.0);
    expect_ok_double(bytes, 1, "(1 + 2) * 3", 9.0);
    expect_ok_double(bytes, 1, "10 / 4", 2.5);
    expect_ok_int(bytes, 1, "10 % 4", 2);
    expect_ok_double(bytes, 1, "-5 + 2", -3.0);
    expect_ok_double(bytes, 1, "+5", 5.0);
    expect_ok_double(bytes, 1, "  2  +   3  ", 5.0);
}

static void test_numeric_literal_formats(void) {
    uint8 bytes[] = {0};

    expect_ok_int(bytes, 1, "0x10", 16);
    expect_ok_int(bytes, 1, "0b1010", 10);
    expect_ok_double(bytes, 1, "1.5", 1.5);
    expect_ok_double(bytes, 1, "1e3", 1000.0);
    expect_ok_double(bytes, 1, "2.5e-1", 0.25);
    expect_ok_int(bytes, 1, "0x10 + 0b11", 19);
}

static void test_byte_access(void) {
    uint8 bytes[] = {0x12, 0x34, 0xA5, 0xFF};

    expect_ok_int(bytes, 4, "$0", 0x12);
    expect_ok_int(bytes, 4, "$1", 0x34);
    expect_ok_int(bytes, 4, "$2", 0xA5);
    expect_ok_int(bytes, 4, "$3", 0xFF);
    expect_ok_int(bytes, 4, "$0 * 256 + $1", 0x1234);
    expect_ok_double(bytes, 4, "($0 * 256) / 2 + $1", 0x934);
}

static void test_parentheses_and_precedence(void) {
    uint8 bytes[] = {0x10, 0x20};

    expect_ok_int(bytes, 2, "1 + 2 * 3 << 1", 14);
    expect_ok_int(bytes, 2, "(1 + 2) * (3 + 4)", 21);
    expect_ok_int(bytes, 2, "$0 + $1 * 2", 0x10 + 0x20 * 2);
    expect_ok_int(bytes, 2, "($0 + $1) * 2", (0x10 + 0x20) * 2);
}

static void test_bitwise_ops(void) {
    uint8 bytes[] = {0x12, 0x34, 0xA5, 0xFF};

    expect_ok_int(bytes, 4, "0xF0 & 0xAA", 0xA0);
    expect_ok_int(bytes, 4, "0xF0 | 0x0A", 0xFA);
    expect_ok_int(bytes, 4, "0xF0 ^ 0xAA", 0x5A);
    expect_ok_int(bytes, 4, "~0", -1);
    expect_ok_int(bytes, 4, "1 << 4", 16);
    expect_ok_int(bytes, 4, "16 >> 2", 4);
    expect_ok_int(bytes, 4, "($2 & 0xF0) >> 4", 0x0A);
    expect_ok_int(bytes, 4, "$2 & 0x0F", 0x05);
    expect_ok_int(bytes, 4, "$0 << 8 | $1", 0x1234);
}

static void test_logical_and_comparison_ops(void) {
    uint8 bytes[] = {0x12, 0x00};

    expect_ok_int(bytes, 2, "1 < 2", 1);
    expect_ok_int(bytes, 2, "2 < 1", 0);
    expect_ok_int(bytes, 2, "2 <= 2", 1);
    expect_ok_int(bytes, 2, "3 > 2", 1);
    expect_ok_int(bytes, 2, "3 >= 4", 0);
    expect_ok_int(bytes, 2, "5 == 5", 1);
    expect_ok_int(bytes, 2, "5 != 5", 0);
    expect_ok_int(bytes, 2, "!0", 1);
    expect_ok_int(bytes, 2, "!5", 0);
    expect_ok_int(bytes, 2, "1 && 1", 1);
    expect_ok_int(bytes, 2, "1 && 0", 0);
    expect_ok_int(bytes, 2, "0 || 7", 1);
    expect_ok_int(bytes, 2, "$0 && $1", 0);
    expect_ok_int(bytes, 2, "$0 || $1", 1);
}

static void test_ternary_operator(void) {
    uint8 bytes[] = {0x41, 0x10, 0x42};

    expect_ok_int(bytes, 3, "1 ? 10 : 20", 10);
    expect_ok_int(bytes, 3, "0 ? 10 : 20", 20);
    expect_ok_int(bytes, 3, "$0 == 0x41 ? $1 : $2", 0x10);
    expect_ok_int(bytes, 3, "$0 == 0x40 ? $1 : $2", 0x42);
    expect_ok_int(bytes, 3, "$1 ? ($2 + 1) : ($0 + 1)", 0x43);
}

static void test_constants(void) {
    uint8 bytes[] = {0};

    expect_ok_int(bytes, 1, "true", 1);
    expect_ok_int(bytes, 1, "false", 0);
    expect_ok_double(bytes, 1, "pi", 3.14159265358979323846);
    expect_ok_double(bytes, 1, "e", 2.71828182845904523536);
}

static void test_len_and_simple_functions(void) {
    uint8 bytes[] = {0x12, 0x34, 0x56, 0x78};

    expect_ok_int(bytes, 4, "len()", 4);
    expect_ok_double(bytes, 4, "min(10, 3)", 3.0);
    expect_ok_double(bytes, 4, "max(10, 3)", 10.0);
    expect_ok_double(bytes, 4, "abs(-12.5)", 12.5);
    expect_ok_double(bytes, 4, "clamp(5, 0, 10)", 5.0);
    expect_ok_double(bytes, 4, "clamp(-5, 0, 10)", 0.0);
    expect_ok_double(bytes, 4, "clamp(15, 0, 10)", 10.0);
    expect_ok_double(bytes, 4, "if(1, 11, 22)", 11.0);
    expect_ok_double(bytes, 4, "if(0, 11, 22)", 22.0);
}

static void test_unsigned_buffer_read_functions(void) {
    uint8 bytes[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};

    expect_ok_int(bytes, 8, "u8(0)", 0x12);
    expect_ok_int(bytes, 8, "u16be(0)", 0x1234);
    expect_ok_int(bytes, 8, "u16le(0)", 0x3412);
    expect_ok_int(bytes, 8, "u24be(0)", 0x123456);
    expect_ok_int(bytes, 8, "u24le(0)", 0x563412);
    expect_ok_int(bytes, 8, "u32be(0)", 0x12345678);
    expect_ok_int(bytes, 8, "u32le(0)", 0x78563412);
    expect_ok_double(bytes, 8, "u64be(0)", 1311768467463790320.0);
    expect_ok_double(bytes, 8, "u64le(0)", 17356517385562371090.0);
}

static void test_signed_buffer_read_functions(void) {
    uint8 bytes1[] = {0xFF, 0xFE};
    uint8 bytes2[] = {0xFE, 0xFF};
    uint8 bytes3[] = {0xFF, 0xFF, 0xFF, 0x80};
    uint8 bytes4[] = {0x80, 0xFF, 0xFF, 0xFF};

    expect_ok_int(bytes1, 2, "s16be(0)", -2);
    expect_ok_int(bytes2, 2, "s16le(0)", -2);
    expect_ok_int(bytes3, 4, "s32be(0)", -128);
    expect_ok_int(bytes4, 4, "s32le(0)", -128);
}

static void test_bit_and_bits_functions(void) {
    uint8 bytes[] = {0xA5, 0x3C};

    expect_ok_int(bytes, 2, "bit(0, 0)", 1);
    expect_ok_int(bytes, 2, "bit(0, 1)", 0);
    expect_ok_int(bytes, 2, "bit(0, 2)", 1);
    expect_ok_int(bytes, 2, "bit(0, 7)", 1);
    expect_ok_int(bytes, 2, "bits(0, 0, 4)", 0x5);
    expect_ok_int(bytes, 2, "bits(0, 4, 4)", 0xA);
    expect_ok_int(bytes, 2, "bits(1, 2, 3)", 0x7);
}

static void test_mixed_realistic_expressions(void) {
    uint8 bytes[] = {0x41, 0x1A, 0xF0, 0x08, 0xFF, 0x7F};

    expect_ok_double(bytes, 6, "($1 * 256 + $2) / 4", ((0x1A * 256.0) + 0xF0) / 4.0);
    expect_ok_int(bytes, 6, "$0 == 0x41 ? u16be(1) : -1", 0x1AF0);
    expect_ok_int(bytes, 6, "($2 & 0xF0) >> 4", 0x0F);
    expect_ok_int(bytes, 6, "bit(3, 3) ? 1 : 0", 1);
    expect_ok_int(bytes, 6, "u16be(4)", 0xFF7F);
    expect_ok_int(bytes, 6, "s16be(4)", -129);
    expect_ok_double(bytes, 6, "clamp(u16be(1) / 10.0, 0, 1000)", 689.6);
}
static void test_only_warn() {
    {
        uint8 bytes[] = {0x12, 0x34};
        expect_ok_double(bytes, 2, "1 2", 0x01);
    }
    {
        Buffer * data_buffer = ad_buffer_from_ascii_hex("123456");
        char * errorReturn = null;
        double result = ad_expr_reduce_buffer(data_buffer, "1 qdsfdq a", &errorReturn);
        assert(result == 1.0);
    }
}
static void test_error_invalid_syntax(void) {
    uint8 bytes[] = {0x12, 0x34};

    expect_error_contains(bytes, 2, "", "unexpected token");
    expect_error_contains(bytes, 2, "(", "unexpected token");
    expect_error_contains(bytes, 2, "(1 + 2", "expected ')'");
    expect_error_contains(bytes, 2, "1 +", "unexpected token");
    expect_error_contains(bytes, 2, "1 ?", "unexpected token");
    expect_error_contains(bytes, 2, "1 ? 2", "expected ':'");
    expect_error_contains(bytes, 2, "foo", "unknown function");
    expect_error_contains(bytes, 2, "min(1)", "expects 2 arguments");
    expect_error_contains(bytes, 2, "abs()", "expects 1 argument");
    expect_error_contains(bytes, 2, "len(1)", "expects 0 arguments");
}

static void test_error_invalid_buffer_access(void) {
    uint8 bytes[] = {0x12, 0x34};

    expect_error_contains(bytes, 2, "u8(2)", "out of range");
    expect_error_contains(bytes, 2, "u16be(1)", "out of range");
    expect_error_contains(bytes, 2, "u32le(0)", "out of range");
    expect_error_contains(bytes, 2, "s16be(1)", "out of range");
    expect_error_contains(bytes, 2, "bit(2, 0)", "out of range");
    expect_error_contains(bytes, 2, "bits(2, 0, 1)", "out of range");
}

static void test_error_invalid_bit_parameters(void) {
    uint8 bytes[] = {0x12};

    expect_error_contains(bytes, 1, "bit(0, -1)", "bit index");
    expect_error_contains(bytes, 1, "bit(0, 8)", "bit index");
    expect_error_contains(bytes, 1, "bits(0, -1, 2)", "start");
    expect_error_contains(bytes, 1, "bits(0, 8, 1)", "start");
    expect_error_contains(bytes, 1, "bits(0, 0, 0)", "width");
    expect_error_contains(bytes, 1, "bits(0, 4, 5)", "width");
}

static void test_error_invalid_runtime(void) {
    uint8 bytes[] = {0x12};

    expect_error_contains(bytes, 1, "1 / 0", "division by zero");
    expect_error_contains(bytes, 1, "1 % 0", "modulo by zero");
    expect_error_contains(bytes, 1, "1 << -1", "shift count");
    expect_error_contains(bytes, 1, "1 << 64", "shift count");
    expect_error_contains(bytes, 1, "1 >> -1", "shift count");
    expect_error_contains(bytes, 1, "1 >> 64", "shift count");
}

static void test_null_argument_errors(void) {
    uint8 bytes[] = {0x12};
    char *err = NULL;
    double v;

    v = ad_expr_reduce(NULL, 1, "1", &err);
    assert(isnan(v));
    assert(err != NULL);
    assert(strstr(err, "bytes is NULL") != NULL);
    free(err);

    err = NULL;
    v = ad_expr_reduce(bytes, 1, NULL, &err);
    assert(isnan(v));
    assert(err != NULL);
    assert(strstr(err, "expr is NULL") != NULL);
    free(err);

    err = NULL;
    v = ad_expr_reduce(bytes, -1, "1", &err);
    assert(isnan(v));
    assert(err != NULL);
    assert(strstr(err, "sz is negative") != NULL);
    free(err);
}

static void test_error_pointer_can_be_null(void) {
    uint8 bytes[] = {0x12};
    double v;

    v = ad_expr_reduce(bytes, 1, "1 + 2", NULL);
    assert(!isnan(v));
    assert(fabs(v - 3.0) < 1e-9);

    v = ad_expr_reduce(bytes, 1, "1 / 0", NULL);
    assert(isnan(v));
}

static void test_nested_function_calls(void) {
    uint8 bytes[] = {0x12, 0x34, 0x56, 0x78};

    expect_ok_double(bytes, 4, "max(min(10, 20), abs(-15))", 15.0);
    expect_ok_double(bytes, 4, "if(u16be(0) == 0x1234, clamp(999, 0, 100), 0)", 100.0);
    expect_ok_double(bytes, 4, "min(u16be(0), u16le(0))", 0x1234);
}

static void test_boolean_coercion_patterns(void) {
    uint8 bytes[] = {0x00, 0x01};

    expect_ok_int(bytes, 2, "if($0, 5, 9)", 9);
    expect_ok_int(bytes, 2, "if($1, 5, 9)", 5);
    expect_ok_int(bytes, 2, "($0 == 0) && ($1 == 1)", 1);
    expect_ok_int(bytes, 2, "($0 != 0) || ($1 == 1)", 1);
}
static void test_some_signals() {
    char * errorReturn = null;
    {
        log_info("vehicle speed");
        Buffer * data_buffer = ad_buffer_from_ascii_hex("FF");
        assert(ad_expr_reduce_buffer(data_buffer, "$0", &errorReturn) == 255.0);
    }
    {
        log_info("engine speed");
        Buffer * data_buffer = ad_buffer_be_from_uint16(1000 * 4);
        double result = ad_expr_reduce_buffer(data_buffer, "($0 * 256 + $1)/4", &errorReturn);
        log_debug("result = %f", result);
        assert(result == 1000.0);
    }
}
static void test_parse_fail() {
    Buffer * data_buffer = ad_buffer_from_ascii_hex("123456");
    {
        char * errorReturn = null;
        ad_expr_reduce_buffer(data_buffer, "aezrjlkeazr", &errorReturn);
        assert(errorReturn != null);
    }
    {
        char * errorReturn = null;
        ad_expr_reduce_buffer(data_buffer, "1 + a", &errorReturn);
        assert(errorReturn != null);
    }
    {
        char * errorReturn = null;
        ad_expr_reduce_buffer(data_buffer, "1 +/ a", &errorReturn);
        assert(errorReturn != null);
    }
}
static void test_particular_case() {
    Buffer * data = ad_buffer_from_ascii_hex("41070000000000");
    {
        char * errorReturn = null;
        double result = ad_expr_reduce_buffer(data, "$2 / 1.28 - 100", &errorReturn);
        assert(result != NAN && result == -100.0);
    }
}
static void test_invert_masked_selected_byte() {
    char *err = NULL;
    int signal_offset = -1;
    Buffer *out = ad_expr_reduce_invert(
        4.0,
        "($0 == 0x41 ? $2 : $3) & 0x7F",
        &signal_offset,
        &err
    );

    assert(err == NULL);
    assert(out != NULL);
    assert(out->size == 4);
    assert((out->buffer[2] & 0x7F) == 4);
    assert((out->buffer[3] & 0x7F) == 4);

    ad_buffer_free(out);
}

static void test_invert_shifted_bit_selected_byte() {
    char *err = NULL;
    int signal_offset = -1;
    Buffer *out = ad_expr_reduce_invert(
        1.0,
        "(($0 == 0x41 ? $2 : $3) >> 7) & 1",
        &signal_offset,
        &err
    );

    assert(err == NULL);
    assert(out != NULL);
    assert(out->size == 4);
    assert(((out->buffer[2] >> 7) & 1) == 1);
    assert(((out->buffer[3] >> 7) & 1) == 1);

    ad_buffer_free(out);
}

static void test_invert_byte_div_minus_formula() {
    char *err = NULL;
    int signal_offset = -1;
    Buffer *out = ad_expr_reduce_invert(
        60.0,
        "(($0 == 0x41 ? $2 : $3) / 2) - 40",
        &signal_offset,
        &err
    );

    assert(err == NULL);
    assert(out != NULL);
    assert(out->size == 4);
    assert(out->buffer[2] == 200);
    assert(out->buffer[3] == 200);

    ad_buffer_free(out);
}

static void test_invert_u16_div_formula() {
    char *err = NULL;
    int signal_offset = -1;
    Buffer *out = ad_expr_reduce_invert(
        1000.0,
        "((($0 == 0x41 ? $2 : $3) * 256) + ($0 == 0x41 ? $4 : $5)) / 4",
        &signal_offset,
        &err
    );

    assert(err == NULL);
    assert(out != NULL);
    assert(out->size == 6);
    assert(out->buffer[2] == 0x0F);
    assert(out->buffer[3] == 0x0F);
    assert(out->buffer[4] == 0xA0);
    assert(out->buffer[5] == 0xA0);

    ad_buffer_free(out);
}

static void test_invert_fails_when_target_out_of_byte_range() {
    char *err = NULL;
    int signal_offset = -1;
    Buffer *out = ad_expr_reduce_invert(
        300.0,
        "($0 == 0x41 ? $2 : $3) & 0x7F",
        &signal_offset,
        &err
    );

    assert(out == NULL);
    assert(err != NULL);
    free(err);
}

static void test_invert_round_trip_masked_selected_byte() {
    char *err = NULL;
    int signal_offset = -1;
    Buffer *out = ad_expr_reduce_invert(
        37.0,
        "($0 == 0x41 ? $2 : $3) & 0x7F",
        &signal_offset,
        &err
    );
    double v_true;
    double v_false;

    assert(err == NULL);
    assert(out != NULL);

    out->buffer[0] = 0x41;
    v_true = ad_expr_reduce_buffer(out, "($0 == 0x41 ? $2 : $3) & 0x7F", &err);
    assert(err == NULL);
    assert(v_true == 37.0);

    out->buffer[0] = 0x42;
    v_false = ad_expr_reduce_buffer(out, "($0 == 0x41 ? $2 : $3) & 0x7F", &err);
    assert(err == NULL);
    assert(v_false == 37.0);

    ad_buffer_free(out);
}
static void test_buffer_creation_vehicle_speed() {
    ad_saej1979_data_register_signals();
    ad_object_vehicle_signal * signal = ad_signal_get("SAEJ1979.vehicle_speed");
    int signal_offset = -1;
    char *err = NULL;
    Buffer *out = ad_expr_reduce_invert(
        155.0,
        signal->rv_formula,
        &signal_offset,
        &err
    );
    assert(err == NULL);
    double speed = ad_expr_reduce_buffer(out, signal->rv_formula, &err);
    assert(err == NULL);
    assert(fabs(speed - 155.0) < 1e-9);
}

bool testExpr() {
    tf_run_case(test_particular_case);
    tf_run_case(test_byte_access);
    tf_run_case(test_logical_and_comparison_ops);
    tf_run_case(test_ternary_operator);
    tf_run_case(test_mixed_realistic_expressions);
    tf_run_case(test_error_invalid_syntax);
    tf_run_case(test_only_warn);
    tf_run_case(test_error_invalid_buffer_access);
    tf_run_case(test_boolean_coercion_patterns);
    tf_run_case(test_basic_arithmetic_literals);
    tf_run_case(test_numeric_literal_formats);
    tf_run_case(test_parentheses_and_precedence);
    tf_run_case(test_bitwise_ops);
    tf_run_case(test_constants);
    tf_run_case(test_len_and_simple_functions);
    tf_run_case(test_unsigned_buffer_read_functions);
    tf_run_case(test_signed_buffer_read_functions);
    tf_run_case(test_bit_and_bits_functions);
    tf_run_case(test_error_invalid_bit_parameters);
    tf_run_case(test_error_invalid_runtime);
    tf_run_case(test_null_argument_errors);
    tf_run_case(test_error_pointer_can_be_null);
    tf_run_case(test_nested_function_calls);
    tf_run_case(test_some_signals);
    tf_run_case(test_parse_fail);
    tf_run_case(test_invert_masked_selected_byte);
    tf_run_case(test_invert_shifted_bit_selected_byte);
    tf_run_case(test_invert_byte_div_minus_formula);
    tf_run_case(test_invert_u16_div_formula);
    tf_run_case(test_invert_fails_when_target_out_of_byte_range);
    tf_run_case(test_invert_round_trip_masked_selected_byte);
    tf_run_case(test_buffer_creation_vehicle_speed);
    return true;
}