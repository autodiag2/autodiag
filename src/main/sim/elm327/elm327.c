#include "sim/elm327/elm327.h"
#include "sim/elm327/sim.h"

#define ELM327_SIM_PP_GET(elm327,parameter) \
    (elm327->nvm.programmable_parameters_states->buffer[parameter] ? elm327->nvm.programmable_parameters->buffer[parameter] : elm327->programmable_parameters_defaults->buffer[parameter])

#define ELM327_SIM_PPS_STATE(elm327,state) \
    for(int i = 0;i < elm327->nvm.programmable_parameters_states->size; i++) { \
        elm327->nvm.programmable_parameters_states->buffer[i] = state; \
    }

void elm327_sim_go_low_power() {
    log_msg(LOG_INFO, "Device go to low power");
}

void elm327_sim_activity_monitor_daemon(ELM327emulation * elm327) {
    elm327->activity_monitor_count = 0x00;
    while(elm327->activity_monitor_count != 0xFF) {
        elm327->activity_monitor_count++;
        usleep(655e3);
        if ( elm327->activity_monitor_timeout < elm327->activity_monitor_count ) {
            byte b = ELM327_SIM_PP_GET(elm327,0x0F);
            if ( bitRetrieve(b,7) ) {
                if ( bitRetrieve(b,5) ) {
                    elm327_sim_go_low_power();
                }
                if ( bitRetrieve(b,3) ) {
                    char *act_alert;
                    asprintf(&act_alert,"%sACT ALERT", bitRetrieve(b,1) ? "!" : "");
                    log_msg(LOG_DEBUG, "Sending \"%s\"", act_alert);

                    #if defined OS_WINDOWS
                    #   warning simulation not available under windows
                    #elif defined OS_POSIX
                        if ( write(elm327->fd,act_alert,strlen(act_alert)) == -1 ) {
                            perror("write");
                        }
                    #endif
                    free(act_alert);
                }
            }
            break;
        }
    }
}

void elm327_sim_start_activity_monitor(ELM327emulation * elm327) {
    if ( elm327->activity_monitor_thread != null ) {
        pthread_cancel(elm327->activity_monitor_thread);
        elm327->activity_monitor_thread = null;
    }
    if ( pthread_create(&elm327->activity_monitor_thread, NULL,
                          (void *(*) (void *)) elm327_sim_activity_monitor_daemon,
                          (void *)elm327) != 0 ) {
        log_msg(LOG_ERROR, "thread creation error");
        exit(EXIT_FAILURE);
    }
}

char * elm327_sim_bus(ELM327emulation * elm327, char * obd_request) {
    char *response = null;
    bool isOBD = true;
    bool hasSpaces = false;
    for(int i = 0; i < strlen(obd_request); i++) {
        char c = obd_request[i];
        if ( c == ' ' ) {
            hasSpaces = true;
        }
        if ( !( c == ELM327_SIM_PP_GET(elm327,0x0D) || c == ELM327_SIM_PP_GET(elm327,0x0A) || c == ' ' || 0x30 <= c && c <= 0x39 || 0x41 <= c && c <= 0x46 ) ) {
            isOBD = false;
        }
    }
    if ( isOBD && elm327->responses ) {
        char * space = hasSpaces ? " " : "";
        char *requestStr;
        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            if ( elm327->can.auto_format ) {
                bool customHeader = false;
                if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
                    if ( elm327->custom_header->size == 2 ) {
                        asprintf(&requestStr,"%01x%02x%s%s", 
                            elm327->custom_header->buffer[0]&0xF, 
                            elm327->custom_header->buffer[1], 
                            space, obd_request);
                        customHeader = true;
                    } else if ( elm327->custom_header->size == 3 ) {
                        asprintf(&requestStr,"%01x%02x%s%s", 
                            elm327->custom_header->buffer[1]&0xF, 
                            elm327->custom_header->buffer[2], 
                            space, obd_request);
                        customHeader = true;
                    }
                } else if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {            
                    if ( elm327->custom_header->size == 3 ) {
                        asprintf(&requestStr,"%02x%s%02x%s%02x%s%02x%s%s", 
                            elm327->can.priority_29bits, space,
                            elm327->custom_header->buffer[0], space,
                            elm327->custom_header->buffer[1], space,
                            elm327->custom_header->buffer[2], space,
                            obd_request);
                        customHeader = true;
                    } else if ( elm327->custom_header->size == 4 ) {
                        asprintf(&requestStr,"%s%s", 
                            elm_ascii_from_bin(hasSpaces,elm327->custom_header),
                            obd_request);
                        customHeader = true;
                    }
                }
                if ( ! customHeader ) {
                    asprintf(&requestStr,"%s%s00%s%s",ecu_sim_generate_obd_header(elm327,elm327->testerAddress,elm327->can.priority_29bits,hasSpaces),space,space,obd_request);
                }
            } else {
                requestStr = obd_request;
            }
        } else {
            if ( elm327->custom_header->size == 3 ) {
                asprintf(&requestStr,"%s%s",elm_ascii_from_bin(hasSpaces, elm327->custom_header),obd_request);
            } else {
                asprintf(&requestStr,"%s%s%s",ecu_sim_generate_obd_header(elm327,elm327->testerAddress,ELM327_CAN_28_BITS_DEFAULT_PRIO,hasSpaces),space,obd_request);
            }
        }
        obd_request = requestStr;
        for(int i = 0; i < elm327->ecus->size; i++) {
            ECUEmulation * ecu = elm327->ecus->list[i];

            if ( ! elm327_protocol_is_j1939(elm327->protocolRunning) ) {
                if ( elm327->receive_address != null && *elm327->receive_address != ecu->address) {
                    continue;
                }
            }

            char * tmpResponse = ecu->saej1979_sim_response((_ECUEmulation *)ecu,(_ELM327emulation *)elm327,obd_request,hasSpaces);

            Buffer * response_header_bin = ecu_sim_generate_header_bin(elm327,ecu,ELM327_CAN_28_BITS_DEFAULT_PRIO);
            if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
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
                        free(tmpResponse);
                        tmpResponse = null;
                    }
                } else {
                    if ( 0 < elm327->can.mask->size ) {
                        log_msg(LOG_WARNING, "A mask is set but with the wrong format for protocol - filtering will not work");
                    }
                }
            }

            int sz = min(response_header_bin->size,12);
            buffer_recycle(elm327->obd_buffer);
            buffer_ensure_capacity(elm327->obd_buffer,sz);
            memmove(elm327->obd_buffer->buffer, response_header_bin->buffer, sz);
            elm327->obd_buffer->size = sz;

            if ( tmpResponse != null ) {
                char * tmpResponseResult;
                asprintf(&tmpResponseResult,"%s%s%s",response==null?"":response,tmpResponse,i+1<elm327->ecus->size ? elm327->eol : "");
                free(tmpResponse);
                free(response);
                response = tmpResponseResult;
            }
        }
        if ( response == null ) {
            double part = 1;
            if ( ! elm327_protocol_is_j1939(elm327->protocolRunning) && 
                    elm327->vehicle_response_timeout_adaptive ) {
                part = rand() / (1.0 * RAND_MAX);
            }
            useconds_t timeout_usec = elm327->vehicle_response_timeout * 1e3 * part;
            if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                timeout_usec *= elm327->can.timeout_multiplier;
            }
            usleep(timeout_usec);
            asprintf(&response,"%s%s", ELM327ResponseStr[ELM327_RESPONSE_NO_DATA-ELM327_RESPONSE_OFFSET],elm327->eol);
        }
        elm327_sim_start_activity_monitor(elm327);
    }
    if ( ! elm327->responses ) {
        response = strdup("");
    }
    return response;
}

