#include "libautodiag/expr.h"

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libautodiag/math.h"

typedef struct ad_expr_parser ad_expr_parser;
typedef struct ad_expr_ast ad_expr_ast;
typedef struct ad_expr_inv_ctx ad_expr_inv_ctx;

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

typedef enum {
    AD_EXPR_AST_NUMBER,
    AD_EXPR_AST_BUFFER,
    AD_EXPR_AST_NAME,
    AD_EXPR_AST_UNARY,
    AD_EXPR_AST_BINARY,
    AD_EXPR_AST_TERNARY,
    AD_EXPR_AST_CALL
} ad_expr_ast_type;

struct ad_expr_ast {
    ad_expr_ast_type type;
    ad_expr_token_type op;
    double number;
    int buffer_index;
    char *name;
    ad_expr_ast *a;
    ad_expr_ast *b;
    ad_expr_ast *c;
    ad_expr_ast **args;
    int argc;
};

struct ad_expr_parser {
    const byte *bytes;
    int sz;
    const char *expr;
    const char *cur;
    ad_expr_token tok;
    char *error;
};

struct ad_expr_inv_ctx {
    ad_expr_ast *root;
    int max_index;
    bool domains[16][256];
    bool assigned[16];
    byte values[16];
    long long nodes_visited;
    long long nodes_limit;
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

static char *ad_expr_strdup_printf(const char *fmt, ...) {
    va_list ap;
    va_list ap2;
    int n;
    char *s;

    va_start(ap, fmt);
    va_copy(ap2, ap);
    n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (n < 0) {
        va_end(ap2);
        return NULL;
    }

    s = (char *)malloc((size_t)n + 1);
    if (s == NULL) {
        va_end(ap2);
        return NULL;
    }

    vsnprintf(s, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    return s;
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

static void ad_expr_inv_set_error(ad_expr_inv_ctx *ctx, const char *fmt, ...) {
    char buf[256];
    va_list ap;
    if (ctx == NULL || ctx->error != NULL) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    ctx->error = ad_expr_strdup(buf);
    if (ctx->error == NULL) {
        ctx->error = ad_expr_strdup("out of memory");
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

static double ad_expr_strtod(const char *s, char **endptr) {
    const char *p = s;
    int sign = 1;
    double int_part = 0.0;
    double frac_part = 0.0;
    double frac_div = 1.0;
    double value;
    int any = 0;

    while (isspace((unsigned char)*p)) p++;

    if (*p == '+') p++;
    else if (*p == '-') {
        sign = -1;
        p++;
    }

    while (*p >= '0' && *p <= '9') {
        int_part = int_part * 10.0 + (*p - '0');
        p++;
        any = 1;
    }

    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9') {
            frac_part = frac_part * 10.0 + (*p - '0');
            frac_div *= 10.0;
            p++;
            any = 1;
        }
    }

    value = int_part + frac_part / frac_div;

    if ((*p == 'e' || *p == 'E') && any) {
        const char *exp_start = p;
        int exp_sign = 1;
        int exp_val = 0;
        int exp_any = 0;

        p++;
        if (*p == '+') p++;
        else if (*p == '-') {
            exp_sign = -1;
            p++;
        }

        while (*p >= '0' && *p <= '9') {
            exp_val = exp_val * 10 + (*p - '0');
            p++;
            exp_any = 1;
        }

        if (exp_any) {
            value *= pow(10.0, exp_sign * exp_val);
        } else {
            p = exp_start;
        }
    }

    if (!any) {
        if (endptr) *endptr = (char *)s;
        return 0.0;
    }

    if (endptr) *endptr = (char *)p;
    return sign * value;
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
        p->tok.number = ad_expr_strtod(s, &endptr);
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
        ad_expr_set_error(p, "out of range (1)");
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
            ad_expr_set_error(p, "out of range (2)");
            return NAN;
        }
        if (bit_index < 0 || 8 <= bit_index) {
            ad_expr_set_error(p, "bit index out of range (2)");
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
            ad_expr_set_error(p, "out of range (3)");
            return NAN;
        }
        if (start < 0 || 8 <= start) {
            ad_expr_set_error(p, "bits start out of range (3)");
            return NAN;
        }
        if (width < 1 || 8 < width || 8 < start + width) {
            ad_expr_set_error(p, "bits width out of range (3)");
            return NAN;
        }
        return (double)((p->bytes[off] >> start) & ((1U << width) - 1U));
    }
    ad_expr_set_error(p, "unknown function");
    return NAN;
}

static double ad_expr_ast_eval(ad_expr_ast *ast, const byte *bytes, int sz, char **errorReturn);

static ad_expr_ast *ad_expr_ast_new(ad_expr_ast_type type) {
    ad_expr_ast *ast = (ad_expr_ast *)calloc(1, sizeof(ad_expr_ast));
    if (ast != NULL) {
        ast->type = type;
    }
    return ast;
}

static void ad_expr_ast_free(ad_expr_ast *ast) {
    int i;
    if (ast == NULL) {
        return;
    }
    ad_expr_ast_free(ast->a);
    ad_expr_ast_free(ast->b);
    ad_expr_ast_free(ast->c);
    if (ast->args != NULL) {
        for (i = 0; i < ast->argc; i++) {
            ad_expr_ast_free(ast->args[i]);
        }
        free(ast->args);
    }
    free(ast->name);
    free(ast);
}

static ad_expr_ast *ad_expr_ast_number(double number) {
    ad_expr_ast *ast = ad_expr_ast_new(AD_EXPR_AST_NUMBER);
    if (ast != NULL) {
        ast->number = number;
    }
    return ast;
}

static ad_expr_ast *ad_expr_ast_buffer(int idx) {
    ad_expr_ast *ast = ad_expr_ast_new(AD_EXPR_AST_BUFFER);
    if (ast != NULL) {
        ast->buffer_index = idx;
    }
    return ast;
}

static ad_expr_ast *ad_expr_ast_name(const char *name) {
    ad_expr_ast *ast = ad_expr_ast_new(AD_EXPR_AST_NAME);
    if (ast != NULL) {
        ast->name = ad_expr_strdup(name);
        if (ast->name == NULL) {
            free(ast);
            return NULL;
        }
    }
    return ast;
}

static ad_expr_ast *ad_expr_ast_unary(ad_expr_token_type op, ad_expr_ast *a) {
    ad_expr_ast *ast = ad_expr_ast_new(AD_EXPR_AST_UNARY);
    if (ast != NULL) {
        ast->op = op;
        ast->a = a;
    }
    return ast;
}

static ad_expr_ast *ad_expr_ast_binary(ad_expr_token_type op, ad_expr_ast *a, ad_expr_ast *b) {
    ad_expr_ast *ast = ad_expr_ast_new(AD_EXPR_AST_BINARY);
    if (ast != NULL) {
        ast->op = op;
        ast->a = a;
        ast->b = b;
    }
    return ast;
}

static ad_expr_ast *ad_expr_ast_ternary(ad_expr_ast *a, ad_expr_ast *b, ad_expr_ast *c) {
    ad_expr_ast *ast = ad_expr_ast_new(AD_EXPR_AST_TERNARY);
    if (ast != NULL) {
        ast->a = a;
        ast->b = b;
        ast->c = c;
    }
    return ast;
}

static ad_expr_ast *ad_expr_ast_call(const char *name, ad_expr_ast **args, int argc) {
    ad_expr_ast *ast = ad_expr_ast_new(AD_EXPR_AST_CALL);
    if (ast != NULL) {
        ast->name = ad_expr_strdup(name);
        ast->args = args;
        ast->argc = argc;
        if (ast->name == NULL) {
            free(ast);
            return NULL;
        }
    }
    return ast;
}

static double ad_expr_ast_function_eval(const char *name, ad_expr_ast **args, int argc, const byte *bytes, int sz, char **errorReturn) {
    double vals[8];
    ad_expr_parser p;
    int i;

    if (argc > (int)(sizeof(vals) / sizeof(vals[0]))) {
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("too many arguments");
        }
        return NAN;
    }

