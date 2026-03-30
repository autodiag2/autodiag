#include "libautodiag/com/http/http.h"
#include "mongoose.h"

struct ad_http_query_ctx {
    struct mg_mgr mgr;
    char *result;
    char *url;
    bool done;
    int status;
};
static void ad_http_cb(struct mg_connection *c, int ev, void *ev_data) {
    struct ad_http_query_ctx *ctx = (struct ad_http_query_ctx *) c->fn_data;

    if (ev == MG_EV_CONNECT) {
        int status = 0;
        if (ev_data != null) status = *(int *) ev_data;

        // Manual host/path split
        const char *url = ctx->url;
        const char *p = strstr(url, "://");
        const char *host_start = url;
        const char *path_start = "/";
        if (p != NULL) host_start = p + 3;
        const char *slash = strchr(host_start, '/');
        char host[128], path[512];
        if (slash) {
            size_t len = slash - host_start;
            if (len >= sizeof(host)) len = sizeof(host)-1;
            memcpy(host, host_start, len);
            host[len] = 0;
            strncpy(path, slash, sizeof(path)-1);
            path[sizeof(path)-1] = 0;
        } else {
            strncpy(host, host_start, sizeof(host)-1);
            host[sizeof(host)-1] = 0;
            strcpy(path, "/");
        }
        bool use_tls = false;
        if (p != null) {
            size_t scheme_len = (size_t) (p - url);
            if (scheme_len == 5 && strncmp(url, "https", 5) == 0) {
                use_tls = true;
            }
            host_start = p + 3;
        }

        if ( use_tls ) {
            struct mg_tls_opts opts = {
                .ca = NULL,
                .name = host
            };
            mg_tls_init(c, &opts);
        }
        
        if (status != 0) {
            ctx->done = true;
            ctx->status = -1;
            c->is_closing = 1;
            return;
        }

        mg_printf(c,
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: autodiag/1.0\r\n"
            "Connection: close\r\n\r\n",
            path, host);
    }

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        ctx->status = mg_http_status(hm);

        char *body = malloc(hm->body.len + 1);
        memcpy(body, hm->body.buf, hm->body.len);
        body[hm->body.len] = 0;

        ctx->result = body;
        ctx->done = true;
        c->is_closing = 1;
    }

    if (ev == MG_EV_ERROR) {
        ctx->status = -1;
        ctx->done = true;
        c->is_closing = 1;
    }
}

char * ad_http_get(const char * url, int *status) {

    struct ad_http_query_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));

    mg_mgr_init(&ctx.mgr);
    ctx.url = strdup(url);

    mg_http_connect(&ctx.mgr, url, ad_http_cb, &ctx);

    while ( ctx.done == false ) {
        mg_mgr_poll(&ctx.mgr, 100);
    }

    mg_mgr_free(&ctx.mgr);

    if ( status ) *status = ctx.status;

    return ctx.result;
}