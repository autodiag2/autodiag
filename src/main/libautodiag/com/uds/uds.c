#include "libautodiag/com/uds/uds.h"

HASHMAP_SRC(object_Int, object_string)
static object_hashmap_object_Int_object_string * service_to_string_map = null;

int object_hashmap_object_Int_object_string_key_comparator(object_Int * k1, object_Int *k2) {
    assert(k1 != null);
    assert(k2 != null);
    return k1->value - k2->value;
}
int object_hashmap_void_char_key_comparator(void * k1, void *k2) {
    return ! ( k1 == k2 );
}
char* uds_service_to_string(final UDService key) {
    if ( service_to_string_map == null ) {
        service_to_string_map = object_hashmap_object_Int_object_string_new();
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL), "Diagnostic Session Control");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_ECU_RESET), "ECU Reset");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION), "Clear Diagnostic Information");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DTC_INFORMATION), "Read DTC Information");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_IDENTIFIER), "Read Data By Identifier");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_MEMORY_BY_ADDRESS), "Read Memory By Address");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_SCALING_DATA_BY_IDENTIFIER), "Read Scaling Data By Identifier");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_SECURITY_ACCESS), "Security Access");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_COMMUNICATION_CONTROL), "Communication Control");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_AUTHENTICATION), "Authentication");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_PERIOD_IDENTIFIER), "Read Data By Period Identifier");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_FRESH_DATA_BY_IDENTIFIER), "Read Fresh Data By Identifier");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_DYNAMICALLY_DEFINE_DATA_IDENTIFIER), "Dynamically Define Data Identifier");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER), "Write Data By Identifier");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER), "Input Output Control By Identifier");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_ROUTINE_CONTROL), "Routine Control");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_DOWNLOAD), "Request Download");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_UPLOAD), "Request Upload");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_TRANSFER_DATA), "Transfer Data");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_TRANSFER_EXIT), "Request Transfer Exit");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_FILE_TRANSFER), "Request File Transfer");
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_TESTER_PRESENT), "Tester Present");
    }
    object_string * result = object_hashmap_object_Int_object_string_get(service_to_string_map, object_Int_new_from(key));
    if ( result ) {
        return strdup(result->data);
    } else {
        return null;
    }
}
