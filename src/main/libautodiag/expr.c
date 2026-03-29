#include "libautodiag/expr.h"

// see https://github.com/antlr/grammars-v4/blob/master/python/python3_13/PythonParser.g4

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NAN
#define NAN (0.0/0.0)
#endif

typedef uint8_t uint8;

typedef struct ad_expr_parser ad_expr_parser;

typedef enum {
    AD_EXPR_TOKEN_EOF,
    AD_EXPR_TOKEN_NUMBER,
    AD_EXPR_TOKEN_NAME,
    AD_EXPR_TOKEN_BUFFER,
    AD_EXPR_TOKEN_LPAREN,
    AD_EXPR_TOKEN_RPAREN,
    AD_EXPR_TOKEN_COMMA,
    AD_EXPR_TOKEN_QUESTION,
    AD_EXPR_TOKEN_COLON,
    AD_EXPR_TOKEN_PLUS,
    AD_EXPR_TOKEN_MINUS,
    AD_EXPR_TOKEN_STAR,
    AD_EXPR_TOKEN_SLASH,
    AD_EXPR_TOKEN_PERCENT,
    AD_EXPR_TOKEN_BANG,
    AD_EXPR_TOKEN_TILDE,
    AD_EXPR_TOKEN_AMP,
    AD_EXPR_TOKEN_PIPE,
    AD_EXPR_TOKEN_CARET,
    AD_EXPR_TOKEN_LT,
    AD_EXPR_TOKEN_GT,
    AD_EXPR_TOKEN_LTE,
    AD_EXPR_TOKEN_GTE,
    AD_EXPR_TOKEN_EQEQ,
    AD_EXPR_TOKEN_NEQ,
    AD_EXPR_TOKEN_ANDAND,
    AD_EXPR_TOKEN_OROR,
    AD_EXPR_TOKEN_LSHIFT,
    AD_EXPR_TOKEN_RSHIFT
} ad_expr_token_type;

typedef struct {
    ad_expr_token_type type;
    double number;
    long long integer;
    char text[64];
} ad_expr_token;

struct ad_expr_parser {
    const uint8 *bytes;
    int sz;
    const char *expr;
    const char *cur;
    ad_expr_token tok;
    char *error;
};

static char *ad_expr_strdup(const char *s) {
    size_t n;
    char *r;
    if (s == NULL) {
        return NULL;
    }
    n = strlen(s);
    r = (char *)malloc(n + 1);
    if (r == NULL) {
        return NULL;
    }
    memcpy(r, s, n + 1);
    return r;
}

static void ad_expr_set_error(ad_expr_parser *p, const char *fmt, ...) {
    char buf[256];
    va_list ap;
    if (p == NULL || p->error != NULL) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    p->error = ad_expr_strdup(buf);
    if (p->error == NULL) {
        p->error = ad_expr_strdup("out of memory");
    }
}

static bool ad_expr_failed(ad_expr_parser *p) {
    return p == NULL || p->error != NULL;
}

static void ad_expr_skip_ws(ad_expr_parser *p) {
    while (*p->cur != 0 && isspace((unsigned char)*p->cur)) {
        p->cur++;
    }
}

static bool ad_expr_is_name_start(int c) {
    return isalpha(c) || c == '_';
}

static bool ad_expr_is_name_continue(int c) {
    return isalnum(c) || c == '_';
}

