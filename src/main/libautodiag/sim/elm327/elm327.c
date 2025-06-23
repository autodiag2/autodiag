#include "libautodiag/sim/elm327/elm327.h"
#include "libautodiag/sim/elm327/sim.h"

#define SIM_ELM327_PP_GET(elm327,parameter) \
    (elm327->nvm.programmable_parameters_states->buffer[parameter] ? elm327->nvm.programmable_parameters->buffer[parameter] : elm327->programmable_parameters_defaults->buffer[parameter])

#define SIM_ELM327_PPS_STATE(elm327,state) \
    for(int i = 0;i < elm327->nvm.programmable_parameters_states->size; i++) { \
        elm327->nvm.programmable_parameters_states->buffer[i] = state; \
    }

void sim_elm327_go_low_power() {
    log_msg(LOG_INFO, "Device go to low power");
}

void sim_elm327_activity_monitor_daemon(SimELM327 * elm327) {
    elm327->activity_monitor_count = 0x00;
    while(elm327->activity_monitor_count != 0xFF) {
        elm327->activity_monitor_count++;
        usleep(655e3);
        if ( elm327->activity_monitor_timeout < elm327->activity_monitor_count ) {
            byte b = SIM_ELM327_PP_GET(elm327,0x0F);
            if ( bitRetrieve(b,7) ) {
                if ( bitRetrieve(b,5) ) {
                    sim_elm327_go_low_power();
                }
                if ( bitRetrieve(b,3) ) {
                    char *act_alert;
                    asprintf(&act_alert,"%sACT ALERT", bitRetrieve(b,1) ? "!" : "");
                    log_msg(LOG_DEBUG, "Sending \"%s\"", act_alert);

                    #if defined OS_WINDOWS
                        DWORD bytes_written = 0;
                        if (!WriteFile(elm327->implementation->pipe_handle, act_alert, strlen(act_alert), &bytes_written, null)) {
                            log_msg(LOG_ERROR, "WriteFile failed with error %lu", GetLastError());
                        }
                    #elif defined OS_POSIX
                        if ( write(elm327->implementation->fd,act_alert,strlen(act_alert)) == -1 ) {
                            perror("write");
                        }
                    #else
                    #   warning OS unsupported
                    #endif
                    free(act_alert);
                }
            }
            break;
        }
    }
}

void sim_elm327_start_activity_monitor(SimELM327 * elm327) {
    if ( elm327->implementation->activity_monitor_thread != null ) {
        pthread_cancel(*elm327->implementation->activity_monitor_thread);
    }
    elm327->implementation->activity_monitor_thread = (pthread_t*)malloc(sizeof(pthread_t));
    if ( pthread_create(elm327->implementation->activity_monitor_thread, NULL,
                          (void *(*) (void *)) sim_elm327_activity_monitor_daemon,
                          (void *)elm327) != 0 ) {
        log_msg(LOG_ERROR, "thread creation error");
        elm327->implementation->activity_monitor_thread = null;
        exit(EXIT_FAILURE);
    }
}

char * sim_elm327_bus(SimELM327 * elm327, char * obd_request) {
    char *response = null;
    bool isOBD = true;
    int space_num = 0;
    for(int i = 0; i < strlen(obd_request); i++) {
        char c = obd_request[i];
        if ( c == ' ' ) {
            space_num ++;
        }
        if ( !( c == SIM_ELM327_PP_GET(elm327,0x0D) || c == SIM_ELM327_PP_GET(elm327,0x0A) || c == ' ' || 0x30 <= c && c <= 0x39 || 0x41 <= c && c <= 0x46 ) ) {
            isOBD = false;
        }
    }
    final bool hasSpaces = 0 < space_num;
    if ( isOBD && elm327->responses ) {
        char * space = hasSpaces ? " " : "";
        char *requestStr;
        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            char *request_header = null;
            if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
                if ( elm327->custom_header->size == 2 ) {
                    asprintf(&request_header,"%01x%02x", 
                        elm327->custom_header->buffer[0]&0xF, 
                        elm327->custom_header->buffer[1]);
                } else if ( elm327->custom_header->size == 3 ) {
                    asprintf(&request_header,"%01x%02x", 
                        elm327->custom_header->buffer[1]&0xF, 
                        elm327->custom_header->buffer[2]);
                }
            } else if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {            
                if ( elm327->custom_header->size == 3 ) {
                    asprintf(&request_header,"%02x%s%02x%s%02x%s%02x", 
                        elm327->can.priority_29bits, space,
                        elm327->custom_header->buffer[0], space,
                        elm327->custom_header->buffer[1], space,
                        elm327->custom_header->buffer[2]);
                } else if ( elm327->custom_header->size == 4 ) {
                    request_header = elm_ascii_from_bin(hasSpaces,elm327->custom_header);
                }
            }
            if ( request_header == null ) {
                request_header = sim_ecu_generate_obd_header(elm327,elm327->testerAddress,elm327->can.priority_29bits,hasSpaces);
            }
            if ( elm327->can.auto_format ) {
                final int pci = strlen(obd_request) - space_num;
                asprintf(&requestStr,"%s%s%02x%s%s", request_header, space, pci, space, obd_request);
            } else {
                asprintf(&requestStr,"%s%s%s", request_header, space, obd_request);
            }
        } else {
            if ( elm327->custom_header->size == 3 ) {
                asprintf(&requestStr,"%s%s",elm_ascii_from_bin(hasSpaces, elm327->custom_header),obd_request);
            } else {
                asprintf(&requestStr,"%s%s%s",sim_ecu_generate_obd_header(elm327,elm327->testerAddress,ELM327_CAN_28_BITS_DEFAULT_PRIO,hasSpaces),space,obd_request);
            }
        }
        obd_request = requestStr;
        for(int i = 0; i < elm327->ecus->size; i++) {
            SimECU * ecu = elm327->ecus->list[i];

            if ( ! elm327_protocol_is_j1939(elm327->protocolRunning) ) {
                if ( elm327->receive_address != null && *elm327->receive_address != ecu->address) {
                    continue;
                }
            }

            char * tmpResponse = ecu->sim_ecu_response(ecu,(SimELM327 *)elm327,obd_request,hasSpaces);

            Buffer * response_header_bin = sim_ecu_generate_header_bin(elm327,ecu,ELM327_CAN_28_BITS_DEFAULT_PRIO);
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
                        tmpResponse = null; // we do not free to compat with python
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
                tmpResponse = null; // we do not free to compat with python
                response = null;    // we do not free to compat with python
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
        sim_elm327_start_activity_monitor(elm327);
    }
    if ( ! elm327->responses ) {
        response = strdup("");
    }
    return response;
}
void sim_elm327_debug(final SimELM327 * elm327) {
    printf("SimELM327: {\n");
    printf("    ecus %p (list: %p, size: %d): {\n", elm327->ecus, elm327->ecus->list, elm327->ecus->size);
    for(int i = 0; i < elm327->ecus->size; i++) {
        SimECU * sim_ecu = elm327->ecus->list[i];
        SimECUGenerator * generator = sim_ecu->generator;
        printf("        ecu: %p {\n", sim_ecu);
        printf("            address: %02hhX\n", sim_ecu->address);
        printf("            generator: %p {\n", sim_ecu->generator);
        printf("                context: %p\n", generator->context);
        printf("                type: %p %s\n", generator->type, generator->type);
        printf("                sim_ecu_generator_response: %p\n", generator->sim_ecu_generator_response);
        printf("            }\n");
        printf("            sim_ecu_response: %p\n", sim_ecu->sim_ecu_response);
        printf("        }\n");
    }
    printf("    }\n");
    printf("}\n");
}
void sim_elm327_set_baud_rate_divisor_timeout(final SimELM327 * elm327, final int timeout) {
    elm327->baud_rate_timeout_msec = timeout * 5;
    if ( timeout == 0 ) {
        elm327->baud_rate_timeout_msec = 256 * 5;
    }
}
/**
 * Init an emulation loading settings from nvm
 */
