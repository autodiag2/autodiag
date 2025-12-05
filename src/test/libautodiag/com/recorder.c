#include "libTest.h"

bool testRecorder(VehicleIFace* iface) {

    viface_recorder_set_state(iface, false);
    viface_recorder_reset(iface);
    list_object_Record* recorder = viface_recorder_get(iface);
    assert(recorder->size == 0);
    viface_recorder_set_state(iface, true);
    viface_lock(iface);
    final Buffer * binRequest = buffer_from_ints(0x01,0x01);
    viface_send(iface, binRequest);
    viface_clear_data(iface);
    viface_recv(iface);
    assert(recorder->size == 1);
    object_Record * record = recorder->list[0];

    assert(buffer_cmp(binRequest, record->binRequest) == 0);
    assert(0 < record->binResponses->size);
    
    printf("result:\n");
    for(int i = 0; i < record->binResponses->size; i ++) {
        list_ECUBufferRecord * r = record->binResponses->list[i];
        ECU * ecu = r->ecu;
        assert(ecu != null);
        vehicle_ecu_debug(ecu);
        for(int j = 0; j < r->size; j++) {
            printf("buffer %d\n", j);
            buffer_dump(r->list[j]);
        }
    }
    assert(record_to_json_file("/tmp/record.json"));

    return true;
}