static void ad_expr_next_token(ad_expr_parser *p) {
    const char *s;
    char *endptr;
    size_t n;

    if (ad_expr_failed(p)) {
        p->tok.type = AD_EXPR_TOKEN_EOF;
        return;
    }

    ad_expr_skip_ws(p);
    s = p->cur;
    p->tok.type = AD_EXPR_TOKEN_EOF;
    p->tok.number = 0.0;
    p->tok.integer = 0;
    p->tok.text[0] = 0;

    if (*s == 0) {
        p->tok.type = AD_EXPR_TOKEN_EOF;
        return;
    }

    if (s[0] == '&' && s[1] == '&') {
        p->tok.type = AD_EXPR_TOKEN_ANDAND;
        p->cur += 2;
        return;
    }
    if (s[0] == '|' && s[1] == '|') {
        p->tok.type = AD_EXPR_TOKEN_OROR;
        p->cur += 2;
        return;
    }
    if (s[0] == '=' && s[1] == '=') {
        p->tok.type = AD_EXPR_TOKEN_EQEQ;
        p->cur += 2;
        return;
    }
    if (s[0] == '!' && s[1] == '=') {
        p->tok.type = AD_EXPR_TOKEN_NEQ;
        p->cur += 2;
        return;
    }
    if (s[0] == '<' && s[1] == '=') {
        p->tok.type = AD_EXPR_TOKEN_LTE;
        p->cur += 2;
        return;
    }
    if (s[0] == '>' && s[1] == '=') {
        p->tok.type = AD_EXPR_TOKEN_GTE;
        p->cur += 2;
        return;
    }
    if (s[0] == '<' && s[1] == '<') {
        p->tok.type = AD_EXPR_TOKEN_LSHIFT;
        p->cur += 2;
        return;
    }
    if (s[0] == '>' && s[1] == '>') {
        p->tok.type = AD_EXPR_TOKEN_RSHIFT;
        p->cur += 2;
        return;
    }

    switch (*s) {
        case '(': p->tok.type = AD_EXPR_TOKEN_LPAREN; p->cur++; return;
        case ')': p->tok.type = AD_EXPR_TOKEN_RPAREN; p->cur++; return;
        case ',': p->tok.type = AD_EXPR_TOKEN_COMMA; p->cur++; return;
        case '?': p->tok.type = AD_EXPR_TOKEN_QUESTION; p->cur++; return;
        case ':': p->tok.type = AD_EXPR_TOKEN_COLON; p->cur++; return;
        case '+': p->tok.type = AD_EXPR_TOKEN_PLUS; p->cur++; return;
        case '-': p->tok.type = AD_EXPR_TOKEN_MINUS; p->cur++; return;
        case '*': p->tok.type = AD_EXPR_TOKEN_STAR; p->cur++; return;
        case '/': p->tok.type = AD_EXPR_TOKEN_SLASH; p->cur++; return;
        case '%': p->tok.type = AD_EXPR_TOKEN_PERCENT; p->cur++; return;
        case '!': p->tok.type = AD_EXPR_TOKEN_BANG; p->cur++; return;
        case '~': p->tok.type = AD_EXPR_TOKEN_TILDE; p->cur++; return;
        case '&': p->tok.type = AD_EXPR_TOKEN_AMP; p->cur++; return;
        case '|': p->tok.type = AD_EXPR_TOKEN_PIPE; p->cur++; return;
        case '^': p->tok.type = AD_EXPR_TOKEN_CARET; p->cur++; return;
        case '<': p->tok.type = AD_EXPR_TOKEN_LT; p->cur++; return;
        case '>': p->tok.type = AD_EXPR_TOKEN_GT; p->cur++; return;
        default: break;
    }

    if (*s == '$') {
        long long v;
        p->cur++;
        if (!isdigit((unsigned char)*p->cur)) {
            ad_expr_set_error(p, "unexpected token after '$'");
            return;
        }
        v = 0;
        while (isdigit((unsigned char)*p->cur)) {
            v = v * 10 + (*p->cur - '0');
            p->cur++;
        }
        p->tok.type = AD_EXPR_TOKEN_BUFFER;
        p->tok.integer = v;
        return;
    }

    if (ad_expr_is_name_start((unsigned char)*s)) {
        n = 0;
        while (ad_expr_is_name_continue((unsigned char)*p->cur)) {
            if (n + 1 < sizeof(p->tok.text)) {
                p->tok.text[n++] = *p->cur;
            }
            p->cur++;
        }
        p->tok.text[n] = 0;
        p->tok.type = AD_EXPR_TOKEN_NAME;
        return;
    }

    if (isdigit((unsigned char)*s) || (s[0] == '.' && isdigit((unsigned char)s[1]))) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            unsigned long long v = strtoull(s, &endptr, 16);
            p->tok.type = AD_EXPR_TOKEN_NUMBER;
            p->tok.number = (double)v;
            p->cur = endptr;
            return;
        }
        if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
            unsigned long long v = 0;
            p->cur += 2;
            if (*p->cur != '0' && *p->cur != '1') {
                ad_expr_set_error(p, "unexpected token");
                return;
            }
            while (*p->cur == '0' || *p->cur == '1') {
                v = (v << 1) | (unsigned long long)(*p->cur - '0');
                p->cur++;
            }
            p->tok.type = AD_EXPR_TOKEN_NUMBER;
            p->tok.number = (double)v;
            return;
        }
        p->tok.type = AD_EXPR_TOKEN_NUMBER;
        p->tok.number = strtod(s, &endptr);
        p->cur = endptr;
        return;
    }

    ad_expr_set_error(p, "unexpected token");
}

