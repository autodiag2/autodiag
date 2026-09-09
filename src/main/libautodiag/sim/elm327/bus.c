#include "libautodiag/sim/elm327/bus.h"
#include "libautodiag/com/socketcan.h"

/**
 * Request by the tester to the vehicle (emu).
 * @param ecu to which send the dataRequest
 * @param dataRequest OBD/UDS
 * @return header to send to the vehicle (emu).
 */
static Buffer* request_header(SimELM327* elm327, SimECU * ecu) {
    log_msg(LOG_DEBUG, "Generating the tester request header");
    Buffer *protocolSpecificHeader = ad_buffer_new();
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            if ( elm327->custom_header->size == 3 ) {
                AD_BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->can.priority_29bits,
                    elm327->custom_header->buffer[0],
                    elm327->custom_header->buffer[1], 
                    elm327->custom_header->buffer[2]
                );
            } else if ( elm327->custom_header->size == 4 ) {
                ad_buffer_append(protocolSpecificHeader, elm327->custom_header);
            } else {
                AD_BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->can.priority_29bits, 0xDA, 0xF1, elm327->testerAddress
                );
            }
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            if ( elm327->custom_header->size == 2 ) {
                AD_BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->custom_header->buffer[0]&0xF, 
                    elm327->custom_header->buffer[1]
                );
            } else if ( elm327->custom_header->size == 3 ) {
                AD_BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->custom_header->buffer[1]&0xF, 
                    elm327->custom_header->buffer[2]
                );
            } else {
                AD_BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    0x7, elm327->testerAddress
                );
            }
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
        if ( elm327->can.extended_addressing ) {
            AD_BUFFER_APPEND_BYTES(protocolSpecificHeader,
                elm327->can.extended_addressing_target_address
            );
        }
    } else {
        if ( elm327->custom_header->size == 3 ) {
            ad_buffer_append(protocolSpecificHeader, elm327->custom_header);
        } else {
            AD_BUFFER_APPEND_BYTES(protocolSpecificHeader,
                0x41, 0x6B, elm327->testerAddress
            );
        }
    }
    return protocolSpecificHeader;     
}
/**
 * Split if needed the request into multiple ones
 */
static ad_list_Buffer * request_frames(SimELM327* elm327, SimECU * ecu, Buffer * dataRequest) {
    Buffer * requestHeader = request_header(elm327, ecu);
    ad_list_Buffer * requestFrames = ad_list_Buffer_new();
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        Buffer * one_frame = ad_buffer_new();
        ad_list_Buffer_append(requestFrames, one_frame);
        if ( elm327->can.auto_format ) {
            ad_buffer_append(one_frame, requestHeader);
            if ( AD_ISO15765_SINGLE_FRAME_MAX_BYTES < dataRequest->size ) {
                int data_length = dataRequest->size;
                ad_buffer_append_byte(one_frame, (Iso15765FirstFrame << 4) | ((data_length >> 8) & 0xF));
                ad_buffer_append_byte(one_frame, data_length & 0xFF);
                ad_buffer_slice_append(one_frame, dataRequest, 0, AD_ISO15765_FIRST_FRAME_MAX_BYTES);
                for(int i = AD_ISO15765_FIRST_FRAME_MAX_BYTES, sn = 1; i < dataRequest->size; i += AD_ISO15765_CONSECUTIVE_FRAME_MAX_BYTES, sn ++) {
                    Buffer * consecutive_frame = ad_buffer_new();
                    ad_buffer_append(consecutive_frame, requestHeader);
                    ad_buffer_append_byte(consecutive_frame, (Iso15765ConsecutiveFrame << 4) | sn % 16);
                    int upper_bound = dataRequest->size - i;
                    ad_buffer_slice_append(consecutive_frame, dataRequest, i, min(AD_ISO15765_CONSECUTIVE_FRAME_MAX_BYTES, upper_bound));
                    ad_list_Buffer_append(requestFrames, consecutive_frame);
                }
            } else {
                final byte pci = dataRequest->size | Iso15765SingleFrame;
                ad_buffer_append_byte(one_frame, pci);
                ad_buffer_append(one_frame, dataRequest);
            }
        } else {
            ad_buffer_append(one_frame, dataRequest);
        }
    } else {
        log_debug("For now non can devices have infinite length");
        Buffer * request_frame = ad_buffer_new();
        ad_buffer_append(request_frame, requestHeader);
        ad_buffer_append(request_frame, dataRequest);
        ad_list_Buffer_append(requestFrames, request_frame);
    }
    return requestFrames;
}
/**
 * Generate the response header to the tester by the bus.
 */
