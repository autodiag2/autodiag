#include "libautodiag/com/uds/uds.h"

static ad_object_hashmap_Int_string * service_to_string_map = null;
static ad_object_hashmap_Int_Int * service_to_response_map = null;
static ad_object_hashmap_Int_string * nrc_to_string_map = null;

char * uds_nrc_to_string(final UDS_NRC nrc) {
    if ( nrc_to_string_map == null ) {
        nrc_to_string_map = ad_object_hashmap_Int_string_new();
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_GENERAL_REJECT), ad_object_string_new_from("General reject"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SERVICE_NOT_SUPPORTED), ad_object_string_new_from("Service not supported"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SUBFUNCTION_NOT_SUPPORTED), ad_object_string_new_from("Sub-function not supported"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_IncorrectMessageLengthOrInvalidFormat), ad_object_string_new_from("Invalid message length/format"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_RESPONSE_TOO_LONG), ad_object_string_new_from("Response too long"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_BUSY_REPEAT_REQUEST), ad_object_string_new_from("Busy-repeat request"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_CONDITIONS_NOT_CORRECT), ad_object_string_new_from("Conditions not correct"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_REQUEST_SEQUENCE_ERROR), ad_object_string_new_from("Request sequence error"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_NO_RESPONSE_FROM_SUBNET), ad_object_string_new_from("No response from subnet component"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_FAILURE_PREVENTS_EXECUTION), ad_object_string_new_from("Failure prevents execution of requested action"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_REQUEST_OUT_OF_RANGE), ad_object_string_new_from("Request out of range"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SECURITY_ACCESS_DENIED), ad_object_string_new_from("Security access denied"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_INVALID_KEY), ad_object_string_new_from("Invalid key"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS), ad_object_string_new_from("Exceeded number of attempts"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED), ad_object_string_new_from("Required time delay has not expired"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED), ad_object_string_new_from("Upload/download not accepted"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_TRANSFER_DATA_SUSPENDED), ad_object_string_new_from("Transfer data suspended"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_PROGRAMMING_FAILURE), ad_object_string_new_from("Programming failure"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER), ad_object_string_new_from("Wrong block sequence counter"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_REQUEST_RECEIVED_RESPONSE_PENDING), ad_object_string_new_from("Request received - response pending"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_SESSION), ad_object_string_new_from("Sub function not supported in active session"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SERVICE_NOT_SUPPORTED_IN_SESSION), ad_object_string_new_from("Service not supported in active session"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_RPM_TOO_HIGH), ad_object_string_new_from("RPM too high"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_RPM_TOO_LOW), ad_object_string_new_from("RPM too low"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_ENGINE_RUNNING), ad_object_string_new_from("Engine is running"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_ENGINE_NOT_RUNNING), ad_object_string_new_from("Engine is not running"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_ENGINE_RUN_TIME_TOO_LOW), ad_object_string_new_from("Engine run time too low"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_TEMPERATURE_TOO_HIGH), ad_object_string_new_from("Temperature too high"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_TEMPERATURE_TOO_LOW), ad_object_string_new_from("Temperature too low"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SPEED_TOO_HIGH), ad_object_string_new_from("Speed too high"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SPEED_TOO_LOW), ad_object_string_new_from("Speed too low"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_THROTTLE_PEDAL_TOO_HIGH), ad_object_string_new_from("Throttle pedal too high"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_THROTTLE_PEDAL_TOO_LOW), ad_object_string_new_from("Throttle pedal too low"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_TRANSMISSION_RANGE_NOT_IN_NEUTRAL), ad_object_string_new_from("Transmission range not in neutral"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_TRANSMISSION_RANGE_NOT_IN_DEAR), ad_object_string_new_from("Transmission range not in dear"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_BRAKE_SWITCHES_NOT_CLOSED), ad_object_string_new_from("Brake switches not closed"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_SHIFTER_LEVEL_NOT_IN_PARK), ad_object_string_new_from("Shifter level not in park"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_TORK_CONVERTER_CLUTCH_LOCKED), ad_object_string_new_from("Tork converter clutch locked"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_VOLTAGE_TOO_HIGH), ad_object_string_new_from("Voltage too high"));
        ad_object_hashmap_Int_string_set(nrc_to_string_map, ad_object_Int_new_from(UDS_NRC_VOLTAGE_TOO_LOW), ad_object_string_new_from("Voltage too low"));
    }
    if ( 0xF0 <= nrc && nrc <= 0xFE ) {
        return strdup("Manufacturer specific conditions are not met");
    }
    ad_object_string * result = ad_object_hashmap_Int_string_get(nrc_to_string_map, ad_object_Int_new_from(nrc));
    if ( result == null ) {
        return null;
    } else {
        return result->data;
    }
}

char* uds_service_to_string(final UDSService key) {
    if ( service_to_string_map == null ) {
        service_to_string_map = ad_object_hashmap_Int_string_new();
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL), ad_object_string_new_from("Diagnostic Session Control"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_ECU_RESET), ad_object_string_new_from("ECU Reset"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION), ad_object_string_new_from("Clear Diagnostic Information"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_READ_DTC_INFORMATION), ad_object_string_new_from("Read DTC Information"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_READ_DATA_BY_IDENTIFIER), ad_object_string_new_from("Read Data By Identifier"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_READ_MEMORY_BY_ADDRESS), ad_object_string_new_from("Read Memory By Address"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_READ_SCALING_DATA_BY_IDENTIFIER), ad_object_string_new_from("Read Scaling Data By Identifier"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_SECURITY_ACCESS), ad_object_string_new_from("Security Access"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_COMMUNICATION_CONTROL), ad_object_string_new_from("Communication Control"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_AUTHENTICATION), ad_object_string_new_from("Authentication"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_READ_DATA_BY_PERIOD_IDENTIFIER), ad_object_string_new_from("Read Data By Period Identifier"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_DYNAMICALLY_DEFINE_DATA_IDENTIFIER), ad_object_string_new_from("Dynamically Define Data Identifier"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER), ad_object_string_new_from("Write Data By Identifier"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER), ad_object_string_new_from("Input Output Control By Identifier"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_ROUTINE_CONTROL), ad_object_string_new_from("Routine Control"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_REQUEST_DOWNLOAD), ad_object_string_new_from("Request Download"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_REQUEST_UPLOAD), ad_object_string_new_from("Request Upload"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_TRANSFER_DATA), ad_object_string_new_from("Transfer Data"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_REQUEST_TRANSFER_EXIT), ad_object_string_new_from("Request Transfer Exit"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_REQUEST_FILE_TRANSFER), ad_object_string_new_from("Request File Transfer"));
        ad_object_hashmap_Int_string_set(service_to_string_map, ad_object_Int_new_from(UDS_SERVICE_TESTER_PRESENT), ad_object_string_new_from("Tester Present"));
    }
    ad_object_string * result = ad_object_hashmap_Int_string_get(service_to_string_map, ad_object_Int_new_from(key));
    if ( result ) {
        return strdup(result->data);
    } else {
        return null;
    }
}