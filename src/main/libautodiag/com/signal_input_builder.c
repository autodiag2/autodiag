#include "libautodiag/com/signal_input_builder.h"

char *ad_signal_input_strdup(const char *s) {
    size_t n;
    char *out;
    if (s == NULL) {
        return NULL;
    }
    n = strlen(s);
    out = (char*)malloc(n + 1);
    if (out == NULL) {
        return NULL;
    }
    memcpy(out, s, n + 1);
    return out;
}

char *ad_signal_input_arg_from_ll(long long v) {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%lld", v);
    return ad_signal_input_strdup(tmp);
}

char *ad_signal_input_arg_from_ull(unsigned long long v) {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%llu", v);
    return ad_signal_input_strdup(tmp);
}

char *ad_signal_input_arg_from_double(double v) {
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "%.17g", v);
    return ad_signal_input_strdup(tmp);
}

char *ad_signal_input_arg_from_char(char v) {
    char tmp[2];
    tmp[0] = v;
    tmp[1] = 0;
    return ad_signal_input_strdup(tmp);
}

static int ad_signal_input_hex_value(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    if ('a' <= c && c <= 'f') {
        return 10 + (c - 'a');
    }
    if ('A' <= c && c <= 'F') {
        return 10 + (c - 'A');
    }
    return -1;
}

static int ad_signal_input_is_ascii_hex_even(const char *s) {
    int n = 0;
    if (s == NULL || s[0] == 0) {
        return 0;
    }
    while (s[n] != 0) {
        if (!isxdigit((unsigned char)s[n])) {
            return 0;
        }
        n++;
    }
    return (n % 2) == 0;
}

static void ad_signal_input_skip_ws(ad_signal_input_expr_parser *p) {
    while (*p->s != 0 && isspace((unsigned char)*p->s)) {
        p->s++;
    }
}

static int ad_signal_input_parse_index(const char *s, int *len_out) {
    int v = 0;
    int n = 0;
    while (isdigit((unsigned char)s[n])) {
        v = v * 10 + (s[n] - '0');
        n++;
    }
    if (n == 0) {
        return -1;
    }
    if (len_out != NULL) {
        *len_out = n;
    }
    return v;
}

static int ad_signal_input_arg_exists(const char **args, int argc, int idx) {
    return 0 <= idx && idx < argc && args[idx] != NULL;
}

static double ad_signal_input_arg_to_double(const char *arg, int *ok) {
    char *end = NULL;
    double v;

    *ok = 1;

    if (arg == NULL) {
        *ok = 0;
        return 0.0;
    }

    if (arg[0] != 0 && arg[1] == 0) {
        return (unsigned char)atoi(arg);
    }

    v = strtod(arg, &end);
    if (end != NULL && *end == 0) {
        return v;
    }

    *ok = 0;
    return 0.0;
}

static int ad_signal_input_buffer_append_be(Buffer *buffer, unsigned long long v, int bits) {
    if (bits == 8) {
        ad_buffer_append_byte(buffer, (byte)(v & 0xFFu));
        return 1;
    }
    if (bits == 16) {
        ad_buffer_append_byte(buffer, (byte)((v >> 8) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)(v & 0xFFu));
        return 1;
    }
    if (bits == 32) {
        ad_buffer_append_byte(buffer, (byte)((v >> 24) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 16) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 8) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)(v & 0xFFu));
        return 1;
    }
    if (bits == 64) {
        ad_buffer_append_byte(buffer, (byte)((v >> 56) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 48) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 40) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 32) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 24) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 16) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)((v >> 8) & 0xFFu));
        ad_buffer_append_byte(buffer, (byte)(v & 0xFFu));
        return 1;
    }
    return 0;
}

static int ad_signal_input_buffer_append_arg_raw(Buffer *buffer, const char *arg) {
    int ok;
    double dv;
    const char *s;

    if (arg == NULL || arg[0] == 0) {
        return 1;
    }
    if (arg[0] != 0 && arg[1] == 0) {
        ad_buffer_append_byte(buffer, atoi(arg));
        return 1;
    }

    dv = ad_signal_input_arg_to_double(arg, &ok);
    if (ok) {
        return ad_signal_input_buffer_append_be(buffer, (unsigned long long)((long long)dv), 8);
    }

    if (ad_signal_input_is_ascii_hex_even(arg)) {
        s = arg;
        while (*s != 0) {
            int hi = ad_signal_input_hex_value(s[0]);
            int lo = ad_signal_input_hex_value(s[1]);
            if (hi < 0 || lo < 0) {
                return 0;
            }
            ad_buffer_append_byte(buffer, (byte)((hi << 4) | lo));
            s += 2;
        }
        return 1;
    }

    ad_buffer_append_bytes(buffer, (byte*)arg, (unsigned)strlen(arg));
    return 1;
}

static ad_signal_input_expr_value ad_signal_input_expr_parse_expr(ad_signal_input_expr_parser *p);

static ad_signal_input_expr_value ad_signal_input_expr_value_make(double value, int flatten_bits) {
    ad_signal_input_expr_value out;
    out.value = value;
    out.flatten_bits = flatten_bits;
    return out;
}

