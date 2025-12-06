#include "libautodiag/sim/ecu/generator.h"
#include "cJSON.h"

typedef struct ReqIdx {
    char *req;
    int idx;
    struct ReqIdx *next;
} ReqIdx;

typedef struct {
    Buffer *address;
    cJSON *records;
    ReqIdx *head;
} GState;

static int get_next_index(GState *st, const char *req) {
    ReqIdx *p = st->head;
    while (p) {
        if (strcmp(p->req, req) == 0) {
            int v = p->idx;
            p->idx++;
            return v;
        }
        p = p->next;
    }
    ReqIdx *n = malloc(sizeof(ReqIdx));
    n->req = strdup(req);
    n->idx = 1;
    n->next = st->head;
    st->head = n;
    return 0;
}

static Buffer *response(SimECUGenerator *generator, Buffer *binRequest) {
    if (!generator->context) return NULL;
    GState *st = (GState*)generator->state;

    if (!st->records) {
        char *filepath = (char*)generator->context;
        FILE *f = fopen(filepath, "r");
        if (!f) return NULL;
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *buf = malloc(len + 1);
        fread(buf, 1, len, f);
        buf[len] = 0;
        fclose(f);
        st->records = cJSON_Parse(buf);
        free(buf);
        if (!st->records) return NULL;
    }

    char *req_hex = buffer_to_hex_string(binRequest);
    int idx = get_next_index(st, req_hex);

    int n = cJSON_GetArraySize(st->records);
    int count = 0;

    for (int i = 0; i < n; i++) {
        cJSON *ecu_obj = cJSON_GetArrayItem(st->records, i);

        char *ecu_addr = cJSON_GetObjectItem(ecu_obj, "ecu")->valuestring;

        if (st->address &&
            buffer_cmp(st->address, buffer_from_ascii_hex(ecu_addr)) != 0 &&
            st->address->size != 0)
            continue;

        cJSON *flow_arr = cJSON_GetObjectItem(ecu_obj, "flow");
        if (!flow_arr) continue;

        int fn = cJSON_GetArraySize(flow_arr);

        for (int j = 0; j < fn; j++) {
            cJSON *flow_obj = cJSON_GetArrayItem(flow_arr, j);

            char *file_req = cJSON_GetObjectItem(flow_obj, "request")->valuestring;

            if (strcmp(req_hex, file_req) == 0) {
                if (count == idx) {
                    cJSON *rs_arr = cJSON_GetObjectItem(flow_obj, "responses");
                    if (!rs_arr || cJSON_GetArraySize(rs_arr) == 0) return buffer_new();
                    char *hex = cJSON_GetArrayItem(rs_arr, 0)->valuestring;
                    return buffer_from_ascii_hex(hex);
                }
                count++;
            }
        }
    }

    return buffer_new();
}

SimECUGenerator* sim_ecu_generator_new_replay(Buffer *address) {
    SimECUGenerator *g = sim_ecu_generator_new();
    g->response = SIM_ECU_GENERATOR_RESPONSE(response);
    g->type = strdup("replay");

    GState *st = malloc(sizeof(GState));
    st->address = buffer_copy(address);
    st->records = NULL;
    st->head = NULL;

    g->state = st;
    return g;
}
