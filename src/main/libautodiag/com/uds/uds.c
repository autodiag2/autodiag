#include "libautodiag/com/uds/uds.h"

UDSFrame * uds_decode_buffer_as_internal(final BUFFER buffer, final int frame_type, final int forceDataSize) {
    UDSFrame * result = null;
    final int sz = buffer->size;
    if ( 1 <= sz ) {
        final char byte_0 = buffer->buffer[0];
        switch(frame_type) {
            case UDS_FRAME_SF: {
                final int data_byte_number = forceDataSize == -1 ? byte_0 & 0x0F : forceDataSize;
                if ( (2 + data_byte_number) <= sz ) {
                    result = (UDSFRAME)malloc(sizeof(UDSSimpleFrame));
                    ((UDSSimpleFrame*)result)->data_byte_number = data_byte_number;
                    ((UDSSimpleFrame*)result)->service_id =  buffer->buffer[1];
                    memcpy(((UDSSimpleFrame*)result)->data,&buffer->buffer[2],((UDSSimpleFrame*)result)->data_byte_number);
                }
                break;
            }
            case UDS_FRAME_FF:
                result = (UDSFRAME)malloc(sizeof(UDSFirstFrame));
                ((UDSFirstFrame*)result)->data_byte_number = forceDataSize == -1 ? byte_0 & 0x0F : forceDataSize;
                log_msg(LOG_ERROR, "Unsupported frame type");
                break;
            case UDS_FRAME_CF:        
                result = (UDSFRAME)malloc(sizeof(UDSConsecutiveFrame));
                ((UDSConsecutiveFrame*)result)->message_sn = byte_0 & 0x0F;
                memcpy(((UDSConsecutiveFrame*)result)->data,&buffer->buffer[1],UDS_FRAME_CF_DATA_SIZE);
                break;
            default:
                log_msg(LOG_INFO, "Unknown frame type");
        }
        if ( result != null ) {
            result->frame_type = frame_type;
        }
    }
    return result;
}

UDSFrame * uds_decode_buffer_auto(final BUFFER buffer) {
    if ( buffer->size < 1 ) {
        final char byte_0 = buffer->buffer[0];
        final int frame_type = byte_0 & 0xF0;
        return uds_decode_buffer_as(buffer, frame_type);
    } else {
        return null;
    }
}

UDSFrame * uds_decode_buffer_as(final BUFFER buffer, final int frame_type) {
    return uds_decode_buffer_as_internal(buffer, frame_type, -1);
}