static Buffer* response_header(SimELM327* elm327,SimECU * ecu, byte can28bits_prio) {
    final Buffer * header = ad_buffer_new();
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            AD_BUFFER_APPEND_BYTES(header, 
                can28bits_prio,
                0xDA,
                elm327->testerAddress,
                ecu->address
            );
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            AD_BUFFER_APPEND_BYTES(header, 
                0x7,
                ecu->address
            );
        } else {
            log_msg(LOG_ERROR, "Missing case here");
            assert( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) || 
                elm327_protocol_is_can_11_bits_id(elm327->protocolRunning)
            );
        }
    } else {
        AD_BUFFER_APPEND_BYTES(header, 
            0x41, 
            elm327->testerAddress,
            ecu->address
        );
    }
    return header;     
}
static Buffer * response_frame_extract_header(final SimELM327* elm327, final Buffer * frame) {
    final Buffer * header = ad_buffer_new();
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            ad_buffer_slice_append(header, frame, 0, 4);
            ad_buffer_left_shift(frame, 4);
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            ad_buffer_slice_append(header, frame, 0, 2);
            ad_buffer_left_shift(frame, 2);
        } else {
            log_msg(LOG_ERROR, "Missing case here");
            assert( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) || 
                elm327_protocol_is_can_11_bits_id(elm327->protocolRunning)
            );
        }
    } else {
        ad_buffer_slice_append(header, frame, 0, 3);
        ad_buffer_left_shift(frame, 3);
    }
    return header;
}
static ad_list_Buffer * response_frames(SimELM327* elm327, SimECU * ecu, Buffer * dataResponse) {
    ad_list_Buffer * result = ad_list_Buffer_new();
    bool iso_15765_is_multi_message = false;
    int iso_15765_multi_message_sn = 0;
    int transportLayerMessageDataBytes = 0;
    for(int responseBodyIndex = 0; responseBodyIndex < dataResponse->size; responseBodyIndex += transportLayerMessageDataBytes, iso_15765_multi_message_sn += 1) {
        
        final Buffer * responseBodyChunk = ad_buffer_new();
        bool iso_15765_is_multi_message_ff = false;

        if ( responseBodyIndex == 0 ) {
            iso_15765_is_multi_message = 7 < dataResponse->size;
            if ( iso_15765_is_multi_message ) {
                iso_15765_is_multi_message_ff = true;
            }
        }

        int transportLayerMessageDataBytesMax = 7;
        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            if ( iso_15765_is_multi_message ) {
                if ( iso_15765_is_multi_message_ff ) {
                    transportLayerMessageDataBytesMax = 6;
                }
            }
        }
        transportLayerMessageDataBytes = min(transportLayerMessageDataBytesMax - responseBodyChunk->size, dataResponse->size - responseBodyIndex);
        ad_buffer_slice_append(responseBodyChunk, dataResponse, responseBodyIndex, transportLayerMessageDataBytes);

        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            if ( iso_15765_is_multi_message ) {
                if ( iso_15765_is_multi_message_ff ) {
                    log_msg(LOG_DEBUG, "reply first frame");
                    int bytesSent = dataResponse->size;
                    int dl11_8 = (bytesSent & 0x0F00) >> 8;
                    final byte pci = Iso15765FirstFrame << 4 | dl11_8;
                    final byte dl7_0 = bytesSent & 0xFF;
                    ad_buffer_prepend_byte(responseBodyChunk, dl7_0);
                    ad_buffer_prepend_byte(responseBodyChunk, pci);
                } else {
                    log_msg(LOG_DEBUG, "reply consecutive frame");
                    final byte pci = Iso15765ConsecutiveFrame << 4 | iso_15765_multi_message_sn % 16;
                    ad_buffer_prepend_byte(responseBodyChunk, pci);
                }
            } else {
                log_msg(LOG_DEBUG, "reply as single frame");
                final byte pci = Iso15765SingleFrame | responseBodyChunk->size;
                ad_buffer_prepend_byte(responseBodyChunk, pci);
            }
        }
        final Buffer * protocolHeader = response_header(elm327, ecu, ELM327_CAN_28_BITS_DEFAULT_PRIO);
        ad_buffer_prepend(responseBodyChunk, protocolHeader);
        ad_list_Buffer_append(result, responseBodyChunk);
        ad_buffer_free(protocolHeader);
    }
    return result;
}
/**
 * Generate the response header by ELM device back to user through serial line.
 */
