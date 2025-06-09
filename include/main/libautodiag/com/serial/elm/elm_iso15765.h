#ifndef __ELM_ISO15765_H
#define __ELM_ISO15765_H

#include "libautodiag/lib.h"
#include "libautodiag/math.h"

int oneHex(char c);

#define ELM_ISO15765_PARSE_RESPONSE_ITERATOR(ptr,end_ptr) \
    switch(elm->guess_response(ptr)) { \
        case DEVICE_RECV_DATA: { \
            log_msg(LOG_INFO, "Parsing one CAN message"); \
            memcpy(id_ascii,ptr,id_sz_chars); \
            ptr += id_sz_chars + elm->printing_of_spaces; \
            \
            final Buffer * address = buffer_from_ascii_hex_n(id_ascii,id_sz_chars); \
            final ECU* current_ecu = vehicle_ecu_add_if_not_in(vehicle, address->buffer, address->size); \
            buffer_free(address); \
            Iso15765Conversation *conversation = Iso15765Conversation_list_find(conversations, current_ecu); \
            \
            final Iso15765FrameType frame_type = oneHex(*ptr); \
            ptr ++; \
            \
            switch(frame_type) { \
                case Iso15765FirstFrame: \
                case Iso15765SingleFrame: { \
                    iso15765_conversation_free(conversation); \
                    conversation = null; \
                } \
            } \
            switch(frame_type) { \
                case Iso15765SingleFrame: { \
                    log_msg(LOG_DEBUG, "single frame"); \
                    final int data_bytes = oneHex(*ptr); \
                    ptr += 1 + elm->printing_of_spaces; \
                    conversation = iso15765_init_conversation(data_bytes); \
                    conversation->current_data_length = conversation->remaining_data_bytes_to_receive; \
                    Iso15765Conversation_list_append(conversations,conversation); \
                } break; \
                case Iso15765FirstFrame: { \
                    log_msg(LOG_DEBUG, "first frame"); \
                    final int data_bytes = (oneHex(*ptr) << 8) + (oneHex(*(ptr+1+elm->printing_of_spaces)) << 4) + oneHex(*(ptr+2+elm->printing_of_spaces)); \
                    ptr += 3 + elm->printing_of_spaces * 2; \
                    conversation = iso15765_init_conversation(data_bytes); \
                    conversation->current_data_length = min(CAN_MAX_BYTES_PER_MESSAGE-2,conversation->remaining_data_bytes_to_receive); \
                    Iso15765Conversation_list_append(conversations,conversation); \
                } break; \
                case Iso15765ConsecutiveFrame: { \
                    if ( conversation == null ) { \
                        log_msg(LOG_ERROR, "Conversation has not started properly"); \
                        final Buffer * bin_buffer = elm_ascii_to_bin_str((ELMDevice*)elm,ptr,end_ptr); \
                        buffer_dump(bin_buffer); \
                    } else { \
                        log_msg(LOG_DEBUG, "consecutive frame"); \
                        final int sequence_number = oneHex(*ptr); \
                        conversation->current_data_length = min(CAN_MAX_BYTES_PER_MESSAGE - 1,conversation->remaining_data_bytes_to_receive); \
                        conversation->current_sn = sequence_number; \
                    } \
                    ptr += 1 + elm->printing_of_spaces; \
                } break; \
                case Iso15765FlowControlFrame: { \
                    log_msg(LOG_WARNING, "flow control frame : Generally unused frame"); \
                } break; \
                default: { \
                    log_msg(LOG_ERROR, "Error unknown frame type received"); \
                    return false; \
                } \
            } \
            if ( conversation != null ) { \
                conversation->ecu = current_ecu; \
                final Buffer * bin_buffer = elm_ascii_to_bin_str((ELMDevice*)elm,ptr,end_ptr); \
                if ( bin_buffer == null ) { \
                    log_msg(LOG_ERROR, "Error while decoding the ascii buffer"); \
                    return false; \
                } \
                int position_in_buffer = 0; \
                switch(frame_type) { \
                    case Iso15765SingleFrame:           \
                    case Iso15765FirstFrame: \
                        break; \
                    case Iso15765ConsecutiveFrame: { \
                        position_in_buffer = \
                            conversation->current_sn*(CAN_MAX_BYTES_PER_MESSAGE-1) \
                            - 1; \
                        break; \
                    } \
                } \
                switch(frame_type) { \
                    case Iso15765SingleFrame:            \
                    case Iso15765ConsecutiveFrame: \
                    case Iso15765FirstFrame: { \
                            int sz = conversation->current_data_length; \
                            memcpy(conversation->data->buffer + position_in_buffer,bin_buffer->buffer,sz); \
                            conversation->remaining_data_bytes_to_receive -= sz; \
                            if ( conversation->remaining_data_bytes_to_receive == 0 ) { \
                                assert(conversation->ecu != null); \
                                if ( bin_buffer != null ) { \
                                    BufferList_append(conversation->ecu->obd_data_buffer,buffer_copy(conversation->data)); \
                                } \
                                if ( Iso15765Conversation_list_remove(conversations, conversation) ) { \
                                    log_msg(LOG_DEBUG, "Conversation removed"); \
                                    iso15765_conversation_free(conversation); \
                                } \
                                conversation = null; \
                                log_msg(LOG_DEBUG, "All data has been received"); \
                            } else if ( conversation->remaining_data_bytes_to_receive < 0 ) { \
                                log_msg(LOG_ERROR, "More data received than expected received"); \
                            } \
                        } break; \
                } \
                buffer_free(bin_buffer); \
            } \
            break; \
        } \
    }
    
#endif