    for (i = 0; i < argc; i++) {
        vals[i] = ad_expr_ast_eval(args[i], bytes, sz, errorReturn);
        if (errorReturn != NULL && *errorReturn != NULL) {
            return NAN;
        }
    }

    memset(&p, 0, sizeof(p));
    p.bytes = bytes;
    p.sz = sz;
    p.expr = "";
    p.cur = "";
    return ad_expr_function_call(&p, name, vals, argc);
}

static double ad_expr_ast_eval(ad_expr_ast *ast, const byte *bytes, int sz, char **errorReturn) {
    double av;
    double bv;
    double cv;

    if (ast == NULL) {
        if (errorReturn != NULL && *errorReturn == NULL) {
            *errorReturn = ad_expr_strdup("invalid AST");
        }
        return NAN;
    }

    switch (ast->type) {
        case AD_EXPR_AST_NUMBER:
            return ast->number;

        case AD_EXPR_AST_BUFFER:
            if (ast->buffer_index < 0 || sz <= ast->buffer_index) {
                return 0.0;
            }
            return (double)bytes[ast->buffer_index];

        case AD_EXPR_AST_NAME:
            if (strcmp(ast->name, "true") == 0) return 1.0;
            if (strcmp(ast->name, "false") == 0) return 0.0;
            if (strcmp(ast->name, "pi") == 0) return 3.14159265358979323846;
            if (strcmp(ast->name, "e") == 0) return 2.71828182845904523536;
            if (errorReturn != NULL && *errorReturn == NULL) {
                *errorReturn = ad_expr_strdup("unknown constant");
            }
            return NAN;

        case AD_EXPR_AST_UNARY:
            av = ad_expr_ast_eval(ast->a, bytes, sz, errorReturn);
            if (errorReturn != NULL && *errorReturn != NULL) return NAN;
            if (ast->op == AD_EXPR_TOKEN_PLUS) return av;
            if (ast->op == AD_EXPR_TOKEN_MINUS) return -av;
            if (ast->op == AD_EXPR_TOKEN_TILDE) return (double)(~ad_expr_to_i64(av));
            if (ast->op == AD_EXPR_TOKEN_BANG) return ad_expr_is_true(av) ? 0.0 : 1.0;
            if (errorReturn != NULL && *errorReturn == NULL) {
                *errorReturn = ad_expr_strdup("unknown unary op");
            }
            return NAN;

        case AD_EXPR_AST_BINARY:
            av = ad_expr_ast_eval(ast->a, bytes, sz, errorReturn);
            if (errorReturn != NULL && *errorReturn != NULL) return NAN;
            bv = ad_expr_ast_eval(ast->b, bytes, sz, errorReturn);
            if (errorReturn != NULL && *errorReturn != NULL) return NAN;
            switch (ast->op) {
                case AD_EXPR_TOKEN_PLUS: return av + bv;
                case AD_EXPR_TOKEN_MINUS: return av - bv;
                case AD_EXPR_TOKEN_STAR: return av * bv;
                case AD_EXPR_TOKEN_SLASH:
                    if (bv == 0.0) {
                        if (errorReturn != NULL && *errorReturn == NULL) *errorReturn = ad_expr_strdup("division by zero");
                        return NAN;
                    }
                    return av / bv;
                case AD_EXPR_TOKEN_PERCENT:
                    if (bv == 0.0) {
                        if (errorReturn != NULL && *errorReturn == NULL) *errorReturn = ad_expr_strdup("modulo by zero");
                        return NAN;
                    }
                    return (double)(ad_expr_to_i64(av) % ad_expr_to_i64(bv));
                case AD_EXPR_TOKEN_AMP: return (double)(ad_expr_to_i64(av) & ad_expr_to_i64(bv));
                case AD_EXPR_TOKEN_PIPE: return (double)(ad_expr_to_i64(av) | ad_expr_to_i64(bv));
                case AD_EXPR_TOKEN_CARET: return (double)(ad_expr_to_i64(av) ^ ad_expr_to_i64(bv));
                case AD_EXPR_TOKEN_LSHIFT:
                    if (bv < 0.0 || 64.0 <= bv) {
                        if (errorReturn != NULL && *errorReturn == NULL) *errorReturn = ad_expr_strdup("shift count out of range");
                        return NAN;
                    }
                    return (double)(ad_expr_to_i64(av) << ad_expr_to_i64(bv));
                case AD_EXPR_TOKEN_RSHIFT:
                    if (bv < 0.0 || 64.0 <= bv) {
                        if (errorReturn != NULL && *errorReturn == NULL) *errorReturn = ad_expr_strdup("shift count out of range");
                        return NAN;
                    }
                    return (double)(ad_expr_to_i64(av) >> ad_expr_to_i64(bv));
                case AD_EXPR_TOKEN_EQEQ: return av == bv ? 1.0 : 0.0;
                case AD_EXPR_TOKEN_NEQ: return av != bv ? 1.0 : 0.0;
                case AD_EXPR_TOKEN_LT: return av < bv ? 1.0 : 0.0;
                case AD_EXPR_TOKEN_LTE: return av <= bv ? 1.0 : 0.0;
                case AD_EXPR_TOKEN_GT: return av > bv ? 1.0 : 0.0;
                case AD_EXPR_TOKEN_GTE: return av >= bv ? 1.0 : 0.0;
                case AD_EXPR_TOKEN_ANDAND: return (ad_expr_is_true(av) && ad_expr_is_true(bv)) ? 1.0 : 0.0;
                case AD_EXPR_TOKEN_OROR: return (ad_expr_is_true(av) || ad_expr_is_true(bv)) ? 1.0 : 0.0;
                default:
                    if (errorReturn != NULL && *errorReturn == NULL) *errorReturn = ad_expr_strdup("unknown binary op");
                    return NAN;
            }

        case AD_EXPR_AST_TERNARY:
            av = ad_expr_ast_eval(ast->a, bytes, sz, errorReturn);
            if (errorReturn != NULL && *errorReturn != NULL) return NAN;
            if (ad_expr_is_true(av)) {
                return ad_expr_ast_eval(ast->b, bytes, sz, errorReturn);
            }
            return ad_expr_ast_eval(ast->c, bytes, sz, errorReturn);

        case AD_EXPR_AST_CALL:
            return ad_expr_ast_function_eval(ast->name, ast->args, ast->argc, bytes, sz, errorReturn);
    }

    if (errorReturn != NULL && *errorReturn == NULL) {
        *errorReturn = ad_expr_strdup("unknown AST node");
    }
    return NAN;
}

