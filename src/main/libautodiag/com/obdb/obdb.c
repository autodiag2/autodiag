#include "libautodiag/com/obdb/obdb.h"

#include "cJSON.h"
#include "libautodiag/com/vehicle_signal.h"

static char *ad_obdb_strdup(const char *s) {
    size_t n;
    char *r;
    if (s == null) {
        return null;
    }
    n = strlen(s);
    r = (char *) malloc(n + 1);
    if (r == null) {
        return null;
    }
    memcpy(r, s, n + 1);
    return r;
}

static char *ad_obdb_strdup_printf(const char *fmt, ...) {
    va_list ap;
    va_list ap2;
    int n;
    char *buf;

    va_start(ap, fmt);
    va_copy(ap2, ap);
    n = vsnprintf(null, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return null;
    }

    buf = (char *) malloc((size_t) n + 1);
    if (buf == null) {
        va_end(ap2);
        return null;
    }

    vsnprintf(buf, (size_t) n + 1, fmt, ap2);
    va_end(ap2);
    return buf;
}

static void ad_obdb_str_append(char **dst, const char *src) {
    size_t a;
    size_t b;
    char *p;

    if (src == null) {
        return;
    }

    if (*dst == null) {
        *dst = ad_obdb_strdup(src);
        return;
    }

    a = strlen(*dst);
    b = strlen(src);
    p = (char *) realloc(*dst, a + b + 1);
    if (p == null) {
        return;
    }
    memcpy(p + a, src, b + 1);
    *dst = p;
}

static void ad_obdb_str_appendf(char **dst, const char *fmt, ...) {
    va_list ap;
    va_list ap2;
    int n;
    char *tmp;

    va_start(ap, fmt);
    va_copy(ap2, ap);
    n = vsnprintf(null, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return;
    }

    tmp = (char *) malloc((size_t) n + 1);
    if (tmp == null) {
        va_end(ap2);
        return;
    }

    vsnprintf(tmp, (size_t) n + 1, fmt, ap2);
    va_end(ap2);

    ad_obdb_str_append(dst, tmp);
    free(tmp);
}

