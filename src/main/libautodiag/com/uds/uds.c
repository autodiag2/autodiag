#include "libautodiag/com/uds/uds.h"

static object_hashmap_object_Int_object_string * service_to_string_map = null;
static object_hashmap_object_Int_object_Int * service_to_response_map = null;

int uds_service_response(final UDSService key) {
    if ( service_to_response_map == null ) {
        service_to_response_map = object_hashmap_object_Int_object_Int_new();
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL), object_Int_new_from(0x50));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_ECU_RESET), object_Int_new_from(0x51));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION), object_Int_new_from(0x54));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_DTC_INFORMATION), object_Int_new_from(0x59));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_IDENTIFIER), object_Int_new_from(0x62));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_MEMORY_BY_ADDRESS), object_Int_new_from(0x63));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_SCALING_DATA_BY_IDENTIFIER), object_Int_new_from(0x64));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_SECURITY_ACCESS), object_Int_new_from(0x67));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_COMMUNICATION_CONTROL), object_Int_new_from(0x68));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_AUTHENTICATION), object_Int_new_from(0x69));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_PERIOD_IDENTIFIER), object_Int_new_from(0x6A));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_FRESH_DATA_BY_IDENTIFIER), object_Int_new_from(0x6C));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_DYNAMICALLY_DEFINE_DATA_IDENTIFIER), object_Int_new_from(0x6C));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER), object_Int_new_from(0x6E));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER), object_Int_new_from(0x70));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_ROUTINE_CONTROL), object_Int_new_from(0x71));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_DOWNLOAD), object_Int_new_from(0x74));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_UPLOAD), object_Int_new_from(0x75));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_TRANSFER_DATA), object_Int_new_from(0x76));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_TRANSFER_EXIT), object_Int_new_from(0x77));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_FILE_TRANSFER), object_Int_new_from(0x78));
        object_hashmap_object_Int_object_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_TESTER_PRESENT), object_Int_new_from(0x7E));
    }
    object_Int * result = object_hashmap_object_Int_object_Int_get(service_to_response_map, object_Int_new_from(key));
    if ( result ) {
        return result->value;
    } else {
        return 0xFF;
    }
}

char* uds_service_to_string(final UDSService key) {
    if ( service_to_string_map == null ) {
        service_to_string_map = object_hashmap_object_Int_object_string_new();
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL), object_string_new_from("Diagnostic Session Control"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_ECU_RESET), object_string_new_from("ECU Reset"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION), object_string_new_from("Clear Diagnostic Information"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DTC_INFORMATION), object_string_new_from("Read DTC Information"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_IDENTIFIER), object_string_new_from("Read Data By Identifier"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_MEMORY_BY_ADDRESS), object_string_new_from("Read Memory By Address"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_SCALING_DATA_BY_IDENTIFIER), object_string_new_from("Read Scaling Data By Identifier"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_SECURITY_ACCESS), object_string_new_from("Security Access"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_COMMUNICATION_CONTROL), object_string_new_from("Communication Control"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_AUTHENTICATION), object_string_new_from("Authentication"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_PERIOD_IDENTIFIER), object_string_new_from("Read Data By Period Identifier"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_FRESH_DATA_BY_IDENTIFIER), object_string_new_from("Read Fresh Data By Identifier"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_DYNAMICALLY_DEFINE_DATA_IDENTIFIER), object_string_new_from("Dynamically Define Data Identifier"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER), object_string_new_from("Write Data By Identifier"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER), object_string_new_from("Input Output Control By Identifier"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_ROUTINE_CONTROL), object_string_new_from("Routine Control"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_DOWNLOAD), object_string_new_from("Request Download"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_UPLOAD), object_string_new_from("Request Upload"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_TRANSFER_DATA), object_string_new_from("Transfer Data"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_TRANSFER_EXIT), object_string_new_from("Request Transfer Exit"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_FILE_TRANSFER), object_string_new_from("Request File Transfer"));
        object_hashmap_object_Int_object_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_TESTER_PRESENT), object_string_new_from("Tester Present"));
    }
    object_string * result = object_hashmap_object_Int_object_string_get(service_to_string_map, object_Int_new_from(key));
    if ( result ) {
        return strdup(result->data);
    } else {
        return null;
    }
}