void sim_elm327_init_from_nvm(SimELM327* elm327, final SIM_ELM327_INIT_TYPE type) {
    elm327->nvm.user_memory = 0;

    if ( type == SIM_ELM327_INIT_TYPE_POWER_OFF ) {
        elm327->nvm.programmable_parameters = buffer_new();
        elm327->nvm.programmable_parameters_states = buffer_new();
        elm327->nvm.programmable_parameters_pending = buffer_new();
        elm327->programmable_parameters_pending_load_type = buffer_new();
        elm327->programmable_parameters_defaults = buffer_new();  
        buffer_ensure_capacity(elm327->programmable_parameters_defaults, SIM_ELM327_PPS_SZ);
        elm327->programmable_parameters_defaults->size = SIM_ELM327_PPS_SZ; 
        buffer_ensure_capacity(elm327->programmable_parameters_pending_load_type, SIM_ELM327_PPS_SZ);
        elm327->programmable_parameters_pending_load_type->size = SIM_ELM327_PPS_SZ;
        buffer_ensure_capacity(elm327->nvm.programmable_parameters_states, SIM_ELM327_PPS_SZ);
        elm327->nvm.programmable_parameters_states->size = SIM_ELM327_PPS_SZ;
        SIM_ELM327_PPS_STATE(elm327,false)  
        buffer_ensure_capacity(elm327->nvm.programmable_parameters, SIM_ELM327_PPS_SZ);
        elm327->nvm.programmable_parameters->size = SIM_ELM327_PPS_SZ;
        buffer_ensure_capacity(elm327->nvm.programmable_parameters_pending, SIM_ELM327_PPS_SZ);
        elm327->nvm.programmable_parameters_pending->size = SIM_ELM327_PPS_SZ;

        // Perform an AT MA command after powerup or reset
        elm327->programmable_parameters_defaults->buffer[0x00] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x00] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Printing of header bytes (AT H default setting)
        elm327->programmable_parameters_defaults->buffer[0x01] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x01] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Allow long messages (AT AL default setting)
        elm327->programmable_parameters_defaults->buffer[0x02] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x02] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // NO DATA timeout time (AT ST default setting) setting = value x 4.096 msec
        elm327->programmable_parameters_defaults->buffer[0x03] = 0x32;
        elm327->programmable_parameters_pending_load_type->buffer[0x03] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Default Adaptive Timing mode (AT AT setting)
        elm327->programmable_parameters_defaults->buffer[0x04] = 0x01;
        elm327->programmable_parameters_pending_load_type->buffer[0x04] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // OBD Source (Tester) Address. Not used for J1939 protocols.
        elm327->programmable_parameters_defaults->buffer[0x06] = 0xF1;
        elm327->programmable_parameters_pending_load_type->buffer[0x06] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Last Protocol to try during automatic searches
        elm327->programmable_parameters_defaults->buffer[0x07] = 0x09;
        elm327->programmable_parameters_pending_load_type->buffer[0x07] = SIM_ELM327_INIT_TYPE_IMMEDIATE;
        // Character echo (AT E default setting)
        elm327->programmable_parameters_defaults->buffer[0x09] = 0x00;
        elm327->programmable_parameters_pending_load_type->buffer[0x09] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Linefeed Character
        elm327->programmable_parameters_defaults->buffer[0x0A] = 0x0A;
        elm327->programmable_parameters_pending_load_type->buffer[0x0A] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // RS232 baud rate divisor when pin 6 is high (logic 1) baud rate (in kbps) = 4000 ÷ (PP 0C value)
        elm327->programmable_parameters_defaults->buffer[0x0C] = 0x68;
        elm327->programmable_parameters_pending_load_type->buffer[0x0C] = SIM_ELM327_INIT_TYPE_POWER_OFF;
        // Carriage Return Character
        elm327->programmable_parameters_defaults->buffer[0x0D] = 0x0D;
        elm327->programmable_parameters_pending_load_type->buffer[0x0D] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Power Control options
        elm327->programmable_parameters_defaults->buffer[0x0E] = 0x9A;
        elm327->programmable_parameters_pending_load_type->buffer[0x0E] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Activity Monitor options
        elm327->programmable_parameters_defaults->buffer[0x0F] = 0xD5;
        elm327->programmable_parameters_pending_load_type->buffer[0x0F] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // J1850 voltage settling time setting (in msec) = (PP 10 value) x 4.096
        elm327->programmable_parameters_defaults->buffer[0x10] = 0x0D;
        elm327->programmable_parameters_pending_load_type->buffer[0x10] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // J1850 Break Signal monitor enable (reports BUS ERROR if break signal duration limits are exceeded)
        elm327->programmable_parameters_defaults->buffer[0x11] = 0x00;
        elm327->programmable_parameters_pending_load_type->buffer[0x11] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // J1850 Volts (pin 3) output polarity
        elm327->programmable_parameters_defaults->buffer[0x12] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x12] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Time delay added between protocols 1 & 2 during a search setting (in msec) = 150 + (PP 13 value) x 4.096
        elm327->programmable_parameters_defaults->buffer[0x13] = 0x55;
        elm327->programmable_parameters_pending_load_type->buffer[0x13] = SIM_ELM327_INIT_TYPE_IMMEDIATE;
        // ISO/KWP final stop bit width (provides P4 interbyte time) setting (in μsec) = 98 + (PP 14 value) x 64
        elm327->programmable_parameters_defaults->buffer[0x14] = 0x50;
        elm327->programmable_parameters_pending_load_type->buffer[0x14] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // ISO/KWP maximum inter-byte time (P1), and also used for the minimum inter-message time (P2).
        elm327->programmable_parameters_defaults->buffer[0x15] = 0x0A;
        elm327->programmable_parameters_pending_load_type->buffer[0x15] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Default ISO/KWP baud rate (AT IB default setting)
        elm327->programmable_parameters_defaults->buffer[0x16] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x16] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // ISO/KWP wakeup message rate (AT SW default setting)
        elm327->programmable_parameters_defaults->buffer[0x17] = 0x92;
        elm327->programmable_parameters_pending_load_type->buffer[0x17] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // ISO/KWP delay before a fast init, if a slow init has taken place setting (in msec) = 1000 + (PP 18 value) x 20.48
        elm327->programmable_parameters_defaults->buffer[0x18] = 0x31;
        elm327->programmable_parameters_pending_load_type->buffer[0x18] = SIM_ELM327_INIT_TYPE_IMMEDIATE;
        // ISO/KWP delay before a slow init, if a fast init has taken place setting (in msec) = 1000 + (PP 19 value) x 20.48
        elm327->programmable_parameters_defaults->buffer[0x19] = 0x31;
        elm327->programmable_parameters_pending_load_type->buffer[0x19] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Protocol 5 fast initiation active time (Tini L ) setting (in msec) = (PP 1A value) x 2.5
        elm327->programmable_parameters_defaults->buffer[0x1A] = 0x0A;
        elm327->programmable_parameters_pending_load_type->buffer[0x1A] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Protocol 5 fast initiation passive time (Tini H ) setting (in msec) = (PP 1B value) x 2.5
        elm327->programmable_parameters_defaults->buffer[0x1B] = 0x0A;
        elm327->programmable_parameters_pending_load_type->buffer[0x1B] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // ISO/KWP outputs used for initiation (b7 to b2 are not used)
        elm327->programmable_parameters_defaults->buffer[0x1C] = 0x03;
        elm327->programmable_parameters_pending_load_type->buffer[0x1C] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // ISO/KWP P3 time (delay before sending requests) Ave time (in msec) = (PP 1D value - 0.5) x 4.096
        elm327->programmable_parameters_defaults->buffer[0x1D] = 0x0F;
        elm327->programmable_parameters_pending_load_type->buffer[0x1D] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // ISO/KWP K line minimum quiet time before an init can begin (W5) setting (in msec) = (PP 1E value) x 4.096
        elm327->programmable_parameters_defaults->buffer[0x1E] = 0x4A;
        elm327->programmable_parameters_pending_load_type->buffer[0x1E] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Default CAN Silent Monitoring setting (for AT CSM)
        elm327->programmable_parameters_defaults->buffer[0x21] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x21] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // CAN auto formatting (AT CAF default setting)
        elm327->programmable_parameters_defaults->buffer[0x24] = 0x00;
        elm327->programmable_parameters_pending_load_type->buffer[0x24] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // CAN auto flow control (AT CFC default setting)
        elm327->programmable_parameters_defaults->buffer[0x25] = 0x00;
        elm327->programmable_parameters_pending_load_type->buffer[0x25] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // CAN filler byte (used to pad out messages)
        elm327->programmable_parameters_defaults->buffer[0x26] = 0x00;
        elm327->programmable_parameters_pending_load_type->buffer[0x26] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // CAN Filter settings (controls CAN sends while searching)
        elm327->programmable_parameters_defaults->buffer[0x28] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x28] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Printing of the CAN data length (DLC) when printing header bytes (AT D0/D1 default setting)
        elm327->programmable_parameters_defaults->buffer[0x29] = 0xFF;
        elm327->programmable_parameters_pending_load_type->buffer[0x29] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // CAN Error Checking (applies to protocols 6 to C)
        elm327->programmable_parameters_defaults->buffer[0x2A] = 0x3C;
        elm327->programmable_parameters_pending_load_type->buffer[0x2A] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_DEFAULTS;
        // Protocol A (SAE J1939) CAN baud rate divisor baud rate (in kbps) = 500 ÷ (PP 2B value)
        elm327->programmable_parameters_defaults->buffer[0x2B] = 0x02;
        elm327->programmable_parameters_pending_load_type->buffer[0x2B] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Protocol B (USER1) CAN options.
        elm327->programmable_parameters_defaults->buffer[0x2C] = 0xE0;
        elm327->programmable_parameters_pending_load_type->buffer[0x2C] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Protocol B (USER1) baud rate divisor. See PP 2B for a description.
        elm327->programmable_parameters_defaults->buffer[0x2D] = 0x04;
        elm327->programmable_parameters_pending_load_type->buffer[0x2D] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Protocol C (USER2) CAN options. See PP 2C for a description.
        elm327->programmable_parameters_defaults->buffer[0x2E] = 0x80;
        elm327->programmable_parameters_pending_load_type->buffer[0x2E] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        // Protocol C (USER2) baud rate divisor. See PP 2B for a description.
        elm327->programmable_parameters_defaults->buffer[0x2F] = 0x0A;
        elm327->programmable_parameters_pending_load_type->buffer[0x2F] = SIM_ELM327_INIT_TYPE_POWER_OFF | SIM_ELM327_INIT_TYPE_RESET;
        
        memcpy( 
                elm327->nvm.programmable_parameters->buffer,
                elm327->programmable_parameters_defaults->buffer,
                elm327->programmable_parameters_defaults->size
        );
        memcpy( 
                elm327->nvm.programmable_parameters_pending->buffer,
                elm327->programmable_parameters_defaults->buffer,
                elm327->programmable_parameters_defaults->size
        );
    }
    elm327->obd_buffer = buffer_new();
    buffer_ensure_capacity(elm327->obd_buffer,12);
    elm327->activity_monitor_count = 0x00;
    elm327->implementation->activity_monitor_thread = null;
    int secs = bitRetrieve(SIM_ELM327_PP_GET(elm327,0x0F), 4) ? 150 : 30;
    elm327->activity_monitor_timeout = (secs / 0.65536) - 1;
    elm327->receive_address = null;
    elm327->device_location = null;
    elm327->vehicle_response_timeout = SIM_ELM327_PP_GET(elm327,0x03) * 4;
    elm327->vehicle_response_timeout_adaptive = true;
    elm327->isMemoryEnabled = true;
    elm327->nvm.protocol = SIM_ELM327_DEFAULT_PROTO;
    elm327->nvm.protocol_is_auto = true;
    sim_elm327_non_volatile_memory_load(elm327, type);
    asprintf(&elm327->eol,"%c%c", SIM_ELM327_PP_GET(elm327,0x0D), SIM_ELM327_PP_GET(elm327,0x0A));
    elm327->protocolRunning = elm327->nvm.protocol;
    elm327->printing_of_spaces = true;
    elm327->dev_identifier = strdup("dev identifier");
    elm327->dev_description = strdup("dev description");    
    elm327->protocol_is_auto_running = elm327->nvm.protocol_is_auto;
    elm327->voltageFactory = (rand() / (1.0 * RAND_MAX)) * 20;
    elm327->voltage = elm327->voltageFactory;
    elm327->baud_rate = (4000.0 / SIM_ELM327_PP_GET(elm327,0x0C)) * 1000;
    sim_elm327_set_baud_rate_divisor_timeout(elm327, SIM_ELM327_PP_GET(elm327,0x0C));
    elm327->responses = true; 
    elm327->printing_of_headers = SIM_ELM327_PP_GET(elm327,0x01) == 0x00;
    elm327->echo = SIM_ELM327_PP_GET(elm327,0x09) == 0x00;
    elm327->testerAddress = SIM_ELM327_PP_GET(elm327,0x06);
    elm327->can.mask = buffer_new();
    elm327->can.filter = buffer_new();
    elm327->can.priority_29bits = ELM327_CAN_28_BITS_DEFAULT_PRIO;
    elm327->can.auto_format = SIM_ELM327_PP_GET(elm327,0x24) == 0x00;
    elm327->can.extended_addressing = false;
    elm327->can.timeout_multiplier = 1;
    elm327->can.display_dlc = SIM_ELM327_PP_GET(elm327, 0x29) == 0x00;
    elm327->custom_header = buffer_new();
    sim_elm327_start_activity_monitor(elm327);
}