/**
 * Init an emulation loading settings from nvm
 */
void elm327_sim_init_from_nvm(ELM327emulation* elm327) {
    elm327->nvm.user_memory = 0;
    elm327->nvm.programmable_parameters = buffer_new();
    elm327->nvm.programmable_parameters_states = buffer_new();
    elm327->programmable_parameters_defaults = buffer_new();    
    buffer_ensure_capacity(elm327->nvm.programmable_parameters, ELM327_SIM_PPS_SZ);
    elm327->nvm.programmable_parameters->size = ELM327_SIM_PPS_SZ;
    buffer_ensure_capacity(elm327->nvm.programmable_parameters_states, ELM327_SIM_PPS_SZ);
    elm327->nvm.programmable_parameters_states->size = ELM327_SIM_PPS_SZ;    
    buffer_ensure_capacity(elm327->programmable_parameters_defaults, ELM327_SIM_PPS_SZ);
    elm327->programmable_parameters_defaults->size = ELM327_SIM_PPS_SZ;
    // Perform an AT MA command after powerup or reset
    elm327->programmable_parameters_defaults->buffer[0x00] = 0xFF;
    // Printing of header bytes (AT H default setting)
    elm327->programmable_parameters_defaults->buffer[0x01] = 0xFF;
    // Allow long messages (AT AL default setting)
    elm327->programmable_parameters_defaults->buffer[0x02] = 0xFF;
    // NO DATA timeout time (AT ST default setting) setting = value x 4.096 msec
    elm327->programmable_parameters_defaults->buffer[0x03] = 0x32;
    // Default Adaptive Timing mode (AT AT setting)
    elm327->programmable_parameters_defaults->buffer[0x04] = 0x01;
    // OBD Source (Tester) Address. Not used for J1939 protocols.
    elm327->programmable_parameters_defaults->buffer[0x06] = 0xF1;
    // Last Protocol to try during automatic searches
    elm327->programmable_parameters_defaults->buffer[0x07] = 0x09;
    // Character echo (AT E default setting)
    elm327->programmable_parameters_defaults->buffer[0x09] = 0x00;
    // Linefeed Character
    elm327->programmable_parameters_defaults->buffer[0x0A] = 0x0A;
    // RS232 baud rate divisor when pin 6 is high (logic 1) baud rate (in kbps) = 4000 ÷ (PP 0C value)
    elm327->programmable_parameters_defaults->buffer[0x0C] = 0x68;
    // Carriage Return Character
    elm327->programmable_parameters_defaults->buffer[0x0D] = 0x0D;
    // Power Control options
    elm327->programmable_parameters_defaults->buffer[0x0E] = 0x9A;
    // Activity Monitor options
    elm327->programmable_parameters_defaults->buffer[0x0F] = 0xD5;
    // J1850 voltage settling time setting (in msec) = (PP 10 value) x 4.096
    elm327->programmable_parameters_defaults->buffer[0x10] = 0x0D;
    // J1850 Break Signal monitor enable (reports BUS ERROR if break signal duration limits are exceeded)
    elm327->programmable_parameters_defaults->buffer[0x11] = 0x00;
    // J1850 Volts (pin 3) output polarity
    elm327->programmable_parameters_defaults->buffer[0x12] = 0xFF;
    // Time delay added between protocols 1 & 2 during a search setting (in msec) = 150 + (PP 13 value) x 4.096
    elm327->programmable_parameters_defaults->buffer[0x13] = 0x55;
    // ISO/KWP final stop bit width (provides P4 interbyte time) setting (in μsec) = 98 + (PP 14 value) x 64
    elm327->programmable_parameters_defaults->buffer[0x14] = 0x50;
    // ISO/KWP maximum inter-byte time (P1), and also used for the minimum inter-message time (P2).
    elm327->programmable_parameters_defaults->buffer[0x15] = 0x0A;
    // Default ISO/KWP baud rate (AT IB default setting)
    elm327->programmable_parameters_defaults->buffer[0x16] = 0xFF;
    // ISO/KWP wakeup message rate (AT SW default setting)
    elm327->programmable_parameters_defaults->buffer[0x17] = 0x92;
    // ISO/KWP delay before a fast init, if a slow init has taken place setting (in msec) = 1000 + (PP 18 value) x 20.48
    elm327->programmable_parameters_defaults->buffer[0x18] = 0x31;
    // ISO/KWP delay before a slow init, if a fast init has taken place setting (in msec) = 1000 + (PP 19 value) x 20.48
    elm327->programmable_parameters_defaults->buffer[0x19] = 0x31;
    // Protocol 5 fast initiation active time (Tini L ) setting (in msec) = (PP 1A value) x 2.5
    elm327->programmable_parameters_defaults->buffer[0x1A] = 0x0A;
    // Protocol 5 fast initiation passive time (Tini H ) setting (in msec) = (PP 1B value) x 2.5
    elm327->programmable_parameters_defaults->buffer[0x1B] = 0x0A;
    // ISO/KWP outputs used for initiation (b7 to b2 are not used)
    elm327->programmable_parameters_defaults->buffer[0x1C] = 0x03;
    // ISO/KWP P3 time (delay before sending requests) Ave time (in msec) = (PP 1D value - 0.5) x 4.096
    elm327->programmable_parameters_defaults->buffer[0x1D] = 0x0F;
    // ISO/KWP K line minimum quiet time before an init can begin (W5) setting (in msec) = (PP 1E value) x 4.096
    elm327->programmable_parameters_defaults->buffer[0x1E] = 0x4A;
    // Default CAN Silent Monitoring setting (for AT CSM)
    elm327->programmable_parameters_defaults->buffer[0x21] = 0xFF;
    // CAN auto formatting (AT CAF default setting)
    elm327->programmable_parameters_defaults->buffer[0x24] = 0x00;
    // CAN auto flow control (AT CFC default setting)
    elm327->programmable_parameters_defaults->buffer[0x25] = 0x00;
    // CAN filler byte (used to pad out messages)
    elm327->programmable_parameters_defaults->buffer[0x26] = 0x00;
    // CAN Filter settings (controls CAN sends while searching)
    elm327->programmable_parameters_defaults->buffer[0x28] = 0xFF;
    // Printing of the CAN data length (DLC) when printing header bytes (AT D0/D1 default setting)
    elm327->programmable_parameters_defaults->buffer[0x29] = 0xFF;
    // CAN Error Checking (applies to protocols 6 to C)
    elm327->programmable_parameters_defaults->buffer[0x2A] = 0x3C;
    // Protocol A (SAE J1939) CAN baud rate divisor baud rate (in kbps) = 500 ÷ (PP 2B value)
    elm327->programmable_parameters_defaults->buffer[0x2B] = 0x02;
    // Protocol B (USER1) CAN options.
    elm327->programmable_parameters_defaults->buffer[0x2C] = 0xE0;
    // Protocol B (USER1) baud rate divisor. See PP 2B for a description.
    elm327->programmable_parameters_defaults->buffer[0x2D] = 0x04;
    // Protocol C (USER2) CAN options. See PP 2C for a description.
    elm327->programmable_parameters_defaults->buffer[0x2E] = 0x80;
    // Protocol C (USER2) baud rate divisor. See PP 2B for a description.
    elm327->programmable_parameters_defaults->buffer[0x2F] = 0x0A;
    memcpy( 
            elm327->nvm.programmable_parameters->buffer,
            elm327->programmable_parameters_defaults->buffer,
            elm327->programmable_parameters_defaults->size
    );
    ELM327_SIM_PPS_STATE(elm327,false)
    elm327->obd_buffer = buffer_new();
    buffer_ensure_capacity(elm327->obd_buffer,12);
    elm327->activity_monitor_count = 0x00;
    elm327->activity_monitor_thread = null;
    int secs = bitRetrieve(ELM327_SIM_PP_GET(elm327,0x0F), 4) ? 150 : 30;
    elm327->activity_monitor_timeout = (secs / 0.65536) - 1;
    elm327->receive_address = null;
    elm327->port_name = null;
    elm327->vehicle_response_timeout = ELM327_SIM_PP_GET(elm327,0x03) * 4;
    elm327->vehicle_response_timeout_adaptive = true;
    elm327->isMemoryEnabled = true;
    elm327->nvm.protocol = ELM327_SIM_DEFAULT_PROTO;
    elm327->nvm.protocol_is_auto = true;
    elm327_sim_non_volatile_memory_load(elm327);
    asprintf(&elm327->eol,"%c%c", ELM327_SIM_PP_GET(elm327,0x0D), ELM327_SIM_PP_GET(elm327,0x0A));
    elm327->protocolRunning = elm327->nvm.protocol;
    elm327->printing_of_spaces = true;
    elm327->dev_identifier = strdup("dev identifier");
    elm327->dev_description = strdup("dev description");    
    elm327->protocol_is_auto_running = elm327->nvm.protocol_is_auto;
    elm327->voltageFactory = (rand() / (1.0 * RAND_MAX)) * 20;
    elm327->voltage = elm327->voltageFactory;
    elm327->baud_rate = 38400;   
    elm327->responses = true; 
    elm327->printing_of_headers = ELM327_SIM_PP_GET(elm327,0x01) == 0x00;
    elm327->echo = ELM327_SIM_PP_GET(elm327,0x09) == 0x00;
    elm327->testerAddress = ELM327_SIM_PP_GET(elm327,0x06);
    elm327->can.mask = buffer_new();
    elm327->can.filter = buffer_new();
    elm327->can.priority_29bits = ELM327_CAN_28_BITS_DEFAULT_PRIO;
    elm327->can.auto_format = ELM327_SIM_PP_GET(elm327,0x24) == 0x00;
    elm327->can.extended_addressing = false;
    elm327->can.timeout_multiplier = 1;
    elm327->can.display_dlc = ELM327_SIM_PP_GET(elm327, 0x29) == 0x00;
    elm327->custom_header = buffer_new();
    elm327_sim_start_activity_monitor(elm327);
}
/**
 * One time emulation init
 */