static bool ad_expr_accept(ad_expr_parser *p, ad_expr_token_type type) {
    if (p->tok.type == type) {
        ad_expr_next_token(p);
        return true;
    }
    return false;
}

static bool ad_expr_expect(ad_expr_parser *p, ad_expr_token_type type, const char *msg) {
    if (p->tok.type == type) {
        ad_expr_next_token(p);
        return true;
    }
    ad_expr_set_error(p, "%s", msg);
    return false;
}

static bool ad_expr_is_true(double v) {
    return v != 0.0;
}

static long long ad_expr_to_i64(double v) {
    return (long long)v;
}

static double ad_expr_read_u(ad_expr_parser *p, int off, int width, bool little_endian) {
    unsigned long long v;
    int i;
    if (off < 0 || width < 1 || p->sz < off + width) {
        ad_expr_set_error(p, "out of range");
        return NAN;
    }
    v = 0;
    if (little_endian) {
        for (i = width - 1; 0 <= i; i--) {
            v = (v << 8) | (unsigned long long)p->bytes[off + i];
        }
    } else {
        for (i = 0; i < width; i++) {
            v = (v << 8) | (unsigned long long)p->bytes[off + i];
        }
    }
    return (double)v;
}

static double ad_expr_read_s(ad_expr_parser *p, int off, int width, bool little_endian) {
    unsigned long long raw;
    unsigned long long sign_bit;
    unsigned long long mask;
    long long signed_value;
    int bits;
    double u;
    u = ad_expr_read_u(p, off, width, little_endian);
    if (ad_expr_failed(p)) {
        return NAN;
    }
    raw = (unsigned long long)u;
    bits = width * 8;
    sign_bit = 1ULL << (bits - 1);
    if ((raw & sign_bit) == 0) {
        return (double)(long long)raw;
    }
    mask = (bits == 64) ? 0ULL : ((1ULL << bits) - 1ULL);
    if (bits == 64) {
        signed_value = (long long)raw;
    } else {
        signed_value = (long long)(raw | ~mask);
    }
    return (double)signed_value;
}

