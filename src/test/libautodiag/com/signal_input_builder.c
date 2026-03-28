#include "libTest.h"
#include "libautodiag/buffer.h"
#include "libautodiag/com/signal_input_builder.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void expect_hex_builderv(const char *expr, const char **args, int argc, const char *expected) {
    Buffer *buffer = ad_signal_input_expr_builderv(expr, args, argc);
    char *hex;
    assert(buffer != NULL);
    hex = ad_buffer_to_hex_string(buffer);
    assert(hex != NULL);
    assert(strcasecmp(hex, expected) == 0);
    free(hex);
    ad_buffer_free(buffer);
}

static void expect_null_builderv(const char *expr, const char **args, int argc) {
    Buffer *buffer = ad_signal_input_expr_builderv(expr, args, argc);
    assert(buffer == NULL);
}

static void test_ad_signal_input_expr_builderv_single_arg(void) {
    const char *args[] = { "5" };
    expect_hex_builderv("$0", args, 1, "05");
}

static void test_ad_signal_input_expr_builderv_two_args_concat(void) {
    const char *args[] = { "1", "2" };
    expect_hex_builderv("$0$1", args, 2, "0102");
}

static void test_ad_signal_input_expr_builderv_optional_present(void) {
    const char *args[] = { "2", "1" };
    expect_hex_builderv("$0${1}01", args, 2, "020101");
}

static void test_ad_signal_input_expr_builderv_optional_missing(void) {
    const char *args[] = { "2" };
    expect_hex_builderv("$0${1}01", args, 1, "0201");
}

static void test_ad_signal_input_expr_builderv_flatten_be32(void) {
    const char *args[] = { "1" };
    expect_hex_builderv("$(flatten_be32($0))", args, 1, "00000001");
}

static void test_ad_signal_input_expr_builderv_flatten_be16_add(void) {
    const char *args[] = { "1" };
    expect_hex_builderv("$(flatten_be16($0 + 10))", args, 1, "000B");
}

static void test_ad_signal_input_expr_builderv_default_flatten_be8(void) {
    const char *args[] = { "1", "0" };
    expect_hex_builderv("$($0 + 1 + $1)", args, 2, "02");
}

static void test_ad_signal_input_expr_builderv_flatten_be8(void) {
    const char *args[] = { "1" };
    expect_hex_builderv("$(flatten_be8($0 + 1))", args, 1, "02");
}

static void test_ad_signal_input_expr_builderv_char_as_numeric_string(void) {
    const char *args[] = { "65" };
    expect_hex_builderv("$0", args, 1, "41");
}

static void test_ad_signal_input_expr_builderv_double_numeric_cast(void) {
    const char *args[] = { "5.0" };
    expect_hex_builderv("$0", args, 1, "05");
}

static void test_ad_signal_input_expr_builderv_double_expr(void) {
    const char *args[] = { "1000.0" };
    expect_hex_builderv("$(flatten_be16(($0 / 4) * 5))", args, 1, "04E2");
}

static void test_ad_signal_input_expr_builderv_mixed_types_as_strings(void) {
    const char *args[] = { "1", "65", "258.0" };
    expect_hex_builderv("$0$1$(flatten_be16($2))", args, 3, "01410102");
}

static void test_ad_signal_input_expr_builderv_ascii_hex_arg(void) {
    const char *args[] = { "0A0B" };
    expect_hex_builderv("$0", args, 1, "0A0B");
}

static void test_ad_signal_input_expr_builderv_literals_only(void) {
    expect_hex_builderv("010203", NULL, 0, "010203");
}

static void test_ad_signal_input_expr_builderv_mixed_literal_and_args(void) {
    const char *args[] = { "2", "3" };
    expect_hex_builderv("AA$0BB$1CC", args, 2, "AA02BB03CC");
}

static void test_ad_signal_input_expr_builderv_spaces_ignored_in_expr(void) {
    const char *args[] = { "1" };
    expect_hex_builderv("$( flatten_be16( $0 + 1 ) )", args, 1, "0002");
}

static void test_ad_signal_input_expr_builderv_missing_do_not_fails(void) {
    const char *args[] = { "1" };
    expect_hex_builderv("$0$1", args, 1, "01");
}

static void test_ad_signal_input_expr_builderv_missing_expr_arg_fails(void) {
    const char *args[] = { "1" };
    expect_null_builderv("$($0 + $1)", args, 1);
}

static void test_ad_signal_input_expr_builderv_bad_syntax_fails(void) {
    const char *args[] = { "1" };
    expect_null_builderv("$(flatten_be16($0 + 1)", args, 1);
}

static void test_ad_signal_input_expr_builderv_bad_token_fails(void) {
    const char *args[] = { "1" };
    expect_null_builderv("$(foo($0))", args, 1);
}

static void test_ad_signal_input_expr_builderv_nested_parentheses(void) {
    const char *args[] = { "1", "2" };
    expect_hex_builderv("$(flatten_be16(($0 + 3) * ($1 + 1)))", args, 2, "000C");
}
static void optionnal_pid_signal() {
    Buffer * signal = ad_signal_input_expr_builder("$0${1}01", 0x01);
    assert(ad_buffer_cmp(signal, ad_buffer_from_ascii_hex("0101")) == 0);
}
bool testSIB() {
    tf_run_case(test_ad_signal_input_expr_builderv_single_arg);
    tf_run_case(test_ad_signal_input_expr_builderv_two_args_concat);
    tf_run_case(test_ad_signal_input_expr_builderv_optional_present);
    tf_run_case(test_ad_signal_input_expr_builderv_optional_missing);
    tf_run_case(test_ad_signal_input_expr_builderv_flatten_be32);
    tf_run_case(test_ad_signal_input_expr_builderv_flatten_be16_add);
    tf_run_case(test_ad_signal_input_expr_builderv_default_flatten_be8);
    tf_run_case(test_ad_signal_input_expr_builderv_flatten_be8);
    tf_run_case(test_ad_signal_input_expr_builderv_char_as_numeric_string);
    tf_run_case(test_ad_signal_input_expr_builderv_double_numeric_cast);
    tf_run_case(test_ad_signal_input_expr_builderv_double_expr);
    tf_run_case(test_ad_signal_input_expr_builderv_mixed_types_as_strings);
    tf_run_case(test_ad_signal_input_expr_builderv_ascii_hex_arg);
    tf_run_case(test_ad_signal_input_expr_builderv_literals_only);
    tf_run_case(test_ad_signal_input_expr_builderv_mixed_literal_and_args);
    tf_run_case(test_ad_signal_input_expr_builderv_spaces_ignored_in_expr);
    tf_run_case(test_ad_signal_input_expr_builderv_missing_do_not_fails);
    tf_run_case(test_ad_signal_input_expr_builderv_missing_expr_arg_fails);
    tf_run_case(test_ad_signal_input_expr_builderv_bad_syntax_fails);
    tf_run_case(test_ad_signal_input_expr_builderv_bad_token_fails);
    tf_run_case(test_ad_signal_input_expr_builderv_nested_parentheses);
    tf_run_case(optionnal_pid_signal);
    return true;
}