static bool ad_expr_ast_is_number(ad_expr_ast *ast, double *out) {
    if (ast != NULL && ast->type == AD_EXPR_AST_NUMBER) {
        if (out != NULL) {
            *out = ast->number;
        }
        return true;
    }
    return false;
}

static int ad_expr_ast_max_buffer_index(ad_expr_ast *ast) {
    int ma;
    int mb;
    int mc;
    int i;
    int mr;

    if (ast == NULL) {
        return -1;
    }

    if (ast->type == AD_EXPR_AST_BUFFER) {
        return ast->buffer_index;
    }

    ma = ad_expr_ast_max_buffer_index(ast->a);
    mb = ad_expr_ast_max_buffer_index(ast->b);
    mc = ad_expr_ast_max_buffer_index(ast->c);
    mr = ma;
    if (mr < mb) mr = mb;
    if (mr < mc) mr = mc;

    if (ast->args != NULL) {
        for (i = 0; i < ast->argc; i++) {
            int mi = ad_expr_ast_max_buffer_index(ast->args[i]);
            if (mr < mi) mr = mi;
        }
    }

    return mr;
}

static bool ad_expr_domain_allow_only(ad_expr_inv_ctx *ctx, int idx, int value) {
    int i;
    bool any = false;

    if (idx < 0 || 16 <= idx) {
        ad_expr_inv_set_error(ctx, "buffer index too large for inversion");
        return false;
    }

    for (i = 0; i < 256; i++) {
        ctx->domains[idx][i] = (i == value);
        if (ctx->domains[idx][i]) {
            any = true;
        }
    }

    if (!any) {
        ad_expr_inv_set_error(ctx, "empty domain");
        return false;
    }

    return true;
}

static bool ad_expr_domain_intersect_mask_eq(ad_expr_inv_ctx *ctx, int idx, int mask, int expected) {
    int i;
    bool any = false;

    if (idx < 0 || 16 <= idx) {
        ad_expr_inv_set_error(ctx, "buffer index too large for inversion");
        return false;
    }

    for (i = 0; i < 256; i++) {
        if (ctx->domains[idx][i] && ((i & mask) != expected)) {
            ctx->domains[idx][i] = false;
        }
        if (ctx->domains[idx][i]) {
            any = true;
        }
    }

    if (!any) {
        ad_expr_inv_set_error(ctx, "empty domain");
        return false;
    }

    return true;
}

