#include "libautodiag/sim/ecu/generator.h"
#include "cJSON.h"

typedef struct ReqIdx {
    char *req;
    int idx;
    struct ReqIdx *next;
} ReqIdx;

typedef struct {
    cJSON *records;
    ReqIdx *head;
} GState;

static int get_next_index(GState *st, const char *req, int max) {
    ReqIdx *p = st->head;
    while (p) {
        if (strcmp(p->req, req) == 0) {
            int v = p->idx;
            p->idx = (p->idx + 1) % max;
            return v;
        }
        p = p->next;
    }
    ReqIdx *n = malloc(sizeof(ReqIdx));
    n->req = strdup(req);
    n->idx = 1 % max;
    n->next = st->head;
    st->head = n;
    return 0;
}

static Buffer *response(SimECUGenerator *generator, Buffer *binRequest) {
    if (!generator->context) {
        log_msg(LOG_ERROR, "This generator need filepath as context aborting");
        assert(generator->context != null);
    }
    GState *st = (GState*)generator->state;

    if (!st->records) {
        char *context = (char*)generator->context;
        FILE *f = fopen(context, "r");
        if ( f ) {
            fseek(f, 0, SEEK_END);
            long len = ftell(f);
            fseek(f, 0, SEEK_SET);
            char *buf = malloc(len + 1);
            fread(buf, 1, len, f);
            buf[len] = 0;
            fclose(f);
            st->records = cJSON_Parse(buf);
            free(buf);
        } else {
            if (errno == ENOENT || errno == ENAMETOOLONG) {
                cJSON * records = cJSON_Parse(context);
                if ( records ) {
                    st->records = records;
                } else {
                    log_msg(LOG_ERROR, "Context '%s' is not a file and not a json string");
                }
            } else if (errno == EACCES) {
                log_msg(LOG_ERROR, "File '%s' exists but missing reading permission", context);
            }
        }
        if (!st->records) {
            log_msg(LOG_ERROR, "Impossible to get the flow from the json aborting ...");
            exit(1);
        }
        if ( cJSON_IsArray(st->records) ) {
            if ( 0 < cJSON_GetArraySize(st->records) ) {
                log_msg(LOG_WARNING, "Multiple ECU records in the loaded file, so this ECU will respond frames of any ECU");
            }
        } else {
            assert(cJSON_IsObject(st->records));
            cJSON * arr = cJSON_CreateArray();
            cJSON_AddItemToArray(arr, st->records);
            st->records = arr;
        }
    }

    char *req_hex = buffer_to_hex_string(binRequest);

    // Calculate max flow count for this request and address filter
    int max = 0;

    int n = cJSON_GetArraySize(st->records);

    for (int i = 0; i < n; i++) {
        cJSON *ecu_obj = cJSON_GetArrayItem(st->records, i);

        cJSON *flow_arr = cJSON_GetObjectItem(ecu_obj, "flow");
        if (!flow_arr) continue;

        int fn = cJSON_GetArraySize(flow_arr);
        for (int j = 0; j < fn; j++) {
            cJSON *flow_obj = cJSON_GetArrayItem(flow_arr, j);
            char *file_req = cJSON_GetObjectItem(flow_obj, "request")->valuestring;
            if (strcmp(req_hex, file_req) == 0) {
                max++;
            }
        }
    }

    if (max == 0) return buffer_new();

    int idx = get_next_index(st, req_hex, max);

    int count = 0;
    for (int i = 0; i < n; i++) {
        cJSON *ecu_obj = cJSON_GetArrayItem(st->records, i);

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
static void init_state(SimECUGenerator * this) {
    GState *st = malloc(sizeof(GState));
    st->records = null;
    st->head = null;

    this->state = st;
}
static char * context_to_string(SimECUGenerator * this) {
    char * jsoncontext = this->context;
    return strdup(jsoncontext);
}
static bool context_load_from_string(SimECUGenerator * this, char * context) {
    this->context = strdup(context);
    init_state(this);
    return true;
}
SimECUGenerator* sim_ecu_generator_new_replay() {
    SimECUGenerator *g = sim_ecu_generator_new();
    g->response = SIM_ECU_GENERATOR_RESPONSE(response);
    g->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
    g->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
    g->type = strdup("replay");
    init_state(g);
    return g;
}
