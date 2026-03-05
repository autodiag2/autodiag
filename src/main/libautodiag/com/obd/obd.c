#include "libautodiag/com/obd/obd.h"

bool obd_standard_parse_buffer(final Vehicle* vehicle, final Buffer* bin_buffer) {
    if ( 2 < bin_buffer->size ) {
        byte priority = ad_buffer_extract_0(bin_buffer); 
        byte receiver_address = ad_buffer_extract_0(bin_buffer); 
        byte emitter_address = ad_buffer_extract_0(bin_buffer); 
        
        final Buffer * address = ad_buffer_new(); 
        ad_buffer_append_byte(address,emitter_address); 
        final ECU* ecu = vehicle_ecu_add_if_not_in(vehicle, address->buffer, address->size); 
        ad_buffer_free(address); 
        
        list_Buffer_append(ecu->data_buffer,ad_buffer_copy(bin_buffer)); 
        return true;
    } else { 
        log_msg(LOG_DEBUG, "Not enough data received"); 
        return false;
    } 
}