SimELM327* sim_elm327_new() {
    final SimELM327* elm327 = (SimELM327*)malloc(sizeof(SimELM327));
    elm327->implementation = (SimELM327Implementation*)malloc(sizeof(SimELM327Implementation));
    elm327->ecus = list_SimECU_new();
    final SimECU *ecu = sim_ecu_emulation_new(0xE8);
    list_SimECU_append(elm327->ecus,ecu);
    elm327->implementation->loop_thread = null;
    elm327->implementation->loop_ready = false;
    sim_elm327_init_from_nvm(elm327, SIM_ELM327_INIT_TYPE_POWER_OFF);
    return elm327;
}
void sim_elm327_destroy(SimELM327 * elm327) {
    if ( elm327->implementation->activity_monitor_thread != null ) {
        pthread_cancel(*elm327->implementation->activity_monitor_thread);
        elm327->implementation->activity_monitor_thread = null;
    }
    if ( elm327->implementation->loop_thread != null ) {
        pthread_cancel(*elm327->implementation->loop_thread);
        elm327->implementation->loop_thread = null;
    }
    elm327->implementation->loop_ready = false;
    free(elm327->implementation);
    free(elm327->eol);
    free(elm327->dev_description);
    free(elm327->dev_identifier);
    free(elm327->device_location);
    free(elm327->receive_address);
    buffer_free(elm327->custom_header);
    buffer_free(elm327->obd_buffer);
    buffer_free(elm327->can.mask);
    buffer_free(elm327->can.filter);
    buffer_free(elm327->nvm.programmable_parameters);
    buffer_free(elm327->nvm.programmable_parameters_pending);
    buffer_free(elm327->nvm.programmable_parameters_states);
    buffer_free(elm327->programmable_parameters_defaults);
    free(elm327);
}
void sim_elm327_loop_as_daemon(SimELM327 * elm327) {
    if ( elm327->implementation->loop_thread != null ) {
        pthread_cancel(*elm327->implementation->loop_thread);
    }
    elm327->implementation->loop_thread = (pthread_t*)malloc(sizeof(pthread_t));
    if ( pthread_create(elm327->implementation->loop_thread, NULL,
                          (void *(*) (void *)) sim_elm327_loop, (void *)elm327) != 0 ) {
        log_msg(LOG_ERROR, "thread creation error");
        elm327->implementation->loop_thread = null;
        exit(EXIT_FAILURE);
    }
}
bool sim_elm327_loop_daemon_wait_ready(SimELM327 * elm327) {
    final int timeout_ms = 10000;
    final int step_ms = 20;
    final int step_n = timeout_ms / step_ms;
    int i = 0;
    for(; i < step_n && elm327->implementation->loop_ready == false; i++) {
        usleep(step_ms * 1e3);
    }
    if ( i == step_n ) {
        log_msg(LOG_ERROR, "timeout while waiting for sim to be ready");
        return false;
    } else {
        usleep(step_ms * 1e3);
        return true;
    }
}

