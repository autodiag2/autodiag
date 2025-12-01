#include "libautodiag/com/obd/saej1979/stored_dtcs.h"

SAEJ1979_DTC * saej1979_dtc_new() {
    SAEJ1979_DTC * dtc = (SAEJ1979_DTC*)malloc(sizeof(SAEJ1979_DTC));
    dtc->description = list_DTC_DESCRIPTION_new();
    dtc->to_string = CAST_DTC_TO_STRING(saej1979_dtc_to_string);
    dtc->ecu = null;
    dtc->detection_method = list_object_string_new();
    return dtc;
}
void saej1979_dtc_free(SAEJ1979_DTC *dtc) {
    if ( dtc->description != null ) {
        list_DTC_DESCRIPTION_free(dtc->description);
        dtc->description = null;
    }
    dtc->ecu = null;
}

#define saej1979_dtcs_iterator(data_buffer) { \
    if ( result == null ) { \
        result = list_DTC_new(); \
    } \
    for(unsigned int byte = 0; 0 < data_buffer->size && byte < data_buffer->size-1; byte += 2) { \
        unsigned char byte_0 = data_buffer->buffer[byte]; \
        unsigned char byte_1 = data_buffer->buffer[byte+1]; \
        if ( byte_0 == 0 && byte_1 == 0 ) { \
            continue; \
        } else { \
            SAEJ1979_DTC * dtc = saej1979_dtc_new(); \
            dtc->data[0] = byte_0; \
            dtc->data[1] = byte_1; \
            dtc->data[2] = 0; \
            dtc->ecu = ecu; \
            dtc_description_fetch_from_fs(dtc, filter); \
            list_DTC_append(result, dtc); \
        } \
    } \
}

SAEJ1979_GENERATE_OBD_REQUEST_ITERATE(
                        list_DTC *,saej1979_retrieve_stored_dtcs,
                        "03",saej1979_dtcs_iterator,null,
                        ecu->obd_service.current_dtc, Vehicle *filter
                    )
SAEJ1979_GENERATE_OBD_REQUEST_ITERATE(
                        list_DTC *,saej1979_retrieve_pending_dtcs,
                        "07",saej1979_dtcs_iterator,null,
                        ecu->obd_service.pending_dtc, Vehicle *filter
                    )
SAEJ1979_GENERATE_OBD_REQUEST_ITERATE(
                        list_DTC *, saej1979_retrieve_permanent_dtcs,
                        "0A",saej1979_dtcs_iterator,null,
                        ecu->obd_service.permanent_dtc, Vehicle *filter
                    )

char * saej1979_dtc_to_string(final DTC * dtc) {
    final Buffer * dtc_bin = buffer_new();
    buffer_append_bytes(dtc_bin, dtc->data, DTC_DATA_SZ);
    final byte type = (dtc_bin->buffer[0] & 0xC0) >> 6;
    dtc_bin->buffer[0] = (dtc_bin->buffer[0] & (~0xC0)) ;
    dtc_bin->size -= 1;
    char *result;
    asprintf(&result, "%c%s",iso15031_dtc_type_first_letter(type),buffer_to_hex_string(dtc_bin));
    return result;
}
SAEJ1979_DTC * saej1979_dtc_from_string(final char *dtc_string) {
    return saej1979_dtc_from_bin(saej1979_dtc_bin_from_string(dtc_string));
}
ISO15031_DTC_TYPE saej1979_dtc_type(final DTC * dtc) {
    return (dtc->data[0] & 0xC0) >> 6;
}
Buffer * saej1979_dtc_number(final DTC * dtc) {
    final Buffer * result = buffer_new();
    buffer_append_bytes(result, dtc->data, DTC_DATA_SZ);
    result->buffer[0] = result->buffer[0] & (~0xC0);
    return result;
}
SAEJ1979_DTC * saej1979_dtc_from_bin(final Buffer * buffer) {
    assert(2 <= buffer->size);
    if ( 2 < buffer->size ) {
        log_msg(LOG_WARNING, "Warning more data received than expected by format");
    }
    SAEJ1979_DTC * dtc = saej1979_dtc_new();
    dtc->data[0] = buffer->buffer[0];
    dtc->data[1] = buffer->buffer[1];
    dtc->data[2] = 0; 
    return dtc;
}
Buffer* saej1979_dtc_bin_from_string(final char *dtc_string) {
    final Buffer * dtc_bin = buffer_from_ascii_hex(&(dtc_string[1]));
    if ( dtc_bin == null ) {
        return null;
    } else {
        ISO15031_DTC_TYPE dtc_type = iso15031_dtc_first_letter_to_type(dtc_string[0]);
        if ( ISO15031_DTC_TYPE_UNKNOWN == dtc_type ) {
            buffer_free(dtc_bin);
            return null;
        } else {
            dtc_bin->buffer[0] |= dtc_type << 6;
            return dtc_bin;
        }
    }
}

char * saej1979_dtc_explanation(final DTC * dtc) {
    char * res;
    final Buffer * dtc_number = saej1979_dtc_number(dtc);
    final char * dtc_number_str = buffer_to_hex_string(dtc_number);
    char * sub = iso15031_dtc_to_subsystem_string(dtc_number_str);
    char * type = iso15031_dtc_type_to_string(saej1979_dtc_type(dtc));
    asprintf(&res,"SAEJ1979(OBD):\nFault on %s, this is a %s DTC\nIt is related to %s\n",
        type, iso15031_dtc_is_generic_code(dtc_number_str[0]) ? "generic" : "manufacturer specific",
        sub  
    );
    free(sub);
    free(type);
    return res;
}

