#include "libautodiag/com/uds/uds.h"

static object_hashmap_Int_string * service_to_string_map = null;
static object_hashmap_Int_Int * service_to_response_map = null;
static object_hashmap_Int_string * nrc_to_string_map = null;

char * uds_nrc_to_string(final UDS_NRC nrc) {
    if ( nrc_to_string_map == null ) {
        nrc_to_string_map = object_hashmap_Int_string_new();
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_GENERAL_REJECT), object_string_new_from("General reject"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SERVICE_NOT_SUPPORTED), object_string_new_from("Service not supported"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SUBFUNCTION_NOT_SUPPORTED), object_string_new_from("Sub-function not supported"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_INVALID_MESSAGE_LENGTH), object_string_new_from("Invalid message length/format"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_RESPONSE_TOO_LONG), object_string_new_from("Response too long"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_BUSY_REPEAT_REQUEST), object_string_new_from("Busy-repeat request"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_CONDITIONS_NOT_CORRECT), object_string_new_from("Conditions not correct"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_REQUEST_SEQUENCE_ERROR), object_string_new_from("Request sequence error"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_NO_RESPONSE_FROM_SUBNET), object_string_new_from("No response from subnet component"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_FAILURE_PREVENTS_EXECUTION), object_string_new_from("Failure prevents execution of requested action"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_REQUEST_OUT_OF_RANGE), object_string_new_from("Request out of range"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SECURITY_ACCESS_DENIED), object_string_new_from("Security access denied"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_INVALID_KEY), object_string_new_from("Invalid key"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS), object_string_new_from("Exceeded number of attempts"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED), object_string_new_from("Required time delay has not expired"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED), object_string_new_from("Upload/download not accepted"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_TRANSFER_DATA_SUSPENDED), object_string_new_from("Transfer data suspended"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_PROGRAMMING_FAILURE), object_string_new_from("Programming failure"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER), object_string_new_from("Wrong block sequence counter"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_REQUEST_RECEIVED_RESPONSE_PENDING), object_string_new_from("Request received - response pending"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_SESSION), object_string_new_from("Sub function not supported in active session"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SERVICE_NOT_SUPPORTED_IN_SESSION), object_string_new_from("Service not supported in active session"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_RPM_TOO_HIGH), object_string_new_from("RPM too high"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_RPM_TOO_LOW), object_string_new_from("RPM too low"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_ENGINE_RUNNING), object_string_new_from("Engine is running"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_ENGINE_NOT_RUNNING), object_string_new_from("Engine is not running"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_ENGINE_RUN_TIME_TOO_LOW), object_string_new_from("Engine run time too low"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_TEMPERATURE_TOO_HIGH), object_string_new_from("Temperature too high"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_TEMPERATURE_TOO_LOW), object_string_new_from("Temperature too low"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SPEED_TOO_HIGH), object_string_new_from("Speed too high"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SPEED_TOO_LOW), object_string_new_from("Speed too low"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_THROTTLE_PEDAL_TOO_HIGH), object_string_new_from("Throttle pedal too high"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_THROTTLE_PEDAL_TOO_LOW), object_string_new_from("Throttle pedal too low"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_TRANSMISSION_RANGE_NOT_IN_NEUTRAL), object_string_new_from("Transmission range not in neutral"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_TRANSMISSION_RANGE_NOT_IN_DEAR), object_string_new_from("Transmission range not in dear"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_BRAKE_SWITCHES_NOT_CLOSED), object_string_new_from("Brake switches not closed"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_SHIFTER_LEVEL_NOT_IN_PARK), object_string_new_from("Shifter level not in park"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_TORK_CONVERTER_CLUTCH_LOCKED), object_string_new_from("Tork converter clutch locked"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_VOLTAGE_TOO_HIGH), object_string_new_from("Voltage too high"));
        object_hashmap_Int_string_set(nrc_to_string_map, object_Int_new_from(UDS_NRC_VOLTAGE_TOO_LOW), object_string_new_from("Voltage too low"));
    }
    if ( 0xF0 <= nrc && nrc <= 0xFE ) {
        return strdup("Manufacturer specific conditions are not met");
    }
    object_string * result = object_hashmap_Int_string_get(nrc_to_string_map, object_Int_new_from(nrc));
    if ( result == null ) {
        return null;
    } else {
        return result->data;
    }
}