static bool ad_expr_domain_intersect_shifted_mask(ad_expr_inv_ctx *ctx, int idx, int shift, int mask, int expected) {
    int i;
    bool any = false;

    if (idx < 0 || 16 <= idx) {
        ad_expr_inv_set_error(ctx, "buffer index too large for inversion");
        return false;
    }

    for (i = 0; i < 256; i++) {
        if (ctx->domains[idx][i] && ((((i >> shift) & mask)) != expected)) {
            ctx->domains[idx][i] = false;
        }
        if (ctx->domains[idx][i]) {
            any = true;
        }
    }

    if (!any) {
        ad_expr_inv_set_error(ctx, "empty domain");
        return false;
    }

    return true;
}
static bool ad_expr_ast_extract_selected_byte(ad_expr_ast *ast, int *idx_true, int *idx_false) {
    if (ast == NULL || ast->type != AD_EXPR_AST_TERNARY) {
        return false;
    }
    if (ast->a == NULL || ast->a->type != AD_EXPR_AST_BINARY || ast->a->op != AD_EXPR_TOKEN_EQEQ) {
        return false;
    }
    if (ast->a->a == NULL || ast->a->a->type != AD_EXPR_AST_BUFFER || ast->a->a->buffer_index != 0) {
        return false;
    }
    if (ast->a->b == NULL || ast->a->b->type != AD_EXPR_AST_NUMBER || ast->a->b->number != 0x41) {
        return false;
    }
    if (ast->b == NULL || ast->b->type != AD_EXPR_AST_BUFFER) {
        return false;
    }
    if (ast->c == NULL || ast->c->type != AD_EXPR_AST_BUFFER) {
        return false;
    }
    if (idx_true != NULL) *idx_true = ast->b->buffer_index;
    if (idx_false != NULL) *idx_false = ast->c->buffer_index;
    return true;
}

static bool ad_expr_ast_pick_signal_offset(ad_expr_ast *ast, int *signal_offset_return) {
    int idx_true;
    int idx_false;

    if (!ad_expr_ast_extract_selected_byte(ast, &idx_true, &idx_false)) {
        return false;
    }

    if (signal_offset_return != NULL) {
        *signal_offset_return = idx_true < idx_false ? idx_true : idx_false;
    }
    return true;
}
static bool ad_expr_ast_pick_signal_range(ad_expr_ast *ast, int *signal_start_offset_return, int *signal_end_offset_return) {
    int idx_true;
    int idx_false;

    if (ast == NULL) {
        return false;
    }

    if (ad_expr_ast_extract_selected_byte(ast, &idx_true, &idx_false)) {
        if (signal_start_offset_return != NULL) {
            *signal_start_offset_return = idx_true < idx_false ? idx_true : idx_false;
        }
        if (signal_end_offset_return != NULL) {
            *signal_end_offset_return = idx_true < idx_false ? idx_false : idx_true;
        }
        return true;
    }

    if (ast->type == AD_EXPR_AST_BINARY &&
        ast->op == AD_EXPR_TOKEN_SLASH &&
        ast->a != NULL &&
        ast->a->type == AD_EXPR_AST_BINARY &&
        ast->a->op == AD_EXPR_TOKEN_PLUS &&
        ast->a->a != NULL &&
        ast->a->a->type == AD_EXPR_AST_BINARY &&
        ast->a->a->op == AD_EXPR_TOKEN_STAR) {
        int hi_true;
        int hi_false;
        int lo_true;
        int lo_false;
        double mul;

        if (ad_expr_ast_extract_selected_byte(ast->a->a->a, &hi_true, &hi_false) &&
            ad_expr_ast_is_number(ast->a->a->b, &mul) &&
            mul == 256.0 &&
            ad_expr_ast_extract_selected_byte(ast->a->b, &lo_true, &lo_false)) {
            int start_i = hi_true < hi_false ? hi_true : hi_false;
            int end_i = lo_true < lo_false ? lo_false : lo_true;
            if (signal_start_offset_return != NULL) {
                *signal_start_offset_return = start_i;
            }
            if (signal_end_offset_return != NULL) {
                *signal_end_offset_return = end_i;
            }
            return true;
        }
    }

    return false;
}

static bool ad_expr_ast_narrow_domains(ad_expr_inv_ctx *ctx, ad_expr_ast *ast, double target);

