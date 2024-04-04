#ifndef __UDS_FRAME_BUFFER_H
#define __UDS_FRAME_BUFFER_H

#include "com/uds/uds.h"

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

#define UDSFRAMEBUFFER UDSFrameBuffer*

#endif