static int ad_obdb_is_hex_string(const char *s) {
    size_t i;
    if (s == null || s[0] == 0) {
        return 0;
    }
    for (i = 0; s[i] != 0; i++) {
        if (isxdigit((unsigned char) s[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

static char *ad_obdb_registry_to_raw_url(const char *registry) {
    if (registry == null || registry[0] == 0) {
        return null;
    }

    if (strstr(registry, "://") != null) {
        return ad_obdb_strdup(registry);
    }

    if (strstr(registry, "/signalsets/") != null) {
        return ad_obdb_strdup_printf("https://raw.githubusercontent.com/%s/refs/heads/main", registry);
    }

    if (strchr(registry, '/') != null) {
        return ad_obdb_strdup_printf("https://raw.githubusercontent.com/%s/refs/heads/main/signalsets/v3/default.json", registry);
    }

    return ad_obdb_strdup_printf("https://raw.githubusercontent.com/OBDb/%s/refs/heads/main/signalsets/v3/default.json", registry);
}

static char *ad_obdb_json_to_compact_string(cJSON *node) {
    if (node == null) {
        return null;
    }
    return cJSON_PrintUnformatted(node);
}

static char *ad_obdb_slug_from_id_or_name(const char *id, const char *name) {
    const char *src = id != null && id[0] != 0 ? id : name;
    size_t i;
    size_t j;
    char *out;

    if (src == null) {
        return null;
    }

    out = (char *) malloc(strlen(src) + 1);
    if (out == null) {
        return null;
    }

    j = 0;
    for (i = 0; src[i] != 0; i++) {
        unsigned char ch = (unsigned char) src[i];
        if (isalnum(ch)) {
            out[j++] = (char) ch;
        } else {
            if (j == 0 || out[j - 1] == '_') {
                continue;
            }
            out[j++] = '_';
        }
    }

    while (0 < j && out[j - 1] == '_') {
        j--;
    }
    out[j] = 0;
    return out;
}

static char *ad_obdb_category_from_path(const char *path) {
    size_t i;
    char *out;
    if (path == null) {
        return null;
    }
    out = ad_obdb_strdup(path);
    if (out == null) {
        return null;
    }
    for (i = 0; out[i] != 0; i++) {
        if (out[i] == '.') {
            out[i] = ',';
        }
    }
    return out;
}

static char *ad_obdb_standard_from_registry(const char *registry) {
    if (registry == null) {
        return ad_obdb_strdup("OBDb");
    }
    if (strstr(registry, "SAEJ1979") != null) {
        return ad_obdb_strdup("SAEJ1979");
    }
    return ad_obdb_strdup("OBDb");
}

static const char *ad_obdb_json_get_string(cJSON *obj, const char *key) {
    cJSON *v = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (cJSON_IsString(v) == 0 || v->valuestring == null) {
        return null;
    }
    return v->valuestring;
}

static double ad_obdb_json_get_number_or_default(cJSON *obj, const char *key, double def) {
    cJSON *v = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (cJSON_IsNumber(v) == 0) {
        return def;
    }
    return v->valuedouble;
}

static char *ad_obdb_build_input_formula(cJSON *cmd_obj) {
    cJSON *child;
    char *expr = null;

    if (cmd_obj == null || cJSON_IsObject(cmd_obj) == 0) {
        return null;
    }

    for (child = cmd_obj->child; child != null; child = child->next) {
        if (cJSON_IsString(child) == 0 || child->valuestring == null) {
            continue;
        }
        ad_obdb_str_append(&expr, child->string);
        ad_obdb_str_append(&expr, child->valuestring);
    }

    return expr;
}

static char *ad_obdb_base_extract_expr(int bix, int len, int sign) {
    int byte_off;
    int bit_off;

    if (len <= 0) {
        return null;
    }

    byte_off = bix / 8;
    bit_off = bix % 8;

    if (bit_off == 0) {
        if (sign && len == 8) return ad_obdb_strdup_printf("s8(%d)", byte_off);
        if (sign && len == 16) return ad_obdb_strdup_printf("s16be(%d)", byte_off);
        if (sign && len == 32) return ad_obdb_strdup_printf("s32be(%d)", byte_off);
        if (!sign && len == 8) return ad_obdb_strdup_printf("u8(%d)", byte_off);
        if (!sign && len == 16) return ad_obdb_strdup_printf("u16be(%d)", byte_off);
        if (!sign && len == 24) return ad_obdb_strdup_printf("u24be(%d)", byte_off);
        if (!sign && len == 32) return ad_obdb_strdup_printf("u32be(%d)", byte_off);
        if (!sign && len == 64) return ad_obdb_strdup_printf("u64be(%d)", byte_off);
    }

    if (len == 1) {
        return ad_obdb_strdup_printf("bit(%d,%d)", byte_off, bit_off);
    }

    return ad_obdb_strdup_printf("bits(%d,%d,%d)", byte_off, bit_off, len);
}

static char *ad_obdb_build_rv_formula(cJSON *fmt) {
    int bix;
    int len;
    int sign;
    double add;
    double mul;
    double div;
    char *expr;
    char *tmp;
    cJSON *map_obj;

    if (fmt == null || cJSON_IsObject(fmt) == 0) {
        return null;
    }

    map_obj = cJSON_GetObjectItemCaseSensitive(fmt, "map");
    if (map_obj != null) {
        return null;
    }

    bix = (int) ad_obdb_json_get_number_or_default(fmt, "bix", 0);
    len = (int) ad_obdb_json_get_number_or_default(fmt, "len", 0);
    sign = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(fmt, "sign")) ? 1 : 0;
    add = ad_obdb_json_get_number_or_default(fmt, "add", 0.0);
    mul = ad_obdb_json_get_number_or_default(fmt, "mul", 1.0);
    div = ad_obdb_json_get_number_or_default(fmt, "div", 1.0);

    expr = ad_obdb_base_extract_expr(bix, len, sign);
    if (expr == null) {
        return null;
    }

    if (mul != 1.0) {
        tmp = ad_obdb_strdup_printf("(%s * %.17g)", expr, mul);
        free(expr);
        expr = tmp;
        if (expr == null) {
            return null;
        }
    }

    if (div != 1.0) {
        tmp = ad_obdb_strdup_printf("(%s / %.17g)", expr, div);
        free(expr);
        expr = tmp;
        if (expr == null) {
            return null;
        }
    }

    if (add != 0.0) {
        if (add < 0.0) {
            tmp = ad_obdb_strdup_printf("(%s - %.17g)", expr, -add);
        } else {
            tmp = ad_obdb_strdup_printf("(%s + %.17g)", expr, add);
        }
        free(expr);
        expr = tmp;
        if (expr == null) {
            return null;
        }
    }

    return expr;
}

static int ad_obdb_register_signal(const char *registry,
                                   const char *target_ecu,
                                   const char *input_formula,
                                   cJSON *signal_json) {
    cJSON *fmt;
    const char *id;
    const char *name;
    const char *description;
    const char *path;
    const char *unit;
    double rv_min;
    double rv_max;
    char *rv_formula;
    char *category;
    char *standard;
    char *slug;
    char *examples;

    if (signal_json == null || cJSON_IsObject(signal_json) == 0) {
        return 0;
    }

    fmt = cJSON_GetObjectItemCaseSensitive(signal_json, "fmt");
    id = ad_obdb_json_get_string(signal_json, "id");
    name = ad_obdb_json_get_string(signal_json, "name");
    description = ad_obdb_json_get_string(signal_json, "description");
    path = ad_obdb_json_get_string(signal_json, "path");
    unit = fmt != null ? ad_obdb_json_get_string(fmt, "unit") : null;

    rv_min = fmt != null ? ad_obdb_json_get_number_or_default(fmt, "min", 0.0) : 0.0;
    rv_max = fmt != null ? ad_obdb_json_get_number_or_default(fmt, "max", 0.0) : 0.0;

    rv_formula = ad_obdb_build_rv_formula(fmt);
    category = ad_obdb_category_from_path(path);
    standard = ad_obdb_standard_from_registry(registry);
    slug = ad_obdb_slug_from_id_or_name(id, name);
    examples = ad_obdb_json_to_compact_string(signal_json);

    AD_SIGNAL_REGISTER(
        name,
        input_formula,
        rv_min,
        rv_max,
        rv_formula,
        description,
        category,
        standard,
        slug,
        unit,
        null,
        target_ecu,
        examples
    );

    free(rv_formula);
    free(category);
    free(standard);
    free(slug);
    free(examples);
    return 1;
}

bool ad_obdb_fetch_signals(char *registry) {
    char *url;
    char *json_text;
    cJSON *root;
    cJSON *commands;
    cJSON *command;
    int registered;

    if (registry == null || registry[0] == 0) {
        return false;
    }

    url = ad_obdb_registry_to_raw_url(registry);
    log_debug("fetching %s", url);
    if (url == null) {
        return false;
    }

    int status = 0;
    json_text = ad_http_get(url, &status);
    free(url);
    if (json_text == null) {
        return false;
    }

    root = cJSON_Parse(json_text);
    free(json_text);
    if (root == null) {
        return false;
    }

    commands = cJSON_GetObjectItemCaseSensitive(root, "commands");
    if (cJSON_IsArray(commands) == 0) {
        cJSON_Delete(root);
        return false;
    }

    registered = 0;

    cJSON_ArrayForEach(command, commands) {
        cJSON *cmd_obj;
        cJSON *signals;
        cJSON *signal;
        const char *hdr;
        const char *rax;
        char *input_formula;

        if (cJSON_IsObject(command) == 0) {
            continue;
        }

        hdr = ad_obdb_json_get_string(command, "hdr");
        cmd_obj = cJSON_GetObjectItemCaseSensitive(command, "cmd");
        signals = cJSON_GetObjectItemCaseSensitive(command, "signals");
        input_formula = ad_obdb_build_input_formula(cmd_obj);

        if (cJSON_IsArray(signals)) {
            cJSON_ArrayForEach(signal, signals) {
                registered += ad_obdb_register_signal(registry, hdr, input_formula, signal);
            }
        }

        free(input_formula);
    }

    cJSON_Delete(root);
    return 0 < registered;
}