bool sim_elm327_receive(SimELM327 * elm327, final Buffer * buffer, int timeout) {
    #ifdef OS_WINDOWS
        if ( ! ConnectNamedPipe(elm327->implementation->pipe_handle, null) ) {
            DWORD err = GetLastError();
            if ( err == ERROR_PIPE_CONNECTED ) {
                log_msg(LOG_DEBUG, "pipe already connected");
            } else {
                if ( err == ERROR_NO_DATA ) {
                    log_msg(LOG_ERROR, "pipe closed");
                } else {
                    log_msg(LOG_ERROR, "connexion au client échouée: (%lu)", GetLastError());
                }
                return false;
            }
        }
        if ( file_pool(&elm327->implementation->pipe_handle, null, SERIAL_DEFAULT_TIMEOUT) == -1 ) {
            log_msg(LOG_ERROR, "Error while pooling");
        }
        DWORD readedBytes = 0;
        final int success = ReadFile(elm327->implementation->pipe_handle, buffer->buffer, buffer->size_allocated-1, &readedBytes, 0);
        if ( ! success ) {
            log_msg(LOG_ERROR, "read error : %lu ERROR_BROKEN_PIPE=%lu", GetLastError(), ERROR_BROKEN_PIPE);
            return false;
        }
        if ( UINT_MAX < readedBytes ) {
            log_msg(LOG_ERROR, "Impossible has happend more bytes received than buffer->size=%u", buffer->size);
            return false;
        }
        buffer->size = readedBytes;
    #elif defined OS_POSIX
        int res = file_pool(&elm327->implementation->fd, null, SERIAL_DEFAULT_TIMEOUT);
        if ( res == -1 ) {
            log_msg(LOG_ERROR, "poll error: %s", strerror(errno));
            return false;
        }
        int rv = read(elm327->implementation->fd,buffer->buffer,buffer->size_allocated-1);
        if ( rv == -1 ) {
            log_msg(LOG_ERROR, "read error: %s", strerror(errno));
            return false;
        }
        buffer->size = rv;
    #else
    #   warning OS unsupported
    #endif
    buffer_ensure_termination(buffer);
    return true;
}

