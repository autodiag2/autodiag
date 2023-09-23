#include "com/serial/kwp2000.h"

int kwp2000_parse_data(final OBDIFACE iface) {
    int service_id = -1;
    for(int i = 0; i < iface->obd_data_buffer->size; i++) {   
        final BUFFER buffer = iface->obd_data_buffer->list[i];
        if ( 0 < buffer->size_used ) {
            if ( buffer->buffer[0] == KWP2000_NEGATIVE_RESPONSE) {
                continue;
            } else {
                if ( (buffer->buffer[0] & KWP2000_POSITIVE_RESPONSE) == 0 ) {
                    log_msg("Error while parsing KWP2000 message", LOG_ERROR);
                } else {
                    service_id = buffer->buffer[0] - KWP2000_POSITIVE_RESPONSE;
                    if ( log_get_level() == LOG_DEBUG ) {
                        char *msg;
                        asprintf(&msg,"Found KWP2000 service %d positive response", service_id);
                        log_msg(msg, LOG_DEBUG);
                    }
                    buffer_left_shift(buffer,1);
                }
            }
        }
    }
    return service_id;
}
