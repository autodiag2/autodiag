#ifndef __UDS_FRAME_Buffer *_H
#define __UDS_FRAME_Buffer *_H

#include "libautodiag/com/uds/uds.h"

/**
 * Holds a list of frames
 */
typedef struct {
    int size;
    UDSFrame **frames;
} UDSFrameBuffer;

/**
 * Create a new empty buffer
 */
UDSFrameBuffer* uds_frame_buffer_new();
/**
 * Append a frame to the buffer for a next use
 */
void uds_frame_buffer_append(UDSFrameBuffer* buffer, UDSFrame *frame);
/**
 * Recycle the buffer for a next use
 */
void uds_frame_buffer_recycle(UDSFrameBuffer* buffer);
/**
 * Free the buffer definitively
 */
void uds_frame_buffer_free(UDSFrameBuffer* buffer);

#define UDSFRAMEBuffer * UDSFrameBuffer*

#endif