void elm327_sim_init(ELM327emulation* elm327) {
    #ifdef OS_WINDOWS
        elm327->pipe_handle = INVALID_HANDLE_VALUE;
    #elif defined OS_POSIX
        elm327->fd = -1;
    #else
    #   warning OS unsupported
    #endif
    elm327_sim_init_from_nvm(elm327);
}

ELM327emulation* elm327_sim_new() {
    ELM327emulation* elm327 = (ELM327emulation*)malloc(sizeof(ELM327emulation));
    elm327->ecus = ECUEmulation_list_new();
    ECUEmulation_list_append(elm327->ecus,ecu_emulation_new(0xE8));
    elm327->loop_thread = null;
    elm327_sim_init_from_nvm(elm327);
    return elm327;
}
void elm327_sim_destroy(ELM327emulation * elm327) {
    if ( elm327->activity_monitor_thread != null ) {
        pthread_cancel(elm327->activity_monitor_thread);
        elm327->activity_monitor_thread = null;
    }
    if ( elm327->loop_thread != null ) {
        pthread_cancel(elm327->loop_thread);
        elm327->loop_thread = null;
    }
    free(elm327->eol);
    free(elm327->dev_description);
    free(elm327->dev_identifier);
    free(elm327->port_name);
    free(elm327->receive_address);
    buffer_free(elm327->custom_header);
    buffer_free(elm327->obd_buffer);
    buffer_free(elm327->can.mask);
    buffer_free(elm327->can.filter);
    buffer_free(elm327->nvm.programmable_parameters);
    buffer_free(elm327->nvm.programmable_parameters_states);
    buffer_free(elm327->programmable_parameters_defaults);
    free(elm327);
}
void elm327_sim_loop_start(ELM327emulation * elm327) {
    if ( elm327->loop_thread != null ) {
        pthread_cancel(elm327->loop_thread);
        elm327->loop_thread = null;
    }
    if ( pthread_create(&elm327->loop_thread, NULL,
                          (void *(*) (void *)) elm327_sim_loop, (void *)elm327) != 0 ) {
        log_msg(LOG_ERROR, "thread creation error");
        exit(EXIT_FAILURE);
    }
}

