#include "com/serial/can/uds/uds_frame_buffer.h"

UDSFrameBuffer* uds_frame_buffer_new() {
    UDSFrameBuffer *ptr = (UDSFrameBuffer*)malloc(sizeof(UDSFrameBuffer));
    ptr->size = 0;
    ptr->frames = null;
    return ptr;
}
void uds_frame_buffer_append(UDSFrameBuffer* buffer, UDSFrame *frame) {
    buffer->size ++;
    buffer->frames = (UDSFrame**)realloc(buffer->frames, sizeof(UDSFrame*) * buffer->size);
    buffer->frames[buffer->size-1] = frame;
}
void uds_frame_buffer_recycle(UDSFrameBuffer* buffer) {
    if ( buffer->frames != null ) {
        free(buffer->frames);
        buffer->frames = null;
    }
    buffer->size = 0;
}
void uds_frame_buffer_free(UDSFrameBuffer* buffer) {
    uds_frame_buffer_recycle(buffer);
}