static char * elm327_response_header_str(SimELM327* elm327, Buffer * header_src) {
    Buffer * header = ad_buffer_copy(header_src);
    char *protocolSpecificHeader = null;
    char *protocolSpecificHeaderFreeLocation = null;
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327->can.extended_addressing ) {
            ad_buffer_append_byte(header, elm327->can.extended_addressing_target_address);
        }
        protocolSpecificHeader = elm_ascii_from_bin(elm327->printing_of_spaces, header);
        protocolSpecificHeaderFreeLocation = protocolSpecificHeader;
        if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            assert(3 <= strlen(protocolSpecificHeader));
            protocolSpecificHeader ++;
            if ( elm327->printing_of_spaces ) {
                *(protocolSpecificHeader+1) = *protocolSpecificHeader;
                protocolSpecificHeader ++;
            }
        }
    } else {
        protocolSpecificHeader = elm_ascii_from_bin(elm327->printing_of_spaces, header);
        protocolSpecificHeaderFreeLocation = protocolSpecificHeader;
    }
    if ( protocolSpecificHeader[strlen(protocolSpecificHeader)-1] == ' ' ) {
        protocolSpecificHeader[strlen(protocolSpecificHeader)-1] = 0x00;
    }
    ad_buffer_free(header);
    char * result = null;
    if ( protocolSpecificHeader != null ) {
        result = strdup(protocolSpecificHeader); 
    }
    if ( protocolSpecificHeaderFreeLocation != null ) {
        free(protocolSpecificHeaderFreeLocation);
    }
    return result;
}
bool sim_elm327_parse_request(SimELM327 * elm327, char * hex_string_request, bool * isHexString_rv, bool * hasSpaces_rv) {
    for(int i = 0; i < strlen(hex_string_request); i++) {
        char c = hex_string_request[i];
        if ( c == ' ' ) {
            if ( hasSpaces_rv != null ) {
                *hasSpaces_rv = true;
            }
        }
        if ( !( c == SIM_ELM327_PP_GET(elm327,0x0D) || c == SIM_ELM327_PP_GET(elm327,0x0A) || c == ' ' || 0x30 <= c && c <= 0x39 || 0x41 <= c && c <= 0x46 || 0x61 <= c && c <= 0x66 ) ) {
            if ( isHexString_rv != null ) {
                *isHexString_rv = false;
            }
        }
    }
    return true;
}
char * sim_elm327_hex_string_request_reduce(SimELM327 * elm327, char * hex_string_request) {
    char * result = (char*)malloc(sizeof(char) * (strlen(hex_string_request) + 1));
    int j = 0;
    for(int i = 0; i < strlen(hex_string_request); i++) {
        char c = hex_string_request[i];
        bool isEol = false;
        for(int k = 0; k < strlen(elm327->eol); k++) {
            if ( elm327->eol[k] == c ) {
                isEol = true;
            }
        }
        if ( c == ' ' || isEol ) {

        } else {
            result[j++] = c;
        }
    }
    result[j] = 0x00;
    return result;
}
bool elm327_protocol_is_iso(int protocol) {
    return ELM327_PROTO_ISO_9141_2 <= protocol && protocol <= ELM327_PROTO_ISO_14230_4_KWP2000_2;
}
static void response_frame_add_checksum(SimELM327 * elm327, Buffer * frame) {
    if ( ELM327_PROTO_ISO_9141_2 <= elm327->protocolRunning && elm327->protocolRunning <= ELM327_PROTO_ISO_14230_4_KWP2000_2 ) {
        byte computed_checksum = 0;
        for(int i = 0; i < frame->size; i++) {
            computed_checksum += frame->buffer[i];
        }
        ad_buffer_append_byte(frame, computed_checksum);
    } else if ( elm327->protocolRunning == ELM327_PROTO_SAE_J1850_1 || ELM327_PROTO_SAE_J1850_2 == elm327->protocolRunning ) {
        byte computed_checksum = j1850_crc(frame);
        ad_buffer_append_byte(frame, computed_checksum);
    }
}
static void ELM327RequestFrameTracker_free_conv(ELM327RequestFrameTracker *holder) {
    if (holder == null) {
        return;
    }

    if (holder->conversation == null) {
        holder->is_can = false;
        return;
    }

    if (holder->conversation->value != null) {
        if (holder->is_can) {
            iso15765_conversation_free(holder->conversation->value);
        } else {
            ad_buffer_free(holder->conversation->value);
        }
    }

    holder->conversation->value = null;
    ad_object_Ptr_free(holder->conversation);
    holder->conversation = null;
    holder->is_can = false;
}
static void ELM327RequestFrameTracker_free(ELM327RequestFrameTracker *holder) {
    if (holder != null) {
        ELM327RequestFrameTracker_free_conv(holder);
        free(holder);
    }
}
static void * ELM327RequestFrameTracker_init_conv(ELM327RequestFrameTracker * holder, bool is_can, int bytes) {
    ELM327RequestFrameTracker_free_conv(holder);

    holder->is_can = is_can;
    holder->conversation = ad_object_Ptr_new();

    if (is_can) {
        Iso15765Conversation *conversation = iso15765_init_conversation(bytes);
        holder->conversation->value = conversation;
        return conversation;
    }

    Buffer *buf = ad_buffer_new();
    holder->conversation->value = buf;
    return buf;
}
static ELM327RequestFrameTracker * ELM327RequestFrameTracker_new() {
    ELM327RequestFrameTracker * holder = (ELM327RequestFrameTracker*)malloc(sizeof(ELM327RequestFrameTracker));
    holder->is_can = false;
    holder->conversation = null;
    return holder;
}
static ELM327RequestFrameTracker* sim_ecu_process_frame(SimELM327 * elm327, SimECU * ecu, Buffer * frame, char ** errorCauseReturn) {
    Buffer * requestFrameHeader = ad_buffer_new();
    Buffer * senderAddress = null;
    Buffer * requestFrameLessId = ad_buffer_new();
    ELM327RequestFrameTracker * completed_flow = null;
    log_msg(LOG_DEBUG, "Receving incoming request by the tester %s (proto: %d)", ad_buffer_to_hex_string(frame), elm327->protocolRunning);
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        AdCanFrame socket_can_frame = {0};
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            assert(4 <= frame->size);
            ad_buffer_slice_append(requestFrameHeader, frame, 0, 4);
            ad_buffer_slice_append(requestFrameLessId, frame, 4, frame->size - 4);
            senderAddress = ad_buffer_copy(requestFrameHeader);
            if ( elm327->socketcan ) {
                socket_can_frame.id = ad_buffer_to_be32(senderAddress);
            }
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            assert(2 <= frame->size);
            ad_buffer_slice_append(requestFrameHeader, frame, 0, 2);
            ad_buffer_slice_append(requestFrameLessId, frame, 2, frame->size - 2);
            senderAddress = ad_buffer_copy(requestFrameHeader);
            if ( elm327->socketcan ) {
                socket_can_frame.id = ad_buffer_to_be16(senderAddress);
            }
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
        if ( elm327->socketcan ) {
            int sz = min(requestFrameLessId->size, 64);
            if ( sz < requestFrameLessId->size ) {
                log_warn("Truncating response frame from %d to %d bytes for socketcan", requestFrameLessId->size, sz);
            }
            memcpy(socket_can_frame.data, requestFrameLessId->buffer, sz);
            socket_can_frame.size = sz;
            ad_socketcan_send(elm327->socketcan, &socket_can_frame);
        }
        ad_buffer_free(requestFrameLessId);
        ad_buffer_left_shift(frame, requestFrameHeader->size);
        if ( elm327->can.extended_addressing ) {
            ad_buffer_left_shift(frame, 1);
        }
        assert(0 < frame->size);
        byte pci = ad_buffer_extract_0(frame);
        byte pci_ft = pci >> 4;
        switch(pci_ft) {
            case Iso15765SingleFrame: {
                log_debug("single frame");
                int data_length = pci & 0x0F;
                ad_object_Ptr * ptr = ad_simECU_conversation_get_by_address(ecu, senderAddress);
                if ( ptr != null ) {
                    ELM327RequestFrameTracker * holder = ptr->value;
                    ELM327RequestFrameTracker_free_conv(holder);
                    log_warn("dropping existing conversation for single frame request");
                } else {
                    ptr = ad_object_Ptr_new();
                    ptr->value = ELM327RequestFrameTracker_new();
                }
                int current_data_length = data_length;
                Iso15765Conversation * conversation = ELM327RequestFrameTracker_init_conv(ptr->value, true, data_length);
                ELM327RequestFrameTracker * holder = ptr->value;
                conversation->current_sn = 0;
                conversation->current_data_length = current_data_length;
                conversation->remaining_data_bytes_to_receive -= current_data_length;
                conversation->data = ad_buffer_copy(frame);
                ad_simECU_conversation_set_by_address(ecu, senderAddress, ptr);
                senderAddress = null;
                if ( data_length != frame->size ) {
                    if ( elm327->can.auto_format ) {
                        log_msg(LOG_WARNING, "Generated pci is different than the actual request size (%d/%d)", data_length, frame->size);
                    } else {
                        log_msg(LOG_WARNING, "Single frame pci size does not match (%d/%d)", data_length, frame->size);
                        if ( errorCauseReturn != null ) {
                            *errorCauseReturn = strdup(ELM327ResponseStr[ELM327_RESPONSE_DATA_ERROR_AT_LINE-ELM327_RESPONSE_OFFSET]);
                        }
                        goto cleanup;
                    }
                }   
                completed_flow = holder;
            } break;
            case Iso15765FirstFrame: {
                log_debug("first frame");
                assert(0 < frame->size);
                byte pci2 = ad_buffer_extract_0(frame);
                int data_length = ((pci & 0x0F) << 8) + pci2;
                int current_data_length = frame->size;
                ad_object_Ptr * ptr = ad_simECU_conversation_get_by_address(ecu, senderAddress);
                Iso15765Conversation * conversation;
                if ( ptr != null ) {
                    ELM327RequestFrameTracker * holder = ptr->value;
                    if ( holder->conversation != null ) {
                        ELM327RequestFrameTracker_free_conv(holder);
                        log_warn("dropping existing conversation for first frame request");
                    }
                } else {
                    ptr = ad_object_Ptr_new();
                    ptr->value = ELM327RequestFrameTracker_new();
                }
                conversation = ELM327RequestFrameTracker_init_conv(ptr->value, true, data_length);
                conversation->current_sn = 0;
                conversation->current_data_length = current_data_length;
                conversation->remaining_data_bytes_to_receive -= current_data_length;
                conversation->data = ad_buffer_copy(frame);
                ad_simECU_conversation_set_by_address(ecu, senderAddress, ptr);
                senderAddress = null;
                log_debug("todo : data length check (for user generated headers for example)");
            } break;
            case Iso15765ConsecutiveFrame: {
                log_debug("consecutive frame");
                ad_object_Ptr * ptr = ad_simECU_conversation_get_by_address(ecu, senderAddress);
                Iso15765Conversation * conversation = null;
                if ( ptr != null ) {
                    ELM327RequestFrameTracker * holder = ptr->value;
                    assert(holder->is_can);
                    conversation = holder->conversation->value;
                }
                if ( conversation == null ) {
                    log_warn("dropping consecutive frame request without first frame");
                    if ( errorCauseReturn != null ) {
                        *errorCauseReturn = strdup(ELM327ResponseStr[ELM327_RESPONSE_DATA_ERROR_AT_LINE-ELM327_RESPONSE_OFFSET]);
                    }
                    goto cleanup;
                }
                int sn = pci & 0xF;
                int current_data_length = frame->size;
                conversation->current_sn = sn;
                conversation->current_data_length = current_data_length;
                if (current_data_length > conversation->remaining_data_bytes_to_receive) {
                    log_warn("ISO-TP frame exceeds declared data length - ignoring excess bytes");
                    current_data_length = conversation->remaining_data_bytes_to_receive;
                }
                conversation->remaining_data_bytes_to_receive -= current_data_length;
                ad_buffer_slice_append(conversation->data, frame, 0, current_data_length);
                log_debug("todo : order check (for user generated headers for example)");
                if ( conversation->remaining_data_bytes_to_receive == 0 ) {
                    completed_flow = ptr->value;
                }
            } break;
            case Iso15765FlowControlFrame: {
                log_debug("flow control frame - ignoring");
            } break;
        }
    } else {
        assert(3 <= frame->size);
        ad_buffer_slice_append(requestFrameHeader, frame, 0, 3);
        ad_buffer_left_shift(frame, requestFrameHeader->size);
        if ( 7 < frame->size ) {
            log_warn("Undefined behaviour, wanted to send %d bytes over elm327 device (ignore extract bytes or send to KWP2000)", frame->size);
            log_warn("Use all the bytes for now");
        }
        senderAddress = ad_buffer_from_bytes(&requestFrameHeader->buffer[2], 1);
        ad_object_Ptr * ptr = ad_simECU_conversation_get_by_address(ecu, senderAddress);
        if (ptr == null) {
            ptr = ad_object_Ptr_new();
            ptr->value = ELM327RequestFrameTracker_new();
            ad_simECU_conversation_set_by_address(ecu, senderAddress, ptr);
            senderAddress = null;
        }

        ELM327RequestFrameTracker *holder = ptr->value;
        ELM327RequestFrameTracker_init_conv(holder, false, 0);
        Buffer * conversation = holder->conversation->value;
        ad_buffer_append(conversation, frame);
        completed_flow = holder;
    }