bool sim_elm327_reply(SimELM327 * elm327, char * buffer, char * serial_response, final bool isGeneric) {
    char * response;
    if ( isGeneric ) {
        asprintf(&response,"%s%s%s%s%s",
            elm327->echo ? buffer : "", elm327->echo ? elm327->eol : "", 
            serial_response,
            elm327->eol,
            SerialResponseStr[SERIAL_RESPONSE_PROMPT-SerialResponseOffset]
        );
        log_msg(LOG_DEBUG, "make a wait before sending the response to avoid write() before read() causing response loss");
        usleep(50e3);
    } else {
        asprintf(&response,"%s%s", serial_response, elm327->eol);
    }
    free(serial_response);
    log_msg(LOG_DEBUG, "sending back %s", response);

    #ifdef OS_WINDOWS
        DWORD bytes_written = 0;
        if (!WriteFile(elm327->implementation->pipe_handle, response, strlen(response), &bytes_written, null)) {
            log_msg(LOG_ERROR, "WriteFile failed with error %lu", GetLastError());
            return false;
        }
    #elif defined OS_POSIX
        if ( write(elm327->implementation->fd,response,strlen(response)) == -1 ) {
            perror("write");
            return false;
        }
    #else
    #   warning OS unsupported
    #endif
    free(response);
    return true;
}
char *lastBinCommand = null;
bool sim_elm327_command_and_protocol_interpreter(SimELM327 * elm327, char* buffer, bool preventWrite) {
    log_msg(LOG_DEBUG, "interpreting '%s' (len: %d)", buffer, strlen(buffer));

    char * command_reduced = serial_at_reduce(buffer);
    int last_index;
    bool commandReconized = false;
    
    #define AT_PARSE(atpart) (command_reduced != null && strcasebeginwith(command_reduced, atpart) && ((last_index = strlen(atpart)) >= 0))
    #define AT_DATA_START command_reduced+last_index
    #define SIM_ELM327_REPLY(isGeneric, ...) \
        if ( ! preventWrite ) { \
            char *serial_response; \
            asprintf(&serial_response, __VA_ARGS__); \
            if ( ! sim_elm327_reply(elm327, buffer, serial_response, isGeneric) ) { \
                exit(1); \
            } \
        } \
        commandReconized = true;
    #define SIM_ELM327_REPLY_GENERIC(...) \
        SIM_ELM327_REPLY(true, __VA_ARGS__)
    #define SIM_ELM327_REPLY_OK() \
        SIM_ELM327_REPLY_GENERIC("%s", SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
    #define SIM_ELM327_ATI "ELM327 v2.1"
    #define SIM_ELM327_REPLY_ATI() \
        SIM_ELM327_REPLY_GENERIC(SIM_ELM327_ATI);

    if ( lastBinCommand != null && strlen(buffer) == 1 && buffer[0] == '\r' ) {
        buffer = lastBinCommand;
    }
    if AT_PARSE("al") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("amc") {
        SIM_ELM327_REPLY_GENERIC("%02x", elm327->activity_monitor_count);
    } else if AT_PARSE("amt") {
        if ( sscanf(AT_DATA_START, "%02x", &elm327->activity_monitor_timeout) == 1 ) {
            SIM_ELM327_REPLY_OK();
        }
    } else if AT_PARSE("cea") {
        if ( sscanf(AT_DATA_START,"%02hhx", &elm327->can.extended_addressing_target_address) == 1 ) {
            elm327->can.extended_addressing = true;
        } else {
            elm327->can.extended_addressing = false;
        }
        SIM_ELM327_REPLY_OK();   
    } else if AT_PARSE("ctm1") {
        elm327->can.timeout_multiplier = 1;
    } else if AT_PARSE("ctm5") {
        elm327->can.timeout_multiplier = 5;     
    } else if AT_PARSE("dm1") {
        if ( 0xA <= elm327->protocolRunning && elm327->protocolRunning <= 0xC ) {
            SIM_ELM327_REPLY_OK();
        }  
    } else if AT_PARSE("dpn") {
        SIM_ELM327_REPLY_GENERIC("%s%01x", elm327->protocol_is_auto_running ? "A" : "", elm327->protocolRunning);
    } else if AT_PARSE("dp") {
        SIM_ELM327_REPLY_GENERIC("%s%s",elm327->protocol_is_auto_running ? "Auto, " : "", elm327_protocol_to_string(elm327->protocolRunning));
    } else if AT_PARSE("d0") {
        elm327->can.display_dlc = false;
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("d1") {
        elm327->can.display_dlc = true;
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("d") {
        log_msg(LOG_INFO, "Reset to defaults");
        sim_elm327_init_from_nvm(elm327, SIM_ELM327_INIT_TYPE_DEFAULTS);
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("e") {
        bool echo = atoi(AT_DATA_START);
        log_msg(LOG_INFO, "Set echo %s", echo ? "on" : "off");
        elm327->echo = echo;
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("fe") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("fc") {
        SIM_ELM327_REPLY_OK();                         
    } else if AT_PARSE("ib10") {
        elm327->baud_rate = 10400;
        SIM_ELM327_REPLY_OK();                            
    } else if AT_PARSE("ib48") {
        elm327->baud_rate = 4800;
        SIM_ELM327_REPLY_OK();                            
    } else if AT_PARSE("ib96") {
        elm327->baud_rate = 9600;
        SIM_ELM327_REPLY_OK(); 
    } else if AT_PARSE("ifr") {
        SIM_ELM327_REPLY_OK();                     
    } else if AT_PARSE("ign") {
        SIM_ELM327_REPLY_GENERIC(rand()%2?"ON":"OFF");
    } else if AT_PARSE("iia") {
        byte value;
        if ( sscanf(AT_DATA_START, "%02hhX", &value) == 1 ) {
            SIM_ELM327_REPLY_OK();
        }
    } else if AT_PARSE("i") {
        SIM_ELM327_REPLY_ATI();
    } else if AT_PARSE("je") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("l") {
        bool lfe = atoi(AT_DATA_START);
        log_msg(LOG_INFO, "Set linefeeds %s", lfe ? "enabled" : "disabled");
        if ( lfe ) {
                asprintf(&elm327->eol,"%c%c", SIM_ELM327_PP_GET(elm327,0x0D), SIM_ELM327_PP_GET(elm327,0x0A));
        } else {
                asprintf(&elm327->eol,"%c", SIM_ELM327_PP_GET(elm327,0x0D));
        }
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("nl") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("pb") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("sw") {
        byte value;
        if ( sscanf(AT_DATA_START, "%02hhX", &value) == 1 ) {
            SIM_ELM327_REPLY_OK();
        }
    } else if ( AT_PARSE("v0") || AT_PARSE("v1") ) {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("wm") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("z") {
        sim_elm327_init_from_nvm(elm327, SIM_ELM327_INIT_TYPE_POWER_OFF);
        SIM_ELM327_REPLY_ATI();
    } else if AT_PARSE("rv") {
        SIM_ELM327_REPLY_GENERIC("%.2f",elm327->voltage);
    } else if AT_PARSE("rtr") {
        SIM_ELM327_REPLY_OK();                                
    } else if AT_PARSE("rd") {
        SIM_ELM327_REPLY_GENERIC("%02X", elm327->nvm.user_memory);
    } else if AT_PARSE("r") {
        int value;
        if ( sscanf(AT_DATA_START, "%d", &value) == 1 ) {
            elm327->responses = value;
            SIM_ELM327_REPLY_OK();                                            
        }   
    } else if AT_PARSE("cv0000") {
        elm327->voltage = elm327->voltageFactory;
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("cv") {
        int value;
        if ( sscanf(AT_DATA_START,"%d", &value) == 1 ) {
            elm327->voltage = value / 100.0;
            SIM_ELM327_REPLY_OK();                    
        }
    } else if AT_PARSE("pps") {
        char *totalRes = strdup("");
        for(int i = 0, j = 0; i < elm327->nvm.programmable_parameters_pending->size; i++, j=(j+1)%4) {
            char * res;
            asprintf(&res,"%02X:%02X %s%s",
                    i, 
                    elm327->nvm.programmable_parameters_pending->buffer[i],
                    elm327->nvm.programmable_parameters_states->buffer[i] ? "N" : "F",
                    j==3 ? "\n" : "    "
                );
            char *resCat;
            asprintf(&resCat,"%s%s",totalRes,res);
            free(res);
            free(totalRes);
            totalRes = resCat;
        }
        SIM_ELM327_REPLY_GENERIC("%s",totalRes);
        free(totalRes);
    } else if AT_PARSE("pp") {
        int parameter, value;
        char state[4];
        if ( sscanf(AT_DATA_START,"%02xsv%02x", &parameter,&value) == 2 ) {
            if ( parameter < elm327->nvm.programmable_parameters_pending->size ) {                        
                elm327->nvm.programmable_parameters_pending->buffer[parameter] = value;
                if ( elm327->programmable_parameters_pending_load_type->buffer[parameter] == SIM_ELM327_INIT_TYPE_IMMEDIATE ) {
                    elm327->nvm.programmable_parameters->buffer[parameter] = value;
                }
                SIM_ELM327_REPLY_OK();                    
            }
        } else if ( sscanf(AT_DATA_START,"%02x%3s", &parameter, state) == 2 ) {
            bool stateBool = strcasecmp(state,"on") == 0;
            if ( parameter == 0xFF ) {
                SIM_ELM327_PPS_STATE(elm327,stateBool)
                SIM_ELM327_REPLY_OK();                    
            } else {
                if ( parameter < elm327->nvm.programmable_parameters_states->size ) {                        
                    elm327->nvm.programmable_parameters_states->buffer[parameter] = stateBool;
                    SIM_ELM327_REPLY_OK();                    
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
        SIM_ELM327_REPLY_GENERIC("%02x%s%s", sz, hasSpaces ? " " : "", elm_ascii_from_bin(hasSpaces,elm327->obd_buffer));
        elm327->obd_buffer->size = sz;
    } else if AT_PARSE("brd") {
        int baud_rate_divisor = 0;
        sscanf(AT_DATA_START,"%02hhx", (unsigned char*)&baud_rate_divisor);
        if ( 0 < baud_rate_divisor ) {
            final int previous_baud_rate = elm327->baud_rate;
            elm327->baud_rate = (4000.0 / baud_rate_divisor) * 1000;
            SIM_ELM327_REPLY(false, "%s", SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
            usleep(elm327->baud_rate_timeout_msec * 1000);
            SIM_ELM327_REPLY(false, "%s", SIM_ELM327_ATI);
            final Buffer * recv = buffer_new();
            buffer_ensure_capacity(recv, 50);
            if ( sim_elm327_receive(elm327, recv, elm327->baud_rate_timeout_msec) ) {
                if ( recv->buffer[0] == '\r' ) {
                    SIM_ELM327_REPLY(false, "%s", SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
                } else {
                    elm327->baud_rate = previous_baud_rate;
                }
                SIM_ELM327_REPLY_GENERIC("");
            }
        }
    } else if AT_PARSE("bi") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("brt") {
        int value;
        sscanf(AT_DATA_START,"%02hhx", (unsigned char*)&value);
        sim_elm327_set_baud_rate_divisor_timeout(elm327, value);
        SIM_ELM327_REPLY_OK();                    
    } else if AT_PARSE("ar") {
        if ( elm327->receive_address != null ) free(elm327->receive_address);
        elm327->receive_address = null;
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("cp") {
        sscanf(AT_DATA_START,"%02hhx",&elm327->can.priority_29bits);
        SIM_ELM327_REPLY_OK();        
    } else if AT_PARSE("st") {
        byte tm;
        if ( sscanf(AT_DATA_START,"%02hhx",&tm) == 1 ) {
            elm327->vehicle_response_timeout = (tm == 0 ? SIM_ELM327_PP_GET(elm327,0x03) : tm) * 4;
            SIM_ELM327_REPLY_OK();
        }
    } else if AT_PARSE("at") {
        int value;
        if ( sscanf(AT_DATA_START,"%d",&value) == 1 ) {        
            elm327->vehicle_response_timeout_adaptive = value != 0;
            SIM_ELM327_REPLY_OK();
        }
    } else if AT_PARSE("ss") {
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("pc") {
        SIM_ELM327_REPLY_OK();            
    } else if AT_PARSE("sh") {
        char header[9];
        if ( sscanf(AT_DATA_START,"%8s",header) == 1 ||
             sscanf(AT_DATA_START,"%6s",header) == 1 ||
             sscanf(AT_DATA_START,"%3s",header) == 1
             ) {
            if ( strlen(header) == 3 ) {
                char *tmp;
                asprintf(&tmp,"0%s", header);
                strcpy(header,tmp);
                free(tmp);
            }

            buffer_recycle(elm327->custom_header);
            elm_ascii_to_bin_internal(false, elm327->custom_header, header, header + strlen(header));
            SIM_ELM327_REPLY_OK();        
        }
    } else if ( AT_PARSE("sr") || AT_PARSE("ra") ) {
        byte * b = (byte*)malloc(sizeof(byte));
        if ( sscanf(AT_DATA_START,"%02hhX", b) == 1 ) {
            elm327->receive_address = b;
            SIM_ELM327_REPLY_OK();
        } else {
            free(b);
        }
    } else if AT_PARSE("sd") {
        if ( sscanf(AT_DATA_START, "%02hhX", &elm327->nvm.user_memory) == 1 ) {
            SIM_ELM327_REPLY_OK();
        }
    } else if AT_PARSE("m") {
        if ( sscanf(AT_DATA_START,"%d", &elm327->isMemoryEnabled) == 1 ) {
            if ( ! elm327->isMemoryEnabled ) {
                sim_elm327_non_volatile_wipe_out();
            }
            SIM_ELM327_REPLY_OK();                    
        }
    } else if AT_PARSE("ta") {
        sscanf(AT_DATA_START, "%02hhX", &elm327->testerAddress);
        SIM_ELM327_REPLY_OK();                    
    } else if AT_PARSE("sp") {
        short unsigned int p;
        bool success = true;
        if ( sscanf(AT_DATA_START, "a%01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = true;
            elm327->nvm.protocol_is_auto = elm327->protocol_is_auto_running;
        } else if ( sscanf(AT_DATA_START, "%01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = false;
            elm327->nvm.protocol_is_auto = elm327->protocol_is_auto_running;
        } else {
            success = false;
        }
        if ( success ) {
            elm327->protocolRunning = p;
            elm327->nvm.protocol = elm327->protocolRunning;
            SIM_ELM327_REPLY_OK();
        }
    } else if AT_PARSE("tp") {
        short unsigned int p;
        bool success = true;
        if ( sscanf(AT_DATA_START, "a%01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = true;
        } else if ( sscanf(AT_DATA_START, "%01hX", &p) == 1 ) {
            elm327->protocol_is_auto_running = false;
        } else {
            success = false;
        }
        if ( success ) {
            elm327->protocolRunning = p;
            SIM_ELM327_REPLY_OK();
        }
    } else if AT_PARSE("s") {
        elm327->printing_of_spaces = atoi(AT_DATA_START);
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("h") {
        elm327->printing_of_headers = atoi(AT_DATA_START);
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("@1") {
        SIM_ELM327_REPLY_GENERIC("%s", elm327->dev_description);
    } else if AT_PARSE("@2") {
        SIM_ELM327_REPLY_GENERIC("%s", elm327->dev_identifier);
    } else if AT_PARSE("@3") {
        elm327->dev_identifier = strdup(AT_DATA_START + strlen(" "));
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("ws") {
        sim_elm327_init_from_nvm(elm327, SIM_ELM327_INIT_TYPE_RESET);
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("lp") {
        sim_elm327_go_low_power();
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("caf") {
        elm327->can.auto_format = atoi(AT_DATA_START);
        log_msg(LOG_INFO, "Can auto format %s", elm327->can.auto_format ? "enabled" : "disabled");
        SIM_ELM327_REPLY_OK();
    } else if AT_PARSE("kw") {
        char number;
        if ( sscanf(AT_DATA_START,"%c",&number) == 1 ) {
            if ( number == '0' || number == '1' ) {
                SIM_ELM327_REPLY_OK();                                
            } else {
                Buffer * b = buffer_new_random(2);
                SIM_ELM327_REPLY_GENERIC("1:%02x 2:%02x",b->buffer[0],b->buffer[1]);
                buffer_free(b);
            }                          
        }
    } else if AT_PARSE("cra") {
        char received_address[9];
        if ( sscanf(AT_DATA_START, "%3s", received_address) == 1 || 
             sscanf(AT_DATA_START, "%8s", received_address) == 1 ) {

            char mask[9];
            mask[strlen(received_address)] = 0;
            for(int i = 0; i < strlen(received_address); i++) {
                mask[i] = received_address[i] == 'x' ? '0' : 'F';
            }
            char *maskCmd;
            asprintf(&maskCmd,"atcm %s", mask);
            sim_elm327_command_and_protocol_interpreter(elm327, maskCmd, true);
            free(maskCmd);

            char filter[9];
            filter[strlen(received_address)] = 0;
            for(int i = 0; i < strlen(received_address); i++) {
                filter[i] = received_address[i] == 'x' ? '0' : received_address[i];
            }
            char *filterCmd;
            asprintf(&filterCmd,"atcf %s", filter);            
            sim_elm327_command_and_protocol_interpreter(elm327, filterCmd, true);
            free(filterCmd);

        } else {
            elm327->can.mask = buffer_new();
            elm327->can.filter = buffer_new();
        }        
    } else if AT_PARSE("cm") {
        char mask[9];
        bool parsed = false;
        if ( sscanf(AT_DATA_START, "%3s", mask) == 1 ) {
            char * tmp;
            asprintf(&tmp,"0%s", mask);
            strcpy(mask,tmp);
            free(tmp);
            SIM_ELM327_REPLY_OK();  
            parsed = true;              
        } else if ( sscanf(AT_DATA_START, "%8s", mask) == 1 ) {
            SIM_ELM327_REPLY_OK();   
            parsed = true;                 
        }
        if ( parsed ) {
            elm327->can.mask = buffer_from_ascii_hex(mask);
        }
    } else if AT_PARSE("csm") {
        SIM_ELM327_REPLY_OK();                    
    } else if AT_PARSE("cfc") {
        SIM_ELM327_REPLY_OK();                    
    } else if AT_PARSE("cf") {
        char filter[9];
        bool parsed = false;
        if ( sscanf(AT_DATA_START, "%3s", filter) == 1 ) {
            char * tmp;
            asprintf(&tmp,"0%s", filter);
            strcpy(filter,tmp);free(tmp);
            SIM_ELM327_REPLY_OK();       
            parsed = true;         
        } else if ( sscanf(AT_DATA_START, "%8s", filter) == 1 ) {
            SIM_ELM327_REPLY_OK();    
            parsed = true;                
        }
        if ( parsed) {
            elm327->can.filter = buffer_from_ascii_hex(filter);
        }                    
    } else {
        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
            if AT_PARSE("cs") {
                SIM_ELM327_REPLY_GENERIC("T:00 R:00 ")
            } else {
                char * response = sim_elm327_bus(elm327,buffer);
                if ( response != null ) {
                    lastBinCommand = strdup(buffer);
                    SIM_ELM327_REPLY_GENERIC("%s", response);
                }
            }
        } else {
            if ( 3 <= elm327->protocolRunning && elm327->protocolRunning <= 5 ) {
                if AT_PARSE("fi") {
                    SIM_ELM327_REPLY_OK();
                } else if AT_PARSE("si") {
                    SIM_ELM327_REPLY_OK();
                } else {
                    char * response = sim_elm327_bus(elm327,buffer);
                    if ( response != null ) {
                        lastBinCommand = strdup(buffer);
                        SIM_ELM327_REPLY_GENERIC("%s", response);
                    }                
                }
            } else {
                char * response = sim_elm327_bus(elm327,buffer);
                if ( response != null ) {
                    lastBinCommand = strdup(buffer);
                    SIM_ELM327_REPLY_GENERIC("%s", response);
                }
            }
        }
    }
    return commandReconized;
}

void sim_elm327_loop(SimELM327 * elm327) {
    #ifdef OS_WINDOWS
        #define MAX_ATTEMPTS 20

        char pipeName[256];

        HANDLE hPipe;
        int i;
        for (i = 0; i < MAX_ATTEMPTS; i++) {
            snprintf(pipeName, sizeof(pipeName), "\\\\.\\pipe\\" SERIAL_LIST_PIPE_PREFIX "%d", i);

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

    #ifdef OS_WINDOWS
        elm327->implementation->pipe_handle = INVALID_HANDLE_VALUE;
    #elif defined OS_POSIX
        elm327->implementation->fd = -1;
    #else
    #   warning OS unsupported
    #endif
    sim_elm327_init_from_nvm(elm327, SIM_ELM327_INIT_TYPE_POWER_OFF);

    #ifdef OS_WINDOWS
        elm327->device_location = strdup(pipeName);
        elm327->implementation->pipe_handle = hPipe;
    #elif defined OS_POSIX
        elm327->device_location = strdup(ptsname(fd));
        elm327->implementation->fd = fd;
    #else
    #   warning OS unsupported
    #endif

    log_msg(LOG_INFO, "sim running on %s", elm327->device_location);

    #ifdef OS_WINDOWS
    #elif defined OS_POSIX
        final POLLFD fileDescriptor = {
            .fd = elm327->implementation->fd,
            .events = POLLIN
        };
    #else
    #   warning OS unsupported
    #endif
    final Buffer * recv_buffer = buffer_new();
    buffer_ensure_capacity(recv_buffer, 100);
    while(true) {
        buffer_recycle(recv_buffer);
        if ( elm327->implementation->loop_ready == false ) {
            elm327->implementation->loop_ready = true;
        }
        if ( ! sim_elm327_receive(elm327, recv_buffer, SERIAL_DEFAULT_TIMEOUT) ) {
            log_msg(LOG_ERROR, "Error during reception, exiting the loop");
            return;
        }

        if ( recv_buffer->size <= 1 ) {
            continue;
        }
        log_msg(LOG_DEBUG, "Received '%s' (len: %d)", recv_buffer->buffer, recv_buffer->size);

        if ( ! sim_elm327_command_and_protocol_interpreter(elm327, recv_buffer->buffer, false) ) {
            if ( ! sim_elm327_reply(elm327, recv_buffer->buffer, strdup(ELMResponseStr[ELM_RESPONSE_UNKNOWN-ELMResponseOffset]), true) ) {
                log_msg(LOG_ERROR, "Error while trying to send, exiting the loop");
                return;
            }
        }
        
        sim_elm327_non_volatile_memory_store(elm327);                
    }

}