int uds_service_response(final UDSService key) {
    if ( service_to_response_map == null ) {
        service_to_response_map = object_hashmap_Int_Int_new();
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL), object_Int_new_from(0x50));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_ECU_RESET), object_Int_new_from(0x51));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION), object_Int_new_from(0x54));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_DTC_INFORMATION), object_Int_new_from(0x59));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_IDENTIFIER), object_Int_new_from(0x62));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_MEMORY_BY_ADDRESS), object_Int_new_from(0x63));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_SCALING_DATA_BY_IDENTIFIER), object_Int_new_from(0x64));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_SECURITY_ACCESS), object_Int_new_from(0x67));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_COMMUNICATION_CONTROL), object_Int_new_from(0x68));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_AUTHENTICATION), object_Int_new_from(0x69));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_PERIOD_IDENTIFIER), object_Int_new_from(0x6A));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_READ_FRESH_DATA_BY_IDENTIFIER), object_Int_new_from(0x6C));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_DYNAMICALLY_DEFINE_DATA_IDENTIFIER), object_Int_new_from(0x6C));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER), object_Int_new_from(0x6E));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER), object_Int_new_from(0x70));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_ROUTINE_CONTROL), object_Int_new_from(0x71));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_DOWNLOAD), object_Int_new_from(0x74));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_UPLOAD), object_Int_new_from(0x75));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_TRANSFER_DATA), object_Int_new_from(0x76));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_TRANSFER_EXIT), object_Int_new_from(0x77));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_REQUEST_FILE_TRANSFER), object_Int_new_from(0x78));
        object_hashmap_Int_Int_set(service_to_response_map, object_Int_new_from(UDS_SERVICE_TESTER_PRESENT), object_Int_new_from(0x7E));
    }
    object_Int * result = object_hashmap_Int_Int_get(service_to_response_map, object_Int_new_from(key));
    if ( result ) {
        return result->value;
    } else {
        return 0xFF;
    }
}

char* uds_service_to_string(final UDSService key) {
    if ( service_to_string_map == null ) {
        service_to_string_map = object_hashmap_Int_string_new();
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL), object_string_new_from("Diagnostic Session Control"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_ECU_RESET), object_string_new_from("ECU Reset"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION), object_string_new_from("Clear Diagnostic Information"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DTC_INFORMATION), object_string_new_from("Read DTC Information"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_IDENTIFIER), object_string_new_from("Read Data By Identifier"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_MEMORY_BY_ADDRESS), object_string_new_from("Read Memory By Address"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_SCALING_DATA_BY_IDENTIFIER), object_string_new_from("Read Scaling Data By Identifier"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_SECURITY_ACCESS), object_string_new_from("Security Access"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_COMMUNICATION_CONTROL), object_string_new_from("Communication Control"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_AUTHENTICATION), object_string_new_from("Authentication"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_DATA_BY_PERIOD_IDENTIFIER), object_string_new_from("Read Data By Period Identifier"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_READ_FRESH_DATA_BY_IDENTIFIER), object_string_new_from("Read Fresh Data By Identifier"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_DYNAMICALLY_DEFINE_DATA_IDENTIFIER), object_string_new_from("Dynamically Define Data Identifier"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER), object_string_new_from("Write Data By Identifier"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER), object_string_new_from("Input Output Control By Identifier"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_ROUTINE_CONTROL), object_string_new_from("Routine Control"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_DOWNLOAD), object_string_new_from("Request Download"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_UPLOAD), object_string_new_from("Request Upload"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_TRANSFER_DATA), object_string_new_from("Transfer Data"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_TRANSFER_EXIT), object_string_new_from("Request Transfer Exit"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_REQUEST_FILE_TRANSFER), object_string_new_from("Request File Transfer"));
        object_hashmap_Int_string_set(service_to_string_map, object_Int_new_from(UDS_SERVICE_TESTER_PRESENT), object_string_new_from("Tester Present"));
    }
    object_string * result = object_hashmap_Int_string_get(service_to_string_map, object_Int_new_from(key));
    if ( result ) {
        return strdup(result->data);
    } else {
        return null;
    }
}