char * elm327_sim_loop_process_command(ELM327emulation * elm327, char* buffer) {
    log_msg(LOG_DEBUG, "received %d bytes: '%s'", strlen(buffer), buffer);

    int last_index;
    char* serial_response = strdup(ELMResponseStr[ELM_RESPONSE_UNKNOWN-ELMResponseOffset]);
    
    #define AT_PARSE(atpart) ((last_index = serial_at_index_end(buffer,atpart)) > -1)
    #define AT_DATA_START buffer+last_index
    #define SET_SERIAL_RESPONSE_OK() serial_response = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);

    if AT_PARSE("al") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("amc") {
        asprintf(&serial_response,"%02x", elm327->activity_monitor_count);
    } else if AT_PARSE("amt") {
        if ( sscanf(AT_DATA_START, " %02x", &elm327->activity_monitor_timeout) == 1 ) {
            SET_SERIAL_RESPONSE_OK();
        }
    } else if AT_PARSE("cea") {
        if ( sscanf(AT_DATA_START," %02hhx", &elm327->can.extended_addressing_target_address) == 1 ) {
            elm327->can.extended_addressing = true;
        } else {
            elm327->can.extended_addressing = false;
        }
        SET_SERIAL_RESPONSE_OK();   
    } else if AT_PARSE("ctm1") {
        elm327->can.timeout_multiplier = 1;
    } else if AT_PARSE("ctm5") {
        elm327->can.timeout_multiplier = 5;     
    } else if AT_PARSE("dm1") {
        if ( 0xA <= elm327->protocolRunning && elm327->protocolRunning <= 0xC ) {
            SET_SERIAL_RESPONSE_OK();
        }  
    } else if AT_PARSE("dpn") {
        asprintf(&serial_response,"%s%01x", elm327->protocol_is_auto_running ? "A" : "", elm327->protocolRunning);
    } else if AT_PARSE("dp") {
        asprintf(&serial_response,"%s%s",elm327->protocol_is_auto_running ? "Auto, " : "", elm327_protocol_to_string(elm327->protocolRunning));
    } else if AT_PARSE("d0") {
        elm327->can.display_dlc = false;
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("d1") {
        elm327->can.display_dlc = true;
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("d") {
        log_msg(LOG_INFO, "Reset to defaults");
        elm327_sim_init_from_nvm(elm327);
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("e") {
        bool echo = atoi(AT_DATA_START);
        log_msg(LOG_INFO, "Set echo %s", echo ? "on" : "off");
        elm327->echo = echo;
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("fe") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("fc") {
        SET_SERIAL_RESPONSE_OK();                         
    } else if AT_PARSE("ib 10") {
        elm327->baud_rate = 10400;
        SET_SERIAL_RESPONSE_OK();                            
    } else if AT_PARSE("ib 48") {
        elm327->baud_rate = 4800;
        SET_SERIAL_RESPONSE_OK();                            
    } else if AT_PARSE("ib 96") {
        elm327->baud_rate = 9600;
        SET_SERIAL_RESPONSE_OK(); 
    } else if AT_PARSE("ifr") {
        SET_SERIAL_RESPONSE_OK();                     
    } else if AT_PARSE("ign") {
        asprintf(&serial_response,"%s",rand()%2?"ON":"OFF");
    } else if AT_PARSE("iia") {
        byte value;
        if ( sscanf(AT_DATA_START, " %02hhX", &value) == 1 ) {
            SET_SERIAL_RESPONSE_OK();
        }
    } else if AT_PARSE("i") {
        serial_response = strdup("ELM327 v2.1");
    } else if AT_PARSE("je") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("l") {
        bool lfe = atoi(AT_DATA_START);
        log_msg(LOG_INFO, "Set linefeeds %s", lfe ? "enabled" : "disabled");
        if ( lfe ) {
                asprintf(&elm327->eol,"%c%c", ELM327_SIM_PP_GET(elm327,0x0D), ELM327_SIM_PP_GET(elm327,0x0A));
        } else {
                asprintf(&elm327->eol,"%c", ELM327_SIM_PP_GET(elm327,0x0D));
        }
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("nl") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("pb") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("sw") {
        byte value;
        if ( sscanf(AT_DATA_START, " %02hhX", &value) == 1 ) {
            SET_SERIAL_RESPONSE_OK();
        }
    } else if ( AT_PARSE("v0") || AT_PARSE("v1") ) {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("wm") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("z") {
        elm327_sim_init_from_nvm(elm327);
        serial_response = strdup("ELM327 v2.1");
    } else if AT_PARSE("rv") {
        asprintf(&serial_response,"%.2f",elm327->voltage);
    } else if AT_PARSE("rtr") {
        SET_SERIAL_RESPONSE_OK();                                
    } else if AT_PARSE("rd") {
        asprintf(&serial_response,"%02X", elm327->nvm.user_memory);
    } else if AT_PARSE("r") {
        int value;
        if ( sscanf(AT_DATA_START, "%d", &value) == 1 ) {
            elm327->responses = value;
            SET_SERIAL_RESPONSE_OK();                                            
        }   
    } else if AT_PARSE("cv 0000") {
        elm327->voltage = elm327->voltageFactory;
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("cv") {
        int value;
        if ( sscanf(AT_DATA_START," %d", &value) == 1 ) {
            elm327->voltage = value / 100.0;
            SET_SERIAL_RESPONSE_OK();                    
        }
    } else if AT_PARSE("pps") {
        char *totalRes = strdup("");
        for(int i = 0, j = 0; i < elm327->nvm.programmable_parameters->size; i++, j=(j+1)%4) {
            char * res;
            asprintf(&res,"%02x:%02x %s%s",
                    i, 
                    elm327->nvm.programmable_parameters->buffer[i],
                    elm327->nvm.programmable_parameters_states->buffer[i] ? "N" : "F",
                    j==3 ? "\n" : "    "
                );
            char *resCat;
            asprintf(&resCat,"%s%s",totalRes,res);
            free(res);
            free(totalRes);
            totalRes = resCat;
        }
        serial_response = totalRes;
    } else if AT_PARSE("pp") {
        int parameter, value;
        char state[4];
        if ( sscanf(AT_DATA_START," %02x sv %02x", &parameter,&value) == 2 ) {
            if ( parameter < elm327->nvm.programmable_parameters->size ) {                        
                elm327->nvm.programmable_parameters->buffer[parameter] = value;
                SET_SERIAL_RESPONSE_OK();                    
            }
        } else if ( sscanf(AT_DATA_START," %02x %3s", &parameter, state) == 2 ) {
            bool stateBool = strcasecmp(state,"on") == 0;
            if ( parameter == 0xFF ) {
                ELM327_SIM_PPS_STATE(elm327,stateBool)
                SET_SERIAL_RESPONSE_OK();                    
            } else {
                if ( parameter < elm327->nvm.programmable_parameters->size ) {                        
                    elm327->nvm.programmable_parameters_states->buffer[parameter] = stateBool;
                    SET_SERIAL_RESPONSE_OK();                    
                }
            }
        }
    } else if AT_PARSE("bd") {
        #ifdef EMULATE_ELM327_v1_5
            bool hasSpaces = true;
        #else
            bool hasSpaces = elm327->printing_of_spaces;
        #endif
        int sz = elm327->obd_buffer->size;
        elm327->obd_buffer->size = elm327->obd_buffer->size_allocated;
        asprintf(&serial_response,"%02x%s%s", sz, hasSpaces ? " " : "", elm_ascii_from_bin(hasSpaces,elm327->obd_buffer));
        elm327->obd_buffer->size = sz;
    } else if AT_PARSE("brd") {
        int value = 0;
        sscanf(AT_DATA_START," %02hhx", (unsigned char*)&value);
        if ( 0 < value ) {
            elm327->baud_rate = 4000000 / ( 1.0 * value);
            SET_SERIAL_RESPONSE_OK();                    
        }
    } else if AT_PARSE("bi") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("brt") {
        int value;
        sscanf(AT_DATA_START," %02hhx", (unsigned char*)&value);
        SET_SERIAL_RESPONSE_OK();                    
    } else if AT_PARSE("ar") {
        if ( elm327->receive_address != null ) free(elm327->receive_address);
        elm327->receive_address = null;
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("cp") {
        sscanf(AT_DATA_START," %02hhx",&elm327->can.priority_29bits);
        SET_SERIAL_RESPONSE_OK();        
    } else if AT_PARSE("st") {
        byte tm;
        if ( sscanf(AT_DATA_START," %02hhx",&tm) == 1 ) {
            elm327->vehicle_response_timeout = (tm == 0 ? ELM327_SIM_PP_GET(elm327,0x03) : tm) * 4;
            SET_SERIAL_RESPONSE_OK();
        }
    } else if AT_PARSE("at") {
        int value;
        if ( sscanf(AT_DATA_START,"%d",&value) == 1 ) {        
            elm327->vehicle_response_timeout_adaptive = value != 0;
            SET_SERIAL_RESPONSE_OK();
        }
    } else if AT_PARSE("ss") {
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("pc") {
        SET_SERIAL_RESPONSE_OK();            
    } else if AT_PARSE("sh") {
        char header[9];
        if ( sscanf(AT_DATA_START," %8s",header) == 1 ||
             sscanf(AT_DATA_START," %6s",header) == 1 ||
             sscanf(AT_DATA_START," %3s",header) == 1
             ) {
            if ( strlen(header) == 3 ) {
                char *tmp;
                asprintf(&tmp,"0%s", header);
                strcpy(header,tmp);
                free(tmp);
            }

            buffer_recycle(elm327->custom_header);
            elm_ascii_to_bin_internal(false, elm327->custom_header, header, header + strlen(header));
            SET_SERIAL_RESPONSE_OK();        
        }
    } else if ( AT_PARSE("sr") || AT_PARSE("ra") ) {
        byte * b = (byte*)malloc(sizeof(byte));
        if ( sscanf(AT_DATA_START," %02hhX", b) == 1 ) {
            elm327->receive_address = b;
            SET_SERIAL_RESPONSE_OK();
        } else {
            free(b);
        }
    } else if AT_PARSE("sd") {
        if ( sscanf(AT_DATA_START, " %02hhX", &elm327->nvm.user_memory) == 1 ) {
            SET_SERIAL_RESPONSE_OK();
        }
    } else if AT_PARSE("m") {
        if ( sscanf(AT_DATA_START," %d", &elm327->isMemoryEnabled) == 1 ) {
            if ( ! elm327->isMemoryEnabled ) {
                elm327_sim_non_volatile_wipe_out();
            }
            SET_SERIAL_RESPONSE_OK();                    
        }
    } else if AT_PARSE("ta") {
        sscanf(AT_DATA_START, " %02hhX", &elm327->testerAddress);
        SET_SERIAL_RESPONSE_OK();                    
    } else if AT_PARSE("sp") {
        short unsigned int p;
        bool success = true;
        if ( sscanf(AT_DATA_START, " A%01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = true;
            elm327->nvm.protocol_is_auto = elm327->protocol_is_auto_running;
        } else if ( sscanf(AT_DATA_START, " %01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = false;
            elm327->nvm.protocol_is_auto = elm327->protocol_is_auto_running;
        } else {
            success = false;
        }
        if ( success ) {
            elm327->protocolRunning = p;
            elm327->nvm.protocol = elm327->protocolRunning;
            SET_SERIAL_RESPONSE_OK();
        }
    } else if AT_PARSE("tp") {
        short unsigned int p;
        bool success = true;
        if ( sscanf(AT_DATA_START, " A%01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = true;
        } else if ( sscanf(AT_DATA_START, " %01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = false;
        } else {
            success = false;
        }
        if ( success ) {
            elm327->protocolRunning = p;
            SET_SERIAL_RESPONSE_OK();
        }
    } else if AT_PARSE("s") {
        elm327->printing_of_spaces = atoi(AT_DATA_START);
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("h") {
        elm327->printing_of_headers = atoi(AT_DATA_START);
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("@1") {
        serial_response = strdup(elm327->dev_description);
    } else if AT_PARSE("@2") {
        serial_response = strdup(elm327->dev_identifier);
    } else if AT_PARSE("@3") {
        elm327->dev_identifier = strdup(AT_DATA_START + strlen(" "));
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("ws") {
        elm327_sim_init_from_nvm(elm327);
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("lp") {
        elm327_sim_go_low_power();
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("caf") {
        elm327->can.auto_format = atoi(AT_DATA_START);
        log_msg(LOG_INFO, "Can auto format %s", elm327->can.auto_format ? "enabled" : "disabled");
        SET_SERIAL_RESPONSE_OK();
    } else if AT_PARSE("kw") {
        char number;
        if ( sscanf(AT_DATA_START,"%c",&number) == 1 ) {
            if ( number == '0' || number == '1' ) {
                SET_SERIAL_RESPONSE_OK();                                
            } else {
                Buffer * b = buffer_new_random(2);
                asprintf(&serial_response,"1:%02x 2:%02x ",b->buffer[0],b->buffer[1]);
                buffer_free(b);
            }                          
        }
    } else if AT_PARSE("cra") {
        char received_address[9];
        if ( sscanf(AT_DATA_START, " %3s", received_address) == 1 || 
             sscanf(AT_DATA_START, " %8s", received_address) == 1 ) {

            char mask[9];
            mask[strlen(received_address)] = 0;
            for(int i = 0; i < strlen(received_address); i++) {
                mask[i] = received_address[i] == 'X' ? '0' : 'F';
            }
            char *maskCmd;
            asprintf(&maskCmd,"atcm %s", mask);
            elm327_sim_loop_process_command(elm327, maskCmd);
            free(maskCmd);

            char filter[9];
            filter[strlen(received_address)] = 0;
            for(int i = 0; i < strlen(received_address); i++) {
                filter[i] = received_address[i] == 'X' ? '0' : received_address[i];
            }
            char *filterCmd;
            asprintf(&filterCmd,"atcf %s", filter);            
            elm327_sim_loop_process_command(elm327, filterCmd);
            free(filterCmd);

        } else {
            elm327->can.mask = buffer_new();
            elm327->can.filter = buffer_new();
        }        
    } else if AT_PARSE("cm") {
        char mask[9];
        if ( sscanf(AT_DATA_START, " %3s", mask) == 1 ) {
            char * tmp;
            asprintf(&tmp,"0%s", mask);
            strcpy(mask,tmp);free(tmp);
            SET_SERIAL_RESPONSE_OK();                
        } else if ( sscanf(AT_DATA_START, " %8s", mask) == 1 ) {
            SET_SERIAL_RESPONSE_OK();                    
        }
        if ( mask != null ) {
            elm327->can.mask = ascii_to_bin_buffer(mask);
        }
    } else if AT_PARSE("csm") {
        SET_SERIAL_RESPONSE_OK();                    
    } else if AT_PARSE("cfc") {
        SET_SERIAL_RESPONSE_OK();                    
    } else if AT_PARSE("cf") {
        char filter[9];
        if ( sscanf(AT_DATA_START, " %3s", filter) == 1 ) {
            char * tmp;
            asprintf(&tmp,"0%s", filter);
            strcpy(filter,tmp);free(tmp);
            SET_SERIAL_RESPONSE_OK();                
        } else if ( sscanf(AT_DATA_START, " %8s", filter) == 1 ) {
            SET_SERIAL_RESPONSE_OK();                    
        }
        if ( filter != null ) {
            elm327->can.filter = ascii_to_bin_buffer(filter);
        }                    
    } else {
        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            if AT_PARSE("cs") {
                serial_response = strdup("T:00 R:00 ");
            } else {
                serial_response = elm327_sim_bus(elm327,buffer);
            }
        } else {
            if ( 3 <= elm327->protocolRunning && elm327->protocolRunning <= 5 ) {
                if AT_PARSE("fi") {
                    SET_SERIAL_RESPONSE_OK();
                } else if AT_PARSE("si") {
                    SET_SERIAL_RESPONSE_OK();
                } else {
                    serial_response = elm327_sim_bus(elm327,buffer);                                
                }
            } else {
                serial_response = elm327_sim_bus(elm327,buffer);
            }
        }
    }
    return serial_response;
}

void elm327_sim_loop(ELM327emulation * elm327) {
    #ifdef OS_WINDOWS
        #define MAX_ATTEMPTS 20

        char pipeName[256];

        HANDLE hPipe;
        int i;
        for (i = 0; i < MAX_ATTEMPTS; i++) {
            snprintf(pipeName, sizeof(pipeName), "\\\\.\\pipe\\elm327sim_%d", i);

            // Création du pipe nommé
            hPipe = CreateNamedPipeA(
                pipeName,             // Nom du pipe
                PIPE_ACCESS_DUPLEX,    // Lecture/écriture
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // Mode byte et bloquant
                1,
                1024,                  // Taille buffer sortie
                1024,                  // Taille buffer entrée
                0,                     // Timeout par défaut
                NULL                   // Sécurité par défaut
            );

            if (hPipe != INVALID_HANDLE_VALUE) {
                log_msg(LOG_INFO, "Pipe créé: %s", pipeName);
                break;
            }

            DWORD err = GetLastError();
            if (err == ERROR_ACCESS_DENIED || err == ERROR_ALREADY_EXISTS || err == ERROR_PIPE_BUSY) {
                log_msg(LOG_INFO, "Pipe %s existe déjà, tentative suivante...", pipeName);
                continue;
            } else {
                log_msg(LOG_ERROR, "Échec de création du pipe %s: (%lu)", pipeName, err);
                break;
            }
        }
        if ( i == MAX_ATTEMPTS ) {
            log_msg(LOG_ERROR, "No valid slot found");
            return;
        }

    #elif defined OS_POSIX
        int fd = posix_openpt(O_RDWR);
        if ( fd == -1 ) {
            perror("openpt");
            return;
        }
        if ( grantpt(fd) == -1 ) {
            perror("grantpt");
            return;
        }
        if ( unlockpt(fd) == -1 ) {
            perror("unlockpt");
            return;
        }
    #else
    #   warning OS unsupported
    #endif

    elm327_sim_init(elm327);

    #ifdef OS_WINDOWS
        elm327->port_name = strdup(pipeName);
        elm327->pipe_handle = hPipe;
    #elif defined OS_POSIX
        elm327->port_name = strdup(ptsname(fd));
        elm327->fd = fd;
    #else
    #   warning OS unsupported
    #endif

    log_msg(LOG_INFO, "sim running on %s", elm327->port_name);

    #ifdef OS_WINDOWS
    #elif defined OS_POSIX
        final POLLFD fileDescriptor = {
            .fd = elm327->fd,
            .events = POLLIN
        };
    #else
    #   warning OS unsupported
    #endif
    while(true) {
        int sz =  100;
        char buffer[sz];
        #ifdef OS_WINDOWS
            if ( ! ConnectNamedPipe(elm327->pipe_handle, null) ) {
                DWORD err = GetLastError();
                if ( err == ERROR_PIPE_CONNECTED ) {
                    log_msg(LOG_DEBUG, "pipe already connected");
                } else {
                    log_msg(LOG_ERROR, "connexion au client échouée: (%lu)", GetLastError());
                    break;
                }
            }
            int bytes_readed = 0;
            if ( ReadFile(elm327->pipe_handle, buffer, sz-1, &bytes_readed, 0) ) {
                buffer[bytes_readed] = 0;
            } else {
                log_msg(LOG_ERROR, "read error : %lu ERROR_BROKEN_PIPE=%lu", GetLastError(), ERROR_BROKEN_PIPE);
                continue;
            }
        #elif defined OS_POSIX
            int res = poll(&fileDescriptor,1,SERIAL_DEFAULT_TIMEOUT);
            if ( res == -1 ) {
                log_msg(LOG_ERROR, "poll error: %s", strerror(errno));
                continue;
            }
            int rv = read(elm327->fd,buffer,sz-1);
            if ( rv == -1 ) {
                log_msg(LOG_ERROR, "read error: %s", strerror(errno));
                continue;
            }

            buffer[rv] = 0;
        #else
        #   warning OS unsupported
        #endif
        
        char * serial_response = elm327_sim_loop_process_command(elm327, buffer);

        if ( serial_response == null ) {
            serial_response = strdup(ELMResponseStr[ELM_RESPONSE_UNKNOWN-ELMResponseOffset]);                
        }

        char * response;
        asprintf(&response,"%s%s%s%s%s",
            elm327->echo ? buffer : "", elm327->echo ? elm327->eol : "", 
            serial_response,
            elm327->eol,
            SerialResponseStr[SERIAL_RESPONSE_PROMPT-SerialResponseOffset]
        );
        free(serial_response);
        log_msg(LOG_DEBUG, "make a wait before sending the response to avoid write() before read() causing response loss");
        usleep(50e3);
        log_msg(LOG_DEBUG, "sending back %s", response);

        #ifdef OS_WINDOWS
            int bytes_written = 0;
            if (!WriteFile(elm327->pipe_handle, response, strlen(response), &bytes_written, null)) {
                log_msg(LOG_ERROR, "WriteFile failed with error %lu", GetLastError());
                break;
            }
        #elif defined OS_POSIX
            if ( write(elm327->fd,response,strlen(response)) == -1 ) {
                perror("write");
                break;
            }
        #else
        #   warning OS unsupported
        #endif
        free(response);
        elm327_sim_non_volatile_memory_store(elm327);                
    }

}