static double ad_expr_function_call(ad_expr_parser *p, const char *name, double *args, int argc) {
    if (strcmp(name, "len") == 0) {
        if (argc != 0) {
            ad_expr_set_error(p, "len expects 0 arguments");
            return NAN;
        }
        return (double)p->sz;
    }
    if (strcmp(name, "min") == 0) {
        if (argc != 2) {
            ad_expr_set_error(p, "min expects 2 arguments");
            return NAN;
        }
        return args[0] < args[1] ? args[0] : args[1];
    }
    if (strcmp(name, "max") == 0) {
        if (argc != 2) {
            ad_expr_set_error(p, "max expects 2 arguments");
            return NAN;
        }
        return args[0] < args[1] ? args[1] : args[0];
    }
    if (strcmp(name, "abs") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "abs expects 1 argument");
            return NAN;
        }
        return fabs(args[0]);
    }
    if (strcmp(name, "clamp") == 0) {
        if (argc != 3) {
            ad_expr_set_error(p, "clamp expects 3 arguments");
            return NAN;
        }
        if (args[0] < args[1]) {
            return args[1];
        }
        if (args[2] < args[0]) {
            return args[2];
        }
        return args[0];
    }
    if (strcmp(name, "if") == 0) {
        if (argc != 3) {
            ad_expr_set_error(p, "if expects 3 arguments");
            return NAN;
        }
        return ad_expr_is_true(args[0]) ? args[1] : args[2];
    }
    if (strcmp(name, "u8") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u8 expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 1, false);
    }
    if (strcmp(name, "u16be") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u16be expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 2, false);
    }
    if (strcmp(name, "u16le") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u16le expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 2, true);
    }
    if (strcmp(name, "u24be") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u24be expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 3, false);
    }
    if (strcmp(name, "u24le") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u24le expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 3, true);
    }
    if (strcmp(name, "u32be") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u32be expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 4, false);
    }
    if (strcmp(name, "u32le") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u32le expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 4, true);
    }
    if (strcmp(name, "u64be") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u64be expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 8, false);
    }
    if (strcmp(name, "u64le") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "u64le expects 1 argument");
            return NAN;
        }
        return ad_expr_read_u(p, (int)ad_expr_to_i64(args[0]), 8, true);
    }
    if (strcmp(name, "s16be") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "s16be expects 1 argument");
            return NAN;
        }
        return ad_expr_read_s(p, (int)ad_expr_to_i64(args[0]), 2, false);
    }
    if (strcmp(name, "s16le") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "s16le expects 1 argument");
            return NAN;
        }
        return ad_expr_read_s(p, (int)ad_expr_to_i64(args[0]), 2, true);
    }
    if (strcmp(name, "s32be") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "s32be expects 1 argument");
            return NAN;
        }
        return ad_expr_read_s(p, (int)ad_expr_to_i64(args[0]), 4, false);
    }
    if (strcmp(name, "s32le") == 0) {
        if (argc != 1) {
            ad_expr_set_error(p, "s32le expects 1 argument");
            return NAN;
        }
        return ad_expr_read_s(p, (int)ad_expr_to_i64(args[0]), 4, true);
    }
    if (strcmp(name, "bit") == 0) {
        int off;
        int bit_index;
        if (argc != 2) {
            ad_expr_set_error(p, "bit expects 2 arguments");
            return NAN;
        }
        off = (int)ad_expr_to_i64(args[0]);
        bit_index = (int)ad_expr_to_i64(args[1]);
        if (off < 0 || p->sz <= off) {
            ad_expr_set_error(p, "out of range");
            return NAN;
        }
        if (bit_index < 0 || 8 <= bit_index) {
            ad_expr_set_error(p, "bit index out of range");
            return NAN;
        }
        return (double)((p->bytes[off] >> bit_index) & 1);
    }
    if (strcmp(name, "bits") == 0) {
        int off;
        int start;
        int width;
        if (argc != 3) {
            ad_expr_set_error(p, "bits expects 3 arguments");
            return NAN;
        }
        off = (int)ad_expr_to_i64(args[0]);
        start = (int)ad_expr_to_i64(args[1]);
        width = (int)ad_expr_to_i64(args[2]);
        if (off < 0 || p->sz <= off) {
            ad_expr_set_error(p, "out of range");
            return NAN;
        }
        if (start < 0 || 8 <= start) {
            ad_expr_set_error(p, "bits start out of range");
            return NAN;
        }
        if (width < 1 || 8 < width || 8 < start + width) {
            ad_expr_set_error(p, "bits width out of range");
            return NAN;
        }
        return (double)((p->bytes[off] >> start) & ((1U << width) - 1U));
    }
    ad_expr_set_error(p, "unknown function");
    return NAN;
}

static double ad_expr_parse_expression(ad_expr_parser *p);
static double ad_expr_parse_disjunction(ad_expr_parser *p);
static double ad_expr_parse_conjunction(ad_expr_parser *p);
static double ad_expr_parse_inversion(ad_expr_parser *p);
static double ad_expr_parse_comparison(ad_expr_parser *p);
static double ad_expr_parse_bitwise_or(ad_expr_parser *p);
static double ad_expr_parse_bitwise_xor(ad_expr_parser *p);
static double ad_expr_parse_bitwise_and(ad_expr_parser *p);
static double ad_expr_parse_shift_expr(ad_expr_parser *p);
static double ad_expr_parse_sum(ad_expr_parser *p);
static double ad_expr_parse_term(ad_expr_parser *p);
static double ad_expr_parse_factor(ad_expr_parser *p);
static double ad_expr_parse_power(ad_expr_parser *p);
static double ad_expr_parse_primary(ad_expr_parser *p);
static double ad_expr_parse_atom(ad_expr_parser *p);