static bool ad_expr_ast_narrow_binary(ad_expr_inv_ctx *ctx, ad_expr_ast *ast, double target) {
    double rhs;
    double lhsn;
    int idx_true;
    int idx_false;
    int raw;
    int hi;
    int lo;

    if (ast == NULL || ast->type != AD_EXPR_AST_BINARY) {
        return true;
    }

    if (ast->op == AD_EXPR_TOKEN_AMP && ad_expr_ast_extract_selected_byte(ast->a, &idx_true, &idx_false) && ad_expr_ast_is_number(ast->b, &rhs)) {
        raw = ((int)llround(target)) & ((int)rhs);
        if (!ad_expr_domain_intersect_mask_eq(ctx, idx_true, (int)rhs, raw)) return false;
        if (!ad_expr_domain_intersect_mask_eq(ctx, idx_false, (int)rhs, raw)) return false;
        return true;
    }

    if (ast->op == AD_EXPR_TOKEN_AMP &&
        ast->a != NULL &&
        ast->a->type == AD_EXPR_AST_BINARY &&
        ast->a->op == AD_EXPR_TOKEN_RSHIFT &&
        ad_expr_ast_extract_selected_byte(ast->a->a, &idx_true, &idx_false) &&
        ad_expr_ast_is_number(ast->a->b, &lhsn) &&
        ad_expr_ast_is_number(ast->b, &rhs)) {
        raw = ((int)llround(target)) & ((int)rhs);
        if (!ad_expr_domain_intersect_shifted_mask(ctx, idx_true, (int)lhsn, (int)rhs, raw)) return false;
        if (!ad_expr_domain_intersect_shifted_mask(ctx, idx_false, (int)lhsn, (int)rhs, raw)) return false;
        return true;
    }

    if (ast->op == AD_EXPR_TOKEN_MINUS &&
        ast->a != NULL &&
        ast->a->type == AD_EXPR_AST_BINARY &&
        ast->a->op == AD_EXPR_TOKEN_SLASH &&
        ad_expr_ast_extract_selected_byte(ast->a->a, &idx_true, &idx_false) &&
        ad_expr_ast_is_number(ast->a->b, &lhsn) &&
        ad_expr_ast_is_number(ast->b, &rhs)) {
        raw = (int)llround((target + rhs) * lhsn);
        if (raw < 0 || 255 < raw) {
            ad_expr_inv_set_error(ctx, "target out of byte range");
            return false;
        }
        if (!ad_expr_domain_allow_only(ctx, idx_true, raw)) return false;
        if (!ad_expr_domain_allow_only(ctx, idx_false, raw)) return false;
        return true;
    }

    if (ast->op == AD_EXPR_TOKEN_SLASH &&
        ast->a != NULL &&
        ast->a->type == AD_EXPR_AST_BINARY &&
        ast->a->op == AD_EXPR_TOKEN_PLUS &&
        ad_expr_ast_is_number(ast->a->a, &lhsn) == false &&
        ad_expr_ast_is_number(ast->b, &rhs)) {
        ad_expr_ast *left = ast->a->a;
        ad_expr_ast *right = ast->a->b;
        double n256;

        if (ad_expr_ast_is_number(right, &n256) && n256 == 0.0) {
        }

        if (left != NULL &&
            left->type == AD_EXPR_AST_BINARY &&
            left->op == AD_EXPR_TOKEN_STAR &&
            ad_expr_ast_extract_selected_byte(left->a, &idx_true, &idx_false) &&
            ad_expr_ast_is_number(left->b, &lhsn) &&
            lhsn == 256.0 &&
            ad_expr_ast_extract_selected_byte(right, NULL, NULL)) {
            int idx_true_lo;
            int idx_false_lo;
            int value;
            if (!ad_expr_ast_extract_selected_byte(right, &idx_true_lo, &idx_false_lo)) {
                return true;
            }
            value = (int)llround(target * rhs);
            if (value < 0 || 65535 < value) {
                ad_expr_inv_set_error(ctx, "target out of 16-bit range");
                return false;
            }
            hi = (value >> 8) & 0xFF;
            lo = value & 0xFF;
            if (!ad_expr_domain_allow_only(ctx, idx_true, hi)) return false;
            if (!ad_expr_domain_allow_only(ctx, idx_false, hi)) return false;
            if (!ad_expr_domain_allow_only(ctx, idx_true_lo, lo)) return false;
            if (!ad_expr_domain_allow_only(ctx, idx_false_lo, lo)) return false;
            return true;
        }
    }

    return true;
}

static bool ad_expr_ast_narrow_domains(ad_expr_inv_ctx *ctx, ad_expr_ast *ast, double target) {
    if (ctx == NULL || ast == NULL) {
        return true;
    }
    if (!ad_expr_ast_narrow_binary(ctx, ast, target)) {
        return false;
    }
    return true;
}

static int ad_expr_domain_count(ad_expr_inv_ctx *ctx, int idx) {
    int i;
    int count = 0;
    for (i = 0; i < 256; i++) {
        if (ctx->domains[idx][i]) {
            count++;
        }
    }
    return count;
}

static int ad_expr_pick_next_var(ad_expr_inv_ctx *ctx) {
    int idx = -1;
    int best = 1000000;
    int i;
    for (i = 0; i <= ctx->max_index; i++) {
        int count;
        if (ctx->assigned[i]) {
            continue;
        }
        count = ad_expr_domain_count(ctx, i);
        if (count < best) {
            best = count;
            idx = i;
        }
    }
    return idx;
}

static bool ad_expr_inv_eval_current(ad_expr_inv_ctx *ctx, double target) {
    byte buf[16];
    char *err = NULL;
    double v;
    int i;

    for (i = 0; i <= ctx->max_index; i++) {
        buf[i] = ctx->values[i];
    }

    v = ad_expr_ast_eval(ctx->root, buf, ctx->max_index + 1, &err);
    if (err != NULL) {
        free(err);
        return false;
    }
    return !isnan(v) && fabs(v - target) < 1e-9;
}

static bool ad_expr_inv_search(ad_expr_inv_ctx *ctx, double target) {
    int idx;
    int i;

    if (ctx->nodes_limit <= ctx->nodes_visited) {
        ad_expr_inv_set_error(ctx, "inversion search limit reached");
        return false;
    }
    ctx->nodes_visited++;

    idx = ad_expr_pick_next_var(ctx);
    if (idx < 0) {
        return ad_expr_inv_eval_current(ctx, target);
    }

    ctx->assigned[idx] = true;
    for (i = 0; i < 256; i++) {
        if (!ctx->domains[idx][i]) {
            continue;
        }
        ctx->values[idx] = (byte)i;
        if (ad_expr_inv_search(ctx, target)) {
            return true;
        }
        if (ctx->error != NULL) {
            break;
        }
    }
    ctx->assigned[idx] = false;
    return false;
}

