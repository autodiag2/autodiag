#ifndef __COM_RECORDER_H
#define __COM_RECORDER_H

#include "libautodiag/lib.h"
#include "libautodiag/model/vehicle.h"
#include "cJSON.h"

typedef Buffer ECUBufferRecord;

AD_LIST_H_DEEP(ECUBufferRecord,
    ECU * ecu;
)
AD_LIST_H(list_ECUBufferRecord)

OBJECT_H(Record,
    Buffer * binRequest;
    list_list_ECUBufferRecord * binResponses;
)

AD_LIST_H(object_Record)
void record_on_request(final Buffer * binRequest);
void record_on_response(final ECU * ecu, final Buffer * binResponse);
void record_clear();
list_object_Record * recorder_get();
bool record_to_json_file(char *filepath);

#endif