cleanup:
    ad_buffer_free(requestFrameHeader);
    ad_buffer_free(senderAddress);
    return completed_flow;
}
static Buffer *sim_ecu_generate_response(SimELM327 * elm327, SimECU * ecu, ELM327RequestFrameTracker * holder) {
    Buffer * dataRequest = null;
    if ( holder->is_can ) {
        Iso15765Conversation * iso15765_conversation = holder->conversation->value;
        assert(iso15765_conversation != null);
        if ( iso15765_conversation->remaining_data_bytes_to_receive != 0 ) {
            log_warn("incomplete data flow - %s (%d remaining) - dropping", iso15765_conversation->remaining_data_bytes_to_receive, ad_buffer_to_hex_string(iso15765_conversation->data));
        } else {
            dataRequest = ad_buffer_copy(iso15765_conversation->data);
        }
    } else {
        Buffer * data = holder->conversation->value;
        dataRequest = ad_buffer_copy(data);
    }
    return sim_ecu_response(ecu, dataRequest);
}
static void bus_lock(SimELM327 * elm327) {
    SimELM327Implementation * impl = (SimELM327Implementation*)elm327->implementation;
    pthread_mutex_lock(&impl->bus_lock);
}
static void bus_unlock(SimELM327 * elm327) {
    SimELM327Implementation * impl = (SimELM327Implementation*)elm327->implementation;
    pthread_mutex_unlock(&impl->bus_lock);
}
char * sim_elm327_bus(SimELM327 * elm327, char * hex_string_request, ad_list_Buffer * framesDirectInjection) {
    bool isHexString = true;
    char *response = strdup("");
    final Buffer * dataRequest = ad_buffer_new();
    if ( hex_string_request != null ) {
        sim_elm327_parse_request(elm327, hex_string_request, &isHexString, null);
        if ( ! isHexString ) {
            return null;
        }
        if ( ! elm327->protocol_is_auto_running ) {
            if ( elm327_protocol_is_iso(elm327->protocolRunning) ) {
                bool response_needed = false;
                if ( ! elm327->iso.bus_initialized ) {
                    if ( elm327->iso.bus_init_start == 0 ) {
                        elm327->iso.bus_init_start = time_ms();
                    }
                    
                    if ( SIM_ELM327_ISO_BUS_INIT_SLOW_MS < (time_ms() - elm327->iso.bus_init_start) ) {
                        elm327->iso.bus_initialized = true;
                        response_needed = true;
                    }
                    if ( elm327->protocolRunning == ELM327_PROTO_ISO_14230_4_KWP2000_1 || 
                        elm327->protocolRunning == ELM327_PROTO_ISO_14230_4_KWP2000_2
                    ) {
                        if ( SIM_ELM327_ISO_14230_BUT_INIT_FAST_MS < (time_ms() - elm327->iso.bus_init_start) ) {
                            elm327->iso.bus_initialized = true;
                            response_needed = true;
                        }
                    }
                }
                if ( response_needed ) {
                    response = gprintf("BUS INIT: OK");
                } else {
                    if ( ! elm327->iso.bus_initialized ) {
                        return gprintf("BUS INIT: ...");
                    }
                }
            }
        }
        if ( ! elm327->responses ) {
            return strdup("");
        }
        char * hex_string_req_reduced = sim_elm327_hex_string_request_reduce(elm327, hex_string_request);
        int hex_string_req_reduced_sz = strlen(hex_string_req_reduced);
        if ( ( hex_string_req_reduced_sz % 2 ) != 0 ) {
            log_msg(LOG_DEBUG, "Ignoring optimization request '%s'", hex_string_req_reduced);
            hex_string_req_reduced[--hex_string_req_reduced_sz] = 0x00;
        }
        elm_ascii_to_bin_internal(false, dataRequest, hex_string_req_reduced, hex_string_req_reduced + hex_string_req_reduced_sz);
        free(hex_string_req_reduced);
        log_msg(LOG_DEBUG, "TODO: handle multiple data request");
    }
    bus_lock(elm327);
    for(int i = 0; i < LIST_SIM_ECU(elm327->ecus)->size; i++) {
        SimECU * ecu = LIST_SIM_ECU(elm327->ecus)->list[i];

        if ( ! elm327_protocol_is_j1939(elm327->protocolRunning) ) {
            if ( elm327->receive_address != null && *elm327->receive_address != ecu->address) {
                continue;
            }
        }

        ad_list_Buffer * requestFrames = null;
        if ( framesDirectInjection != null && hex_string_request == null ) {
            requestFrames = framesDirectInjection;
        } else {
            assert(hex_string_request);
            requestFrames = request_frames(elm327, ecu, dataRequest);
        }
        char * ecuResponse = null;

        ecu->generator->flavour.is_Iso15765_4 = elm327_protocol_is_can(elm327->protocolRunning);

        ELM327RequestFrameTracker * completed_flow = null;
        for(int rq_frame_i = 0; rq_frame_i < requestFrames->size; rq_frame_i++) {
            char *errorCauseReturn = null;
            Buffer * requestFrame = requestFrames->list[rq_frame_i];
            completed_flow = sim_ecu_process_frame(elm327, ecu, requestFrame, &errorCauseReturn);
            if ( errorCauseReturn != null ) {
                log_warn("Error processing frame: %s", errorCauseReturn);
                ecuResponse = strdup(errorCauseReturn);
                free(errorCauseReturn);
            }
            if ( completed_flow ) {
                break; // for now
            }
        }
        
        final Buffer * dataResponse = ad_buffer_new();
        if ( completed_flow ) {
            ad_buffer_append_melt(dataResponse, sim_ecu_generate_response(elm327, ecu, completed_flow));
        }

        if ( 0 < dataRequest->size && dataRequest->buffer[0] == OBD_SERVICE_CLEAR_DTC ) {
            if ( 0 < dataResponse->size ) {
                if ( (dataResponse->buffer[0] & OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE) == OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE ) {
                    log_msg(LOG_DEBUG, "DTCs cleared request received, replying OK (elm327 style)");
                    ad_buffer_recycle(dataResponse);
                    ecuResponse = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
                }
            }
        }

        if ( ecuResponse == null && 0 < dataResponse->size ) {
            ad_list_Buffer * frames = response_frames(elm327, ecu, dataResponse);

            if ( ! elm327->printing_of_headers ) {
                if ( 1 < frames->size ) {
                    log_msg(LOG_DEBUG, "Should add the multiple single frames format");
                    asprintf(&ecuResponse, "%03d%s", dataResponse->size, elm327->eol);
                }
            }
            for(int frame_idx = 0; frame_idx < frames->size; frame_idx++) {
                Buffer * frame = frames->list[frame_idx];
                char * header = null;
                if ( elm327->printing_of_headers ) {
                    response_frame_add_checksum(elm327, frame);
                }
                Buffer * headerBin = response_frame_extract_header(elm327, frame);

                if ( elm327->socketcan && elm327_protocol_is_can(elm327->protocolRunning) ) {
                    AdCanFrame socket_can_frame = {0};
                    if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning)) {
                        socket_can_frame.id = ad_buffer_to_be16(headerBin);
                    } else {
                        socket_can_frame.id = ad_buffer_to_be32(headerBin);
                    }
                    int sz = min(frame->size, 64);
                    if ( sz < frame->size ) {
                        log_warn("Truncating response frame from %d to %d bytes for socketcan", frame->size, sz);
                    }
                    memcpy(socket_can_frame.data, frame->buffer, sz);
                    socket_can_frame.size = sz;
                    ad_socketcan_send(elm327->socketcan, &socket_can_frame);
                }

                if ( elm327->printing_of_headers ) {
                    header = elm327_response_header_str(elm327, headerBin);
                } else {
                    if ( elm327_protocol_is_can(elm327->protocolRunning) && 1 < frames->size ) {
                        header = gprintf("%d:", frame_idx);
                    }
                }
                ad_buffer_free(headerBin);
                if ( elm327->can.auto_format && ! elm327->printing_of_headers ) {
                    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                        byte pci_0 = ad_buffer_extract_0(frame);
                        switch((pci_0 & 0xF0) >> 4) {
                            case Iso15765FirstFrame:
                                ad_buffer_extract_0(frame);
                                break;
                            case Iso15765ConsecutiveFrame:
                            case Iso15765SingleFrame:
                                break;
                            default:
                                log_msg(LOG_DEBUG, "TODO");
                                break;
                        }
                    }
                }
                char *elmFrameStr;
                char *elmFrameDataStr = elm_ascii_from_bin(elm327->printing_of_spaces, frame);
                asprintf(&elmFrameStr, "%s%s%s%s%s", ecuResponse == null ? "" : ecuResponse, 
                    header == null ? "" : header, (header != null && elm327->printing_of_spaces) ? " " : "", 
                    elmFrameDataStr, elm327->eol
                );
                if ( header != null ) {
                    free(header);
                }
                free(elmFrameDataStr);
                free(ecuResponse);
                ecuResponse = elmFrameStr;
            }

            ad_list_Buffer_empty(frames);
            free(frames);
        }
        ad_buffer_free(dataResponse);

        Buffer * response_header_bin = response_header(elm327,ecu,ELM327_CAN_28_BITS_DEFAULT_PRIO);
        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            assert(elm327->can.mask != null);
            assert(elm327->can.filter != null);
            assert(elm327->can.mask->size == elm327->can.filter->size);
            if ( response_header_bin->size == elm327->can.mask->size ) {
                bool filtered = false;
                for(int i = 0; i < elm327->can.mask->size; i ++) {
                    byte m = elm327->can.mask->buffer[i];
                    int cmp1 = (response_header_bin->buffer[i] & m);
                    int cmp2 = (elm327->can.filter->buffer[i] & m);
                    if ( cmp1 != cmp2 ) {
                        filtered = true;
                        break;
                    }
                }
                if ( filtered ) {
                    free(ecuResponse);
                    ecuResponse = null;
                }
            } else {
                if ( 0 < elm327->can.mask->size ) {
                    log_msg(LOG_WARNING, "A mask is set but with the wrong format for protocol - filtering will not work");
                }
            }
        }

        int sz = min(response_header_bin->size,12);
        ad_buffer_recycle(elm327->obd_buffer);
        ad_buffer_ensure_capacity(elm327->obd_buffer,sz);
        memmove(elm327->obd_buffer->buffer, response_header_bin->buffer, sz);
        elm327->obd_buffer->size = sz;
        ad_buffer_free(response_header_bin);

        if ( ecuResponse != null ) {
            char * tmpResponseResult;
            asprintf(&tmpResponseResult,"%s%s%s",response==null?"":response,ecuResponse,i+1<LIST_SIM_ECU(elm327->ecus)->size ? elm327->eol : "");
            free(ecuResponse);
            ecuResponse = null;
            free(response);
            response = null;
            response = tmpResponseResult;
        }
    }
    bus_unlock(elm327);
    ad_buffer_free(dataRequest);
    if ( strlen(response) == 0 ) {
        double part = 1;
        if ( ! elm327_protocol_is_j1939(elm327->protocolRunning) && 
                elm327->vehicle_response_timeout_adaptive ) {
            final int adaptive_response_should_be_quicker_than_default = 5;
            part = ( rand() / (1.0 * RAND_MAX) ) / adaptive_response_should_be_quicker_than_default;
        }
        useconds_t timeout_usec = elm327->vehicle_response_timeout * 1e3 * part;
        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            timeout_usec *= elm327->can.timeout_multiplier;
        }
        usleep(timeout_usec);
        asprintf(&response,"%s%s", ELM327ResponseStr[ELM327_RESPONSE_NO_DATA-ELM327_RESPONSE_OFFSET],elm327->eol);
    }
    sim_elm327_start_activity_monitor(elm327);
    return response;
}