static Buffer *ad_expr_inv_build_buffer(ad_expr_inv_ctx *ctx) {
    Buffer *out;
    int i;
    out = ad_buffer_new();
    if (out == NULL) {
        return NULL;
    }
    for (i = 0; i <= ctx->max_index; i++) {
        ad_buffer_append_byte(out, ctx->values[i]);
    }
    return out;
}

static ad_expr_ast *ad_expr_parse_expression_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_disjunction_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_conjunction_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_inversion_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_comparison_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_bitwise_or_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_bitwise_xor_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_bitwise_and_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_shift_expr_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_sum_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_term_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_factor_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_power_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_primary_ast(ad_expr_parser *p);
static ad_expr_ast *ad_expr_parse_atom_ast(ad_expr_parser *p);

static ad_expr_ast *ad_expr_parse_expression_ast(ad_expr_parser *p) {
    ad_expr_ast *cond;
    ad_expr_ast *yes;
    ad_expr_ast *no;
    cond = ad_expr_parse_disjunction_ast(p);
    if (ad_expr_failed(p)) {
        return NULL;
    }
    if (ad_expr_accept(p, AD_EXPR_TOKEN_QUESTION)) {
        yes = ad_expr_parse_expression_ast(p);
        if (ad_expr_failed(p)) {
            ad_expr_ast_free(cond);
            return NULL;
        }
        if (!ad_expr_expect(p, AD_EXPR_TOKEN_COLON, "expected ':'")) {
            ad_expr_ast_free(cond);
            ad_expr_ast_free(yes);
            return NULL;
        }
        no = ad_expr_parse_expression_ast(p);
        if (ad_expr_failed(p)) {
            ad_expr_ast_free(cond);
            ad_expr_ast_free(yes);
            return NULL;
        }
        return ad_expr_ast_ternary(cond, yes, no);
    }
    return cond;
}

static ad_expr_ast *ad_expr_parse_disjunction_ast(ad_expr_parser *p) {
    ad_expr_ast *v;
    ad_expr_ast *rhs;
    v = ad_expr_parse_conjunction_ast(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_OROR) {
        ad_expr_next_token(p);
        rhs = ad_expr_parse_conjunction_ast(p);
        if (ad_expr_failed(p)) {
            ad_expr_ast_free(v);
            return NULL;
        }
        v = ad_expr_ast_binary(AD_EXPR_TOKEN_OROR, v, rhs);
    }
    return v;
}

static ad_expr_ast *ad_expr_parse_conjunction_ast(ad_expr_parser *p) {
    ad_expr_ast *v;
    ad_expr_ast *rhs;
    v = ad_expr_parse_inversion_ast(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_ANDAND) {
        ad_expr_next_token(p);
        rhs = ad_expr_parse_inversion_ast(p);
        if (ad_expr_failed(p)) {
            ad_expr_ast_free(v);
            return NULL;
        }
        v = ad_expr_ast_binary(AD_EXPR_TOKEN_ANDAND, v, rhs);
    }
    return v;
}

static ad_expr_ast *ad_expr_parse_inversion_ast(ad_expr_parser *p) {
    ad_expr_ast *v;
    if (ad_expr_accept(p, AD_EXPR_TOKEN_BANG)) {
        v = ad_expr_parse_inversion_ast(p);
        if (ad_expr_failed(p)) {
            return NULL;
        }
        return ad_expr_ast_unary(AD_EXPR_TOKEN_BANG, v);
    }
    return ad_expr_parse_comparison_ast(p);
}

static ad_expr_ast *ad_expr_parse_comparison_ast(ad_expr_parser *p) {
    ad_expr_ast *left;
    ad_expr_ast *right;
    ad_expr_token_type op;
    left = ad_expr_parse_bitwise_or_ast(p);
    while (!ad_expr_failed(p)) {
        switch (p->tok.type) {
            case AD_EXPR_TOKEN_EQEQ:
            case AD_EXPR_TOKEN_NEQ:
            case AD_EXPR_TOKEN_LTE:
            case AD_EXPR_TOKEN_LT:
            case AD_EXPR_TOKEN_GTE:
            case AD_EXPR_TOKEN_GT:
                op = p->tok.type;
                ad_expr_next_token(p);
                right = ad_expr_parse_bitwise_or_ast(p);
                if (ad_expr_failed(p)) {
                    ad_expr_ast_free(left);
                    return NULL;
                }
                left = ad_expr_ast_binary(op, left, right);
                break;
            default:
                return left;
        }
    }
    return left;
}

static ad_expr_ast *ad_expr_parse_bitwise_or_ast(ad_expr_parser *p) {
    ad_expr_ast *left;
    ad_expr_ast *right;
    left = ad_expr_parse_bitwise_xor_ast(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_PIPE) {
        ad_expr_next_token(p);
        right = ad_expr_parse_bitwise_xor_ast(p);
        if (ad_expr_failed(p)) {
            ad_expr_ast_free(left);
            return NULL;
        }
        left = ad_expr_ast_binary(AD_EXPR_TOKEN_PIPE, left, right);
    }
    return left;
}

static ad_expr_ast *ad_expr_parse_bitwise_xor_ast(ad_expr_parser *p) {
    ad_expr_ast *left;
    ad_expr_ast *right;
    left = ad_expr_parse_bitwise_and_ast(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_CARET) {
        ad_expr_next_token(p);
        right = ad_expr_parse_bitwise_and_ast(p);
        if (ad_expr_failed(p)) {
            ad_expr_ast_free(left);
            return NULL;
        }
        left = ad_expr_ast_binary(AD_EXPR_TOKEN_CARET, left, right);
    }
    return left;
}

