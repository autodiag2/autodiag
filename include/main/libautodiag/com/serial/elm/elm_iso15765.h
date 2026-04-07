#ifndef __AD_ELM_ISO15765_H
#define __AD_ELM_ISO15765_H

#include "libautodiag/lib.h"
#include "libautodiag/math.h"
#include "libautodiag/com/can/iso15765.h"

int oneHex(char c);

#define ELM_ISO15765_PARSE_RESPONSE_ITERATOR(ptr,end_ptr) \
    switch(elm->guess_response(ptr)) { \
        case DEVICE_RECV_DATA: { \
            log_msg(LOG_INFO, "Parsing one CAN message"); \
            if ( strlen(ptr) < id_sz_chars ) { \
                log_msg(LOG_ERROR, "Incoming frame is too short"); \
                return false; \
            } \
            memcpy(id_ascii,ptr,id_sz_chars); \
            if ( strlen(ptr) < (id_sz_chars + elm->printing_of_spaces) ) { \
                log_msg(LOG_ERROR, "Incoming frame is too short"); \
                return false; \
            } \
            ptr += id_sz_chars + elm->printing_of_spaces; \
            \
            final Buffer * address = ad_buffer_from_ascii_hex_n((char*)id_ascii,id_sz_chars); \
            if ( address == null ) { \
                log_msg(LOG_ERROR, "Data has been detected but address cannot be decoded, maybe it is not ISO15765 frame"); \
                return false; \
            } \
            final ad_object_ECU* current_ecu = vehicle_ecu_add_if_not_in(vehicle, address->buffer, address->size); \
            ad_buffer_free(address); \
            Iso15765Conversation *conversation = ad_list_Iso15765Conversation_find(conversations, current_ecu); \
            \
            if ( strlen(ptr) < 1 ) { \
                log_msg(LOG_ERROR, "Incoming frame is too short"); \
                return false; \
            } \
            final Iso15765FrameType frame_type = oneHex(*ptr); \
            ptr ++; \
            \
            switch(frame_type) { \
                case Iso15765FirstFrame: \
                case Iso15765SingleFrame: { \
                    iso15765_conversation_free(conversation); \
                    conversation = null; \
                } break; \
                default: break; \
            } \
            switch(frame_type) { \
                case Iso15765SingleFrame: { \
                    log_msg(LOG_DEBUG, "single frame"); \
                    if ( strlen(ptr) < (1 + elm->printing_of_spaces) ) { \
                        log_msg(LOG_ERROR, "Incoming frame is too short"); \
                        return false; \
                    } \
                    final int data_bytes = oneHex(*ptr); \
                    ptr += 1 + elm->printing_of_spaces; \
                    conversation = iso15765_init_conversation(data_bytes); \
                    conversation->current_data_length = conversation->remaining_data_bytes_to_receive; \
                    ad_list_Iso15765Conversation_append(conversations,conversation); \
                } break; \
                case Iso15765FirstFrame: { \
                    log_msg(LOG_DEBUG, "first frame"); \
                    if ( strlen(ptr) < (3 + elm->printing_of_spaces * 2) ) { \
                        log_msg(LOG_ERROR, "Incoming frame is too short"); \
                        return false; \
                    } \
                    final int data_bytes = (oneHex(*ptr) << 8) + (oneHex(*(ptr+1+elm->printing_of_spaces)) << 4) + oneHex(*(ptr+2+elm->printing_of_spaces)); \
                    ptr += 3 + elm->printing_of_spaces * 2; \
                    conversation = iso15765_init_conversation(data_bytes); \
                    conversation->current_data_length = min(CAN_MAX_BYTES_PER_MESSAGE-2,conversation->remaining_data_bytes_to_receive); \
                    ad_list_Iso15765Conversation_append(conversations,conversation); \
                } break; \
                case Iso15765ConsecutiveFrame: { \
                    if (strlen(ptr) < (1 + elm->printing_of_spaces)) { \
                        log_msg(LOG_ERROR, "Incoming frame is too short"); \
                        return false; \
                    } \
                    if (conversation == null) { \
                        log_msg(LOG_ERROR, "Conversation has not started properly"); \
                        final Buffer *bin_buffer = elm_ascii_to_bin_str((ELMDevice*)elm, ptr, end_ptr); \
                        ad_buffer_dump(bin_buffer); \
                    } else { \
                        final int sequence_number = oneHex(*ptr); \
                        final int expected_sequence_number = (conversation->current_sn + 1) & 0xF; \
                        int absolute_sn = (conversation->current_sn & ~0xF) | sequence_number; \
                        \
                        if (sequence_number != expected_sequence_number) { \
                            if (sequence_number == 0 && expected_sequence_number == 0) { \
                                absolute_sn += 0x10; \
                            } else { \
                                log_msg(LOG_ERROR, "Unexpected consecutive frame sequence number: got=%X expected=%X", sequence_number, expected_sequence_number); \
                                return false; \
                            } \
                        } else if (sequence_number == 0 && (conversation->current_sn & 0xF) == 0xF) { \
                            absolute_sn += 0x10; \
                        } \
                        \
                        conversation->current_data_length = min(CAN_MAX_BYTES_PER_MESSAGE - 1, conversation->remaining_data_bytes_to_receive); \
                        conversation->current_sn = absolute_sn; \
                        log_debug("consecutive frame with order : %d", conversation->current_sn); \
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
                    case Iso15765FlowControlFrame: \
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
                            if ( bin_buffer->size < sz ) { \
                                log_msg(LOG_ERROR, "Incoming frame is too short"); \
                                return false; \
                            } \
                            memcpy(conversation->data->buffer + position_in_buffer,bin_buffer->buffer,sz); \
                            conversation->remaining_data_bytes_to_receive -= sz; \
                            if ( conversation->remaining_data_bytes_to_receive == 0 ) { \
                                assert(conversation->ecu != null); \
                                if ( bin_buffer != null ) { \
                                    ad_list_Buffer_append(conversation->ecu->data_buffer,ad_buffer_copy(conversation->data)); \
                                } \
                                if ( ad_list_Iso15765Conversation_remove(conversations, conversation) ) { \
                                    log_msg(LOG_DEBUG, "Conversation removed"); \
                                    iso15765_conversation_free(conversation); \
                                } \
                                conversation = null; \
                                log_msg(LOG_DEBUG, "All data has been received"); \
                            } else if ( conversation->remaining_data_bytes_to_receive < 0 ) { \
                                log_msg(LOG_ERROR, "More data received than expected received"); \
                            } \
                        } break; \
                    default: break; \
                } \
                ad_buffer_free(bin_buffer); \
            } \
            break; \
        } \
    }

bool elm_iso15765_parse_response_internal(final void* elm_arg, ad_list_Iso15765Conversation* conversations, char * id_ascii, int id_sz_chars, void * vehicle_arg);

#endif