static int ad_signal_input_match(ad_signal_input_expr_parser *p, const char *kw) {
    size_t n = strlen(kw);
    if (strncmp(p->s, kw, n) == 0) {
        p->s += n;
        return 1;
    }
    return 0;
}

static ad_signal_input_expr_value ad_signal_input_expr_parse_primary(ad_signal_input_expr_parser *p) {
    ad_signal_input_expr_value out = ad_signal_input_expr_value_make(0.0, 0);
    ad_signal_input_skip_ws(p);

    if (*p->s == '(') {
        p->s++;
        out = ad_signal_input_expr_parse_expr(p);
        ad_signal_input_skip_ws(p);
        if (*p->s != ')') {
            p->error = "expected ')'";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        return out;
    }

    if (*p->s == '$') {
        int idx;
        int len;
        int ok;
        double v;
        p->s++;
        idx = ad_signal_input_parse_index(p->s, &len);
        if (idx < 0) {
            p->error = "expected argument index";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s += len;
        if (!ad_signal_input_arg_exists(p->args, p->argc, idx)) {
            p->error = "missing argument";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        v = ad_signal_input_arg_to_double(p->args[idx], &ok);
        if (!ok) {
            p->error = "argument is not numeric";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        return ad_signal_input_expr_value_make(v, 0);
    }

    if (ad_signal_input_match(p, "flatten_be8")) {
        ad_signal_input_expr_value inner;
        ad_signal_input_skip_ws(p);
        if (*p->s != '(') {
            p->error = "expected '(' after flatten_be8";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner = ad_signal_input_expr_parse_expr(p);
        ad_signal_input_skip_ws(p);
        if (*p->s != ')') {
            p->error = "expected ')'";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner.flatten_bits = 8;
        return inner;
    }

    if (ad_signal_input_match(p, "flatten_be16")) {
        ad_signal_input_expr_value inner;
        ad_signal_input_skip_ws(p);
        if (*p->s != '(') {
            p->error = "expected '(' after flatten_be16";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner = ad_signal_input_expr_parse_expr(p);
        ad_signal_input_skip_ws(p);
        if (*p->s != ')') {
            p->error = "expected ')'";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner.flatten_bits = 16;
        return inner;
    }

    if (ad_signal_input_match(p, "flatten_be32")) {
        ad_signal_input_expr_value inner;
        ad_signal_input_skip_ws(p);
        if (*p->s != '(') {
            p->error = "expected '(' after flatten_be32";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner = ad_signal_input_expr_parse_expr(p);
        ad_signal_input_skip_ws(p);
        if (*p->s != ')') {
            p->error = "expected ')'";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner.flatten_bits = 32;
        return inner;
    }

    if (ad_signal_input_match(p, "flatten_be64")) {
        ad_signal_input_expr_value inner;
        ad_signal_input_skip_ws(p);
        if (*p->s != '(') {
            p->error = "expected '(' after flatten_be64";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner = ad_signal_input_expr_parse_expr(p);
        ad_signal_input_skip_ws(p);
        if (*p->s != ')') {
            p->error = "expected ')'";
            return ad_signal_input_expr_value_make(0.0, 0);
        }
        p->s++;
        inner.flatten_bits = 64;
        return inner;
    }

    {
        char *end = NULL;
        double v = strtod(p->s, &end);
        if (end != p->s) {
            p->s = end;
            return ad_signal_input_expr_value_make(v, 0);
        }
    }

    p->error = "unexpected token";
    return ad_signal_input_expr_value_make(0.0, 0);
}

static ad_signal_input_expr_value ad_signal_input_expr_parse_unary(ad_signal_input_expr_parser *p) {
    ad_signal_input_skip_ws(p);
    if (*p->s == '+') {
        p->s++;
        return ad_signal_input_expr_parse_unary(p);
    }
    if (*p->s == '-') {
        ad_signal_input_expr_value x;
        p->s++;
        x = ad_signal_input_expr_parse_unary(p);
        x.value = -x.value;
        return x;
    }
    return ad_signal_input_expr_parse_primary(p);
}

static ad_signal_input_expr_value ad_signal_input_expr_parse_mul(ad_signal_input_expr_parser *p) {
    ad_signal_input_expr_value left = ad_signal_input_expr_parse_unary(p);
    while (p->error == NULL) {
        ad_signal_input_expr_value right;
        ad_signal_input_skip_ws(p);
        if (*p->s != '*' && *p->s != '/') {
            break;
        }
        if (*p->s == '*') {
            p->s++;
            right = ad_signal_input_expr_parse_unary(p);
            left.value *= right.value;
            if (left.flatten_bits == 0) {
                left.flatten_bits = right.flatten_bits;
            }
        } else {
            p->s++;
            right = ad_signal_input_expr_parse_unary(p);
            left.value /= right.value;
            if (left.flatten_bits == 0) {
                left.flatten_bits = right.flatten_bits;
            }
        }
    }
    return left;
}

static ad_signal_input_expr_value ad_signal_input_expr_parse_expr(ad_signal_input_expr_parser *p) {
    ad_signal_input_expr_value left = ad_signal_input_expr_parse_mul(p);
    while (p->error == NULL) {
        ad_signal_input_expr_value right;
        ad_signal_input_skip_ws(p);
        if (*p->s != '+' && *p->s != '-') {
            break;
        }
        if (*p->s == '+') {
            p->s++;
            right = ad_signal_input_expr_parse_mul(p);
            left.value += right.value;
            if (left.flatten_bits == 0) {
                left.flatten_bits = right.flatten_bits;
            }
        } else {
            p->s++;
            right = ad_signal_input_expr_parse_mul(p);
            left.value -= right.value;
            if (left.flatten_bits == 0) {
                left.flatten_bits = right.flatten_bits;
            }
        }
    }
    return left;
}

static int ad_signal_input_append_expr_eval(Buffer *buffer, const char *src, const char **args, int argc) {
    ad_signal_input_expr_parser p;
    ad_signal_input_expr_value v;
    int bits;
    long long iv;

    p.s = src;
    p.args = args;
    p.argc = argc;
    p.error = NULL;

    v = ad_signal_input_expr_parse_expr(&p);
    ad_signal_input_skip_ws(&p);

    if (p.error != NULL || *p.s != 0) {
        return 0;
    }

    bits = v.flatten_bits == 0 ? 8 : v.flatten_bits;
    iv = (long long)v.value;

    return ad_signal_input_buffer_append_be(buffer, (unsigned long long)iv, bits);
}

Buffer *ad_signal_input_expr_builderv(const char *expr, const char **args, int argc) {
    Buffer *out;
    const char *s;

    if (expr == NULL) {
        return NULL;
    }

    out = ad_buffer_new();
    if (out == NULL) {
        return NULL;
    }

    s = expr;
    while (*s != 0) {
        if (*s != '$') {
            if (isxdigit((unsigned char)s[0]) && isxdigit((unsigned char)s[1])) {
                int hi = ad_signal_input_hex_value(s[0]);
                int lo = ad_signal_input_hex_value(s[1]);
                if (hi < 0 || lo < 0) {
                    ad_buffer_free(out);
                    return NULL;
                }
                ad_buffer_append_byte(out, (byte)((hi << 4) | lo));
                s += 2;
                continue;
            }
            if (!isspace((unsigned char)*s)) {
                ad_buffer_append_byte(out, (byte)*s);
            }
            s++;
            continue;
        }

        s++;

        if (*s == '{') {
            int idx;
            int len;
            s++;
            idx = ad_signal_input_parse_index(s, &len);
            if (idx < 0) {
                ad_buffer_free(out);
                return NULL;
            }
            s += len;
            if (*s != '}') {
                ad_buffer_free(out);
                return NULL;
            }
            s++;
            if (ad_signal_input_arg_exists(args, argc, idx)) {
                if (!ad_signal_input_buffer_append_arg_raw(out, args[idx])) {
                    ad_buffer_free(out);
                    return NULL;
                }
            }
            continue;
        }

        if (*s == '(') {
            const char *start = ++s;
            int depth = 1;
            while (*s != 0 && depth > 0) {
                if (*s == '(') {
                    depth++;
                } else if (*s == ')') {
                    depth--;
                }
                if (depth > 0) {
                    s++;
                }
            }
            if (*s != ')') {
                ad_buffer_free(out);
                return NULL;
            }
            {
                size_t n = (size_t)(s - start);
                char *sub = (char*)malloc(n + 1);
                int ok;
                if (sub == NULL) {
                    ad_buffer_free(out);
                    return NULL;
                }
                memcpy(sub, start, n);
                sub[n] = 0;
                ok = ad_signal_input_append_expr_eval(out, sub, args, argc);
                free(sub);
                if (!ok) {
                    ad_buffer_free(out);
                    return NULL;
                }
            }
            s++;
            continue;
        }

        if (isdigit((unsigned char)*s)) {
            int idx;
            int len;
            idx = ad_signal_input_parse_index(s, &len);
            if (idx < 0) {
                ad_buffer_free(out);
                return NULL;
            }
            s += len;
            if (!ad_signal_input_arg_exists(args, argc, idx)) {
                ad_buffer_free(out);
                return NULL;
            }
            if (!ad_signal_input_buffer_append_arg_raw(out, args[idx])) {
                ad_buffer_free(out);
                return NULL;
            }
            continue;
        }

        ad_buffer_free(out);
        return NULL;
    }

    return out;
}

Buffer *ad_signal_input_expr_builder_auto_impl(const char *expr, char **owned_args, int argc) {
    Buffer *out;
    const char **args;
    int i;

    args = (const char**)calloc((size_t)argc, sizeof(char*));
    if (args == NULL) {
        for (i = 0; i < argc; i++) {
            free(owned_args[i]);
        }
        free(owned_args);
        return NULL;
    }

    for (i = 0; i < argc; i++) {
        args[i] = owned_args[i];
    }

    out = ad_signal_input_expr_builderv(expr, args, argc);

    for (i = 0; i < argc; i++) {
        free(owned_args[i]);
    }
    free(args);
    //free(owned_args);

    return out;
}