static ad_expr_ast *ad_expr_parse_bitwise_and_ast(ad_expr_parser *p) {
    ad_expr_ast *left;
    ad_expr_ast *right;
    left = ad_expr_parse_shift_expr_ast(p);
    while (!ad_expr_failed(p) && p->tok.type == AD_EXPR_TOKEN_AMP) {
        ad_expr_next_token(p);
        right = ad_expr_parse_shift_expr_ast(p);
        if (ad_expr_failed(p)) {
            ad_expr_ast_free(left);
            return NULL;
        }
        left = ad_expr_ast_binary(AD_EXPR_TOKEN_AMP, left, right);
    }
    return left;
}

static ad_expr_ast *ad_expr_parse_shift_expr_ast(ad_expr_parser *p) {
    ad_expr_ast *left;
    ad_expr_ast *right;
    ad_expr_token_type op;
    left = ad_expr_parse_sum_ast(p);
    while (!ad_expr_failed(p)) {
        if (p->tok.type == AD_EXPR_TOKEN_LSHIFT || p->tok.type == AD_EXPR_TOKEN_RSHIFT) {
            op = p->tok.type;
            ad_expr_next_token(p);
            right = ad_expr_parse_sum_ast(p);
            if (ad_expr_failed(p)) {
                ad_expr_ast_free(left);
                return NULL;
            }
            left = ad_expr_ast_binary(op, left, right);
            continue;
        }
        break;
    }
    return left;
}

static ad_expr_ast *ad_expr_parse_sum_ast(ad_expr_parser *p) {
    ad_expr_ast *left;
    ad_expr_ast *right;
    ad_expr_token_type op;
    left = ad_expr_parse_term_ast(p);
    while (!ad_expr_failed(p)) {
        if (p->tok.type == AD_EXPR_TOKEN_PLUS || p->tok.type == AD_EXPR_TOKEN_MINUS) {
            op = p->tok.type;
            ad_expr_next_token(p);
            right = ad_expr_parse_term_ast(p);
            if (ad_expr_failed(p)) {
                ad_expr_ast_free(left);
                return NULL;
            }
            left = ad_expr_ast_binary(op, left, right);
            continue;
        }
        break;
    }
    return left;
}

static ad_expr_ast *ad_expr_parse_term_ast(ad_expr_parser *p) {
    ad_expr_ast *left;
    ad_expr_ast *right;
    ad_expr_token_type op;
    left = ad_expr_parse_factor_ast(p);
    while (!ad_expr_failed(p)) {
        if (p->tok.type == AD_EXPR_TOKEN_STAR || p->tok.type == AD_EXPR_TOKEN_SLASH || p->tok.type == AD_EXPR_TOKEN_PERCENT) {
            op = p->tok.type;
            ad_expr_next_token(p);
            right = ad_expr_parse_factor_ast(p);
            if (ad_expr_failed(p)) {
                ad_expr_ast_free(left);
                return NULL;
            }
            left = ad_expr_ast_binary(op, left, right);
            continue;
        }
        break;
    }
    return left;
}

static ad_expr_ast *ad_expr_parse_factor_ast(ad_expr_parser *p) {
    if (ad_expr_accept(p, AD_EXPR_TOKEN_PLUS)) {
        return ad_expr_ast_unary(AD_EXPR_TOKEN_PLUS, ad_expr_parse_factor_ast(p));
    }
    if (ad_expr_accept(p, AD_EXPR_TOKEN_MINUS)) {
        return ad_expr_ast_unary(AD_EXPR_TOKEN_MINUS, ad_expr_parse_factor_ast(p));
    }
    if (ad_expr_accept(p, AD_EXPR_TOKEN_TILDE)) {
        return ad_expr_ast_unary(AD_EXPR_TOKEN_TILDE, ad_expr_parse_factor_ast(p));
    }
    return ad_expr_parse_power_ast(p);
}

static ad_expr_ast *ad_expr_parse_power_ast(ad_expr_parser *p) {
    return ad_expr_parse_primary_ast(p);
}

static ad_expr_ast *ad_expr_parse_call_arguments_ast(ad_expr_parser *p, const char *name) {
    ad_expr_ast **args = NULL;
    int argc = 0;
    int cap = 0;

    if (!ad_expr_expect(p, AD_EXPR_TOKEN_LPAREN, "expected '('")) {
        return NULL;
    }

    if (p->tok.type != AD_EXPR_TOKEN_RPAREN) {
        for (;;) {
            ad_expr_ast *arg;
            if (argc == cap) {
                int new_cap = cap == 0 ? 4 : cap * 2;
                ad_expr_ast **new_args = (ad_expr_ast **)realloc(args, sizeof(ad_expr_ast *) * (size_t)new_cap);
                if (new_args == NULL) {
                    int i;
                    for (i = 0; i < argc; i++) {
                        ad_expr_ast_free(args[i]);
                    }
                    free(args);
                    ad_expr_set_error(p, "out of memory");
                    return NULL;
                }
                args = new_args;
                cap = new_cap;
            }
            arg = ad_expr_parse_expression_ast(p);
            if (ad_expr_failed(p)) {
                int i;
                for (i = 0; i < argc; i++) {
                    ad_expr_ast_free(args[i]);
                }
                free(args);
                return NULL;
            }
            args[argc++] = arg;
            if (!ad_expr_accept(p, AD_EXPR_TOKEN_COMMA)) {
                break;
            }
        }
    }

    if (!ad_expr_expect(p, AD_EXPR_TOKEN_RPAREN, "expected ')'")) {
        int i;
        for (i = 0; i < argc; i++) {
            ad_expr_ast_free(args[i]);
        }
        free(args);
        return NULL;
    }

    return ad_expr_ast_call(name, args, argc);
}

