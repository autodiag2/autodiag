#ifndef __AD_COM_SIB_H
#define __AD_COM_SIB_H

#include "libautodiag/buffer.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *s;
    const char **args;
    int argc;
    const char *error;
} ad_signal_input_expr_parser;

typedef struct {
    double value;
    int flatten_bits;
} ad_signal_input_expr_value;

Buffer *ad_signal_input_expr_builder_auto_impl(const char *expr, char **owned_args, int argc);
char *ad_signal_input_arg_from_char(char v);
char *ad_signal_input_arg_from_double(double v);
char *ad_signal_input_arg_from_ull(unsigned long long v);
char *ad_signal_input_arg_from_ll(long long v);
char *ad_signal_input_strdup(const char *s);

#define __AD_SIG_NARG_I(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define __AD_SIG_NARG(...) __AD_SIG_NARG_I(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)

#define __AD_SIG_CAT_I(a,b) a##b
#define __AD_SIG_CAT(a,b) __AD_SIG_CAT_I(a,b)

#define __AD_SIG_ARG_TO_OWNED(x) _Generic((x), \
    char: ad_signal_input_arg_from_char, \
    signed char: ad_signal_input_arg_from_ll, \
    unsigned char: ad_signal_input_arg_from_ull, \
    short: ad_signal_input_arg_from_ll, \
    unsigned short: ad_signal_input_arg_from_ull, \
    int: ad_signal_input_arg_from_ll, \
    unsigned int: ad_signal_input_arg_from_ull, \
    long: ad_signal_input_arg_from_ll, \
    unsigned long: ad_signal_input_arg_from_ull, \
    long long: ad_signal_input_arg_from_ll, \
    unsigned long long: ad_signal_input_arg_from_ull, \
    float: ad_signal_input_arg_from_double, \
    double: ad_signal_input_arg_from_double, \
    long double: ad_signal_input_arg_from_double, \
    char *: ad_signal_input_strdup, \
    const char *: ad_signal_input_strdup \
)(x)

#define __AD_SIG_ARGS_0() NULL
#define __AD_SIG_ARGS_1(a1) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1) })
#define __AD_SIG_ARGS_2(a1,a2) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2) })
#define __AD_SIG_ARGS_3(a1,a2,a3) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3) })
#define __AD_SIG_ARGS_4(a1,a2,a3,a4) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3), __AD_SIG_ARG_TO_OWNED(a4) })
#define __AD_SIG_ARGS_5(a1,a2,a3,a4,a5) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3), __AD_SIG_ARG_TO_OWNED(a4), __AD_SIG_ARG_TO_OWNED(a5) })
#define __AD_SIG_ARGS_6(a1,a2,a3,a4,a5,a6) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3), __AD_SIG_ARG_TO_OWNED(a4), __AD_SIG_ARG_TO_OWNED(a5), __AD_SIG_ARG_TO_OWNED(a6) })
#define __AD_SIG_ARGS_7(a1,a2,a3,a4,a5,a6,a7) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3), __AD_SIG_ARG_TO_OWNED(a4), __AD_SIG_ARG_TO_OWNED(a5), __AD_SIG_ARG_TO_OWNED(a6), __AD_SIG_ARG_TO_OWNED(a7) })
#define __AD_SIG_ARGS_8(a1,a2,a3,a4,a5,a6,a7,a8) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3), __AD_SIG_ARG_TO_OWNED(a4), __AD_SIG_ARG_TO_OWNED(a5), __AD_SIG_ARG_TO_OWNED(a6), __AD_SIG_ARG_TO_OWNED(a7), __AD_SIG_ARG_TO_OWNED(a8) })
#define __AD_SIG_ARGS_9(a1,a2,a3,a4,a5,a6,a7,a8,a9) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3), __AD_SIG_ARG_TO_OWNED(a4), __AD_SIG_ARG_TO_OWNED(a5), __AD_SIG_ARG_TO_OWNED(a6), __AD_SIG_ARG_TO_OWNED(a7), __AD_SIG_ARG_TO_OWNED(a8), __AD_SIG_ARG_TO_OWNED(a9) })
#define __AD_SIG_ARGS_10(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) ((char*[]){ __AD_SIG_ARG_TO_OWNED(a1), __AD_SIG_ARG_TO_OWNED(a2), __AD_SIG_ARG_TO_OWNED(a3), __AD_SIG_ARG_TO_OWNED(a4), __AD_SIG_ARG_TO_OWNED(a5), __AD_SIG_ARG_TO_OWNED(a6), __AD_SIG_ARG_TO_OWNED(a7), __AD_SIG_ARG_TO_OWNED(a8), __AD_SIG_ARG_TO_OWNED(a9), __AD_SIG_ARG_TO_OWNED(a10) })

#define __AD_SIG_ARGS_DISPATCH(N, ...) __AD_SIG_CAT(__AD_SIG_ARGS_, N)(__VA_ARGS__)

#define ad_signal_input_expr_builder(expr, ...) \
    ad_signal_input_expr_builder_auto_impl((expr), __AD_SIG_ARGS_DISPATCH(__AD_SIG_NARG(__VA_ARGS__), __VA_ARGS__), __AD_SIG_NARG(__VA_ARGS__))

Buffer *ad_signal_input_expr_builderv(const char *expr, const char **args, int argc);

#endif