#ifndef __UDS_H
#define __UDS_H

#include "libautodiag/lib.h"

typedef enum {
    UDS_FRAME_SF,UDS_FRAME_FF,UDS_FRAME_CF
} UDSFrameType;

typedef struct {
    /**
     * Frame type
     */
    UDSFrameType frame_type;
} UDSFrame;

typedef struct {
    UDSFrame;
    /**
     * Number of data bytes in this message (max 16 bytes with 4 bits)
     */
    int data_byte_number;
    /**
     * Service ID for UDS
     */
    int service_id;
    /**
     * Data bytes in the message
     */
    char data[16];
} UDSSimpleFrame;

typedef struct {
    UDSFrame;
    /**
     * Number of data bytes in this message (max 16 bytes with 4 bits)
     */
    int data_byte_number;
    /**
     * .... missing
     */
} UDSFirstFrame;

#define UDS_FRAME_CF_DATA_SIZE 7
typedef struct {
    UDSFrame;
    /**
     * Message serial number
     */
    int message_sn;
    /**
     * Frame data
     */
    char data[UDS_FRAME_CF_DATA_SIZE];
} UDSConsecutiveFrame;

/**
 * Easiers
 */
#define UDSFRAME UDSFrame *

/**
 * For given frame in buffer (in binary format) give corresponding data structure
 * @return decoded frame or null in case of decoding error result must be free'd
 */
UDSFrame * uds_decode_buffer_as(final BUFFER buffer, final int frame_type);
UDSFrame * uds_decode_buffer_auto(final BUFFER buffer);
UDSFrame * uds_decode_buffer_as_internal(final BUFFER buffer, final int frame_type, final int forceDataSize);
#endif