static ad_expr_ast *ad_expr_parse_primary_ast(ad_expr_parser *p) {
    return ad_expr_parse_atom_ast(p);
}

static ad_expr_ast *ad_expr_parse_atom_ast(ad_expr_parser *p) {
    ad_expr_ast *v;
    char name[64];
    int idx;

    if (p->tok.type == AD_EXPR_TOKEN_NUMBER) {
        v = ad_expr_ast_number(p->tok.number);
        ad_expr_next_token(p);
        return v;
    }

    if (p->tok.type == AD_EXPR_TOKEN_BUFFER) {
        idx = (int)p->tok.integer;
        ad_expr_next_token(p);
        return ad_expr_ast_buffer(idx);
    }

    if (p->tok.type == AD_EXPR_TOKEN_NAME) {
        strncpy(name, p->tok.text, sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
        ad_expr_next_token(p);
        if (p->tok.type == AD_EXPR_TOKEN_LPAREN) {
            return ad_expr_parse_call_arguments_ast(p, name);
        }
        return ad_expr_ast_name(name);
    }

    if (ad_expr_accept(p, AD_EXPR_TOKEN_LPAREN)) {
        v = ad_expr_parse_expression_ast(p);
        if (ad_expr_failed(p)) {
            return NULL;
        }
        if (!ad_expr_expect(p, AD_EXPR_TOKEN_RPAREN, "expected ')'")) {
            ad_expr_ast_free(v);
            return NULL;
        }
        return v;
    }

    ad_expr_set_error(p, "unexpected token");
    return NULL;
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
                ad_expr_set_error(p, "shift count out of range (4)");
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
                ad_expr_set_error(p, "shift count out of range (5)");
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
    if (!ad_expr_expect(p, AD_EXPR_TOKEN_LPAREN, "expected '('")) {
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
    if (!ad_expr_expect(p, AD_EXPR_TOKEN_RPAREN, "expected ')'")) {
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
            return 0.0;
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
        if (!ad_expr_expect(p, AD_EXPR_TOKEN_RPAREN, "expected ')'")) {
            return NAN;
        }
        return v;
    }

    ad_expr_set_error(p, "unexpected token");
    return NAN;
}

double ad_expr_reduce(const byte *bytes, int sz, const char *expr, char **errorReturn) {
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

double ad_expr_reduce_buffer(Buffer *buffer, const char *expr, char **errorReturn) {
    return ad_expr_reduce(buffer->buffer, buffer->size, expr, errorReturn);
}

static ad_expr_ast *ad_expr_parse_ast(const char *expr, char **errorReturn) {
    ad_expr_parser p;
    ad_expr_ast *root;

    if (errorReturn != NULL) {
        *errorReturn = NULL;
    }

    memset(&p, 0, sizeof(p));
    p.expr = expr;
    p.cur = expr;
    ad_expr_next_token(&p);
    root = ad_expr_parse_expression_ast(&p);

    if (!ad_expr_failed(&p) && p.tok.type != AD_EXPR_TOKEN_EOF) {
        ad_expr_ast_free(root);
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("unexpected trailing input");
        }
        return NULL;
    }

    if (p.error != NULL) {
        if (errorReturn != NULL) {
            *errorReturn = p.error;
        } else {
            free(p.error);
        }
        ad_expr_ast_free(root);
        return NULL;
    }

    return root;
}

Buffer *ad_expr_reduce_invert(double target, const char *expr, int *signal_start_offset_return, char **errorReturn) {    ad_expr_inv_ctx ctx;
    ad_expr_ast *root;
    int i;
    Buffer *out;

    if (errorReturn != NULL) {
        *errorReturn = NULL;
    }

    if (expr == NULL) {
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("expr is NULL");
        }
        return NULL;
    }

    root = ad_expr_parse_ast(expr, errorReturn);
    if (root == NULL) {
        return NULL;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.root = root;
    ctx.max_index = ad_expr_ast_max_buffer_index(root);
    ctx.nodes_limit = 1000000;

    if (ctx.max_index < 0) {
        ad_expr_ast_free(root);
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("expression has no buffer references");
        }
        return NULL;
    }

    if (15 < ctx.max_index) {
        ad_expr_ast_free(root);
        if (errorReturn != NULL) {
            *errorReturn = ad_expr_strdup("too many buffer references for inversion");
        }
        return NULL;
    }

    for (i = 0; i < 16; i++) {
        int j;
        for (j = 0; j < 256; j++) {
            ctx.domains[i][j] = true;
        }
    }

    if (!ad_expr_ast_narrow_domains(&ctx, root, target)) {
        ad_expr_ast_free(root);
        if (errorReturn != NULL) {
            *errorReturn = ctx.error != NULL ? ctx.error : ad_expr_strdup("failed to narrow domains");
        } else {
            free(ctx.error);
        }
        return NULL;
    }

    if (!ad_expr_inv_search(&ctx, target)) {
        ad_expr_ast_free(root);
        if (errorReturn != NULL) {
            if (ctx.error != NULL) {
                *errorReturn = ctx.error;
            } else {
                *errorReturn = ad_expr_strdup("no satisfying payload found");
            }
        } else {
            free(ctx.error);
        }
        return NULL;
    }

    out = ad_expr_inv_build_buffer(&ctx);
    ad_expr_ast_free(root);
    free(ctx.error);

    if (out == NULL && errorReturn != NULL) {
        *errorReturn = ad_expr_strdup("out of memory");
    }

    return out;
}