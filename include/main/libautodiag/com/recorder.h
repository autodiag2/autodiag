#ifndef __AD_COM_RECORDER_H
#define __AD_COM_RECORDER_H

#include "libautodiag/lib.h"
#include "libautodiag/model/vehicle.h"

typedef Buffer ECUBufferRecord;

AD_LIST_H_DEEP(ECUBufferRecord,
    ECU * ecu;
)
AD_LIST_H(ad_list_ECUBufferRecord)

AD_OBJECT_H(Record,
    Buffer * binRequest;
    ad_list_ad_list_ECUBufferRecord * binResponses;
)

AD_LIST_H(ad_object_Record)
void record_on_request(final Buffer * binRequest);
void record_on_response(final ECU * ecu, final Buffer * binResponse);
void record_clear();
ad_list_ad_object_Record * recorder_get();
bool record_to_json_file(char *filepath);

#endif