static double ad_expr_parse_expression(ad_expr_parser *p) {
    double cond;
    double yes;
    double no;
    cond = ad_expr_parse_disjunction(p);
    if (ad_expr_failed(p)) {
        return NAN;
    }
    if (ad_expr_accept(p, AD_EXPR_TOKEN_QUESTION)) {
        yes = ad_expr_parse_expression(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        if (!ad_expr_expect(p, AD_EXPR_TOKEN_COLON, "expected ':'")) {
            return NAN;
        }
        no = ad_expr_parse_expression(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        return ad_expr_is_true(cond) ? yes : no;
    }
    return cond;
}

static double ad_expr_parse_disjunction(ad_expr_parser *p) {
    double v;
    double rhs;
    v = ad_expr_parse_conjunction(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_OROR) {
        ad_expr_next_token(p);
        rhs = ad_expr_parse_conjunction(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        v = (ad_expr_is_true(v) || ad_expr_is_true(rhs)) ? 1.0 : 0.0;
    }
    return v;
}

static double ad_expr_parse_conjunction(ad_expr_parser *p) {
    double v;
    double rhs;
    v = ad_expr_parse_inversion(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_ANDAND) {
        ad_expr_next_token(p);
        rhs = ad_expr_parse_inversion(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        v = (ad_expr_is_true(v) && ad_expr_is_true(rhs)) ? 1.0 : 0.0;
    }
    return v;
}

static double ad_expr_parse_inversion(ad_expr_parser *p) {
    double v;
    if (ad_expr_accept(p, AD_EXPR_TOKEN_BANG)) {
        v = ad_expr_parse_inversion(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        return ad_expr_is_true(v) ? 0.0 : 1.0;
    }
    return ad_expr_parse_comparison(p);
}

static double ad_expr_parse_comparison(ad_expr_parser *p) {
    double left;
    double right;
    left = ad_expr_parse_bitwise_or(p);
    while (!ad_expr_failed(p)) {
        switch (p->tok.type) {
            case AD_EXPR_TOKEN_EQEQ:
                ad_expr_next_token(p);
                right = ad_expr_parse_bitwise_or(p);
                left = (left == right) ? 1.0 : 0.0;
                break;
            case AD_EXPR_TOKEN_NEQ:
                ad_expr_next_token(p);
                right = ad_expr_parse_bitwise_or(p);
                left = (left != right) ? 1.0 : 0.0;
                break;
            case AD_EXPR_TOKEN_LTE:
                ad_expr_next_token(p);
                right = ad_expr_parse_bitwise_or(p);
                left = (left <= right) ? 1.0 : 0.0;
                break;
            case AD_EXPR_TOKEN_LT:
                ad_expr_next_token(p);
                right = ad_expr_parse_bitwise_or(p);
                left = (left < right) ? 1.0 : 0.0;
                break;
            case AD_EXPR_TOKEN_GTE:
                ad_expr_next_token(p);
                right = ad_expr_parse_bitwise_or(p);
                left = (left >= right) ? 1.0 : 0.0;
                break;
            case AD_EXPR_TOKEN_GT:
                ad_expr_next_token(p);
                right = ad_expr_parse_bitwise_or(p);
                left = (left > right) ? 1.0 : 0.0;
                break;
            default:
                return left;
        }
        if (ad_expr_failed(p)) {
            return NAN;
        }
    }
    return left;
}

static double ad_expr_parse_bitwise_or(ad_expr_parser *p) {
    double left;
    double right;
    left = ad_expr_parse_bitwise_xor(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_PIPE) {
        ad_expr_next_token(p);
        right = ad_expr_parse_bitwise_xor(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        left = (long long)left | (long long)right;
    }
    return left;
}

static double ad_expr_parse_bitwise_xor(ad_expr_parser *p) {
    double left;
    double right;
    left = ad_expr_parse_bitwise_and(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_CARET) {
        ad_expr_next_token(p);
        right = ad_expr_to_i64(ad_expr_parse_bitwise_and(p));
        if (ad_expr_failed(p)) {
            return NAN;
        }
        left = (long long)left ^ (long long)right;
    }
    return left;
}

static double ad_expr_parse_bitwise_and(ad_expr_parser *p) {
    double left;
    double right;
    left = ad_expr_parse_shift_expr(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_AMP) {
        ad_expr_next_token(p);
        right = ad_expr_parse_shift_expr(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        left = (long long)left & (long long)right;
    }
    return left;
}

static double ad_expr_parse_shift_expr(ad_expr_parser *p) {
    double left;
    double right;
    left = ad_expr_parse_sum(p);
    while (!ad_expr_failed(p)) {
        if (p->tok.type == AD_EXPR_TOKEN_LSHIFT) {
            ad_expr_next_token(p);
            right = ad_expr_parse_sum(p);
            if (ad_expr_failed(p)) {
                return NAN;
            }
            if (right < 0 || 64 <= right) {
                ad_expr_set_error(p, "shift count out of range");
                return NAN;
            }
            left = (long long)left << (long long)right;
            continue;
        }
        if (p->tok.type == AD_EXPR_TOKEN_RSHIFT) {
            ad_expr_next_token(p);
            right = ad_expr_parse_sum(p);
            if (ad_expr_failed(p)) {
                return NAN;
            }
            if (right < 0 || 64 <= right) {
                ad_expr_set_error(p, "shift count out of range");
                return NAN;
            }
            left = (long long)left >> (long long)right;
            continue;
        }
        break;
    }
    return left;
}

static double ad_expr_parse_sum(ad_expr_parser *p) {
    double left;
    double right;
    left = ad_expr_parse_term(p);
    while (!ad_expr_failed(p)) {
        if (p->tok.type == AD_EXPR_TOKEN_PLUS) {
            ad_expr_next_token(p);
            right = ad_expr_parse_term(p);
            left += right;
            continue;
        }
        if (p->tok.type == AD_EXPR_TOKEN_MINUS) {
            ad_expr_next_token(p);
            right = ad_expr_parse_term(p);
            left -= right;
            continue;
        }
        break;
    }
    return left;
}

static double ad_expr_parse_term(ad_expr_parser *p) {
    double left;
    double right;
    left = ad_expr_parse_factor(p);
    while (!ad_expr_failed(p)) {
        if (p->tok.type == AD_EXPR_TOKEN_STAR) {
            ad_expr_next_token(p);
            right = ad_expr_parse_factor(p);
            left *= right;
            continue;
        }
        if (p->tok.type == AD_EXPR_TOKEN_SLASH) {
            ad_expr_next_token(p);
            right = ad_expr_parse_factor(p);
            if (right == 0.0) {
                ad_expr_set_error(p, "division by zero");
                return NAN;
            }
            left /= (1.0 * right);
            continue;
        }
        if (p->tok.type == AD_EXPR_TOKEN_PERCENT) {
            ad_expr_next_token(p);
            right = ad_expr_parse_factor(p);
            if (right == 0.0) {
                ad_expr_set_error(p, "modulo by zero");
                return NAN;
            }
            left = (double)(ad_expr_to_i64(left) % ad_expr_to_i64(right));
            continue;
        }
        break;
    }
    return left;
}

static double ad_expr_parse_factor(ad_expr_parser *p) {
    if (ad_expr_accept(p, AD_EXPR_TOKEN_PLUS)) {
        return ad_expr_parse_factor(p);
    }
    if (ad_expr_accept(p, AD_EXPR_TOKEN_MINUS)) {
        return -ad_expr_parse_factor(p);
    }
    if (ad_expr_accept(p, AD_EXPR_TOKEN_TILDE)) {
        return (double)(~ad_expr_to_i64(ad_expr_parse_factor(p)));
    }
    return ad_expr_parse_power(p);
}

static double ad_expr_parse_power(ad_expr_parser *p) {
    return ad_expr_parse_primary(p);
}

static double ad_expr_parse_call_arguments(ad_expr_parser *p, const char *name) {
    double args[8];
    int argc;
    argc = 0;
    if (!ad_expr_expect(p, AD_EXPR_TOKEN_LPAREN, "expected '('") ) {
        return NAN;
    }
    if (p->tok.type != AD_EXPR_TOKEN_RPAREN) {
        for (;;) {
            if (argc >= (int)(sizeof(args) / sizeof(args[0]))) {
                ad_expr_set_error(p, "too many arguments");
                return NAN;
            }
            args[argc++] = ad_expr_parse_expression(p);
            if (ad_expr_failed(p)) {
                return NAN;
            }
            if (!ad_expr_accept(p, AD_EXPR_TOKEN_COMMA)) {
                break;
            }
        }
    }
    if (!ad_expr_expect(p, AD_EXPR_TOKEN_RPAREN, "expected ')'") ) {
        return NAN;
    }
    return ad_expr_function_call(p, name, args, argc);
}

static double ad_expr_parse_primary(ad_expr_parser *p) {
    return ad_expr_parse_atom(p);
}

static double ad_expr_parse_atom(ad_expr_parser *p) {
    double v;
    char name[64];
    int idx;

    if (p->tok.type == AD_EXPR_TOKEN_NUMBER) {
        v = p->tok.number;
        ad_expr_next_token(p);
        return v;
    }

    if (p->tok.type == AD_EXPR_TOKEN_BUFFER) {
        idx = (int)p->tok.integer;
        ad_expr_next_token(p);
        if (idx < 0 || p->sz <= idx) {
            ad_expr_set_error(p, "out of range");
            return NAN;
        }
        return (double)p->bytes[idx];
    }

    if (p->tok.type == AD_EXPR_TOKEN_NAME) {
        strncpy(name, p->tok.text, sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
        ad_expr_next_token(p);

        if (strcmp(name, "true") == 0) {
            return 1.0;
        }
        if (strcmp(name, "false") == 0) {
            return 0.0;
        }
        if (strcmp(name, "pi") == 0) {
            return 3.14159265358979323846;
        }
        if (strcmp(name, "e") == 0) {
            return 2.71828182845904523536;
        }
        if (p->tok.type == AD_EXPR_TOKEN_LPAREN) {
            return ad_expr_parse_call_arguments(p, name);
        }
        ad_expr_set_error(p, "unknown function");
        return NAN;
    }

    if (ad_expr_accept(p, AD_EXPR_TOKEN_LPAREN)) {
        v = ad_expr_parse_expression(p);
        if (ad_expr_failed(p)) {
            return NAN;
        }
        if (!ad_expr_expect(p, AD_EXPR_TOKEN_RPAREN, "expected ')'") ) {
            return NAN;
        }
        return v;
    }

    ad_expr_set_error(p, "unexpected token");
    return NAN;
}

double ad_expr_reduce(const uint8 *bytes, int sz, const char *expr, char **errorReturn) {
    ad_expr_parser p;
    double v;

    if (errorReturn != NULL) {
        *errorReturn = NULL;
    }
    if (bytes == NULL) {
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("bytes is NULL");
        }
        return NAN;
    }
    if (expr == NULL) {
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("expr is NULL");
        }
        return NAN;
    }
    if (sz < 0) {
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("sz is negative");
        }
        return NAN;
    }

    memset(&p, 0, sizeof(p));
    p.bytes = bytes;
    p.sz = sz;
    p.expr = expr;
    p.cur = expr;
    ad_expr_next_token(&p);
    v = ad_expr_parse_expression(&p);

    if (!ad_expr_failed(&p) && p.tok.type != AD_EXPR_TOKEN_EOF) {
        log_warn("'%s': unexpected trailing input", expr);
        return v;
    }

    if (p.error != NULL) {
        if (errorReturn != NULL) {
            *errorReturn = p.error;
        } else {
            free(p.error);
        }
        return NAN;
    }

    return v;
}
double ad_expr_reduce_buffer(Buffer * buffer, const char *expr, char **errorReturn) {
    return ad_expr_reduce(buffer->buffer, buffer->size, expr, errorReturn);
}