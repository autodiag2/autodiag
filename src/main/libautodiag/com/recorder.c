#include "libautodiag/com/recorder.h"

AD_LIST_SRC_DEEP(ECUBufferRecord,
    {
        list->ecu = null;
    },
    {
        list->ecu = null;
    }
)
AD_LIST_SRC(object_Record)
AD_LIST_SRC(list_ECUBufferRecord)

int ECUBufferRecord_cmp(final ECUBufferRecord *r1, final ECUBufferRecord *r2) {
    return buffer_cmp(r1, r2);
}
int object_Record_cmp(object_Record * r1, object_Record * r2) {
    return r1 - r2;
}
int list_ECUBufferRecord_cmp(list_ECUBufferRecord * r1, list_ECUBufferRecord * r2) {
    return r1->ecu - r2->ecu;
}
object_Record * object_Record_new() {
    object_Record * r = (object_Record*)malloc(sizeof(object_Record));
    r->binRequest = buffer_new();
    r->binResponses = list_list_ECUBufferRecord_new();
    return r;
}
void object_Record_free(final object_Record *r) {
    assert(r != null);
    if ( r->binRequest != null ) {
        buffer_free(r->binRequest);
        r->binRequest = null;
    }
    if ( r->binResponses != null ) {
        list_list_ECUBufferRecord_free(r->binResponses);
        r->binResponses = null;
    }
    free(r);
}
object_Record * object_Record_assign(object_Record *r, final object_Record *r2) {
    r->binRequest = buffer_copy(r2->binRequest);
    list_list_ECUBufferRecord_clear(r->binResponses);
    log_msg(LOG_DEBUG, "TODO, deep copy");
    r->binResponses = r2->binResponses;
    return r;
}
static final list_object_Record * recorder = null;
static void ensure_init() {
    if ( recorder == null ) {
        recorder = list_object_Record_new();
    }
}
void record_on_request(final Buffer * binRequest) {
    ensure_init();
    final object_Record * record = object_Record_new();
    record->binRequest = buffer_copy(binRequest);
    list_object_Record_append(recorder, record);
}
list_ECUBufferRecord * ecu_response_list_find(final list_list_ECUBufferRecord * list, final ECU * ecu) {
    for(int i = 0; i < list->size; i++) {
        list_ECUBufferRecord * o = list->list[i];
        if ( ecu == o->ecu ) {
            return o;
        }
    }
    return null;
}
void record_on_response(final ECU * ecu, final Buffer * binResponse) {
    ensure_init();
    assert(0 < recorder->size);
    final object_Record * record = recorder->list[recorder->size-1];

    list_list_ECUBufferRecord * responses = record->binResponses;
    list_ECUBufferRecord * list = ecu_response_list_find(responses, ecu);
    if ( list == null ) {
        list = list_ECUBufferRecord_new();
        list_list_ECUBufferRecord_append(responses, list);
    }
    list_ECUBufferRecord_append(list, buffer_copy(binResponse));
}