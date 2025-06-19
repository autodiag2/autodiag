#include "libTest.h"

#define SIM_START_WAIT_MS 100e3
void testSIM_1() {

    SimELM327* elm327 = sim_elm327_new();
    SimECU_list_append(elm327->ecus,sim_ecu_emulation_new(0xE9));
    SimECU_list_append(elm327->ecus,sim_ecu_emulation_new(0x1A));        
    sim_elm327_loop_as_daemon(elm327);
    usleep(SIM_START_WAIT_MS);
    final OBDIFace* iface = port_open(strdup(elm327->device_location));

    {
        int sz = 4;
        char cmds[][100] = {"ate0","ATE0","at e0","AT E0"};
        for(int i = 0; i < sz; i++) {
            printf("i=%d cmd=%s\n", i, cmds[i]);
            iface->device->send(iface->device,cmds[i]);
            obd_clear_data(iface);
            assert(iface->device->recv(iface->device) == SERIAL_RESPONSE_OK);
        }
    }
    {
        for(int i = 1; i <= 9; i ++) {
            char *setProto;
            asprintf(&setProto,"atsp %01x", i);
            obd_clear_data(iface);
            iface->device->send(iface->device,setProto);
            iface->device->recv(iface->device);
            ((ELM327Device*)iface->device)->protocol = i;
            free(setProto);
            {
                obd_send(iface,"0101");
                obd_clear_data(iface);
                obd_recv(iface);
                assert(iface->vehicle->data_buffer->size == 3);

                obd_clear_data(iface);
                iface->device->send(iface->device,"atsr E9");
                iface->device->recv(iface->device);
                obd_send(iface,"0101");
                obd_clear_data(iface);
                obd_recv(iface);
                assert(iface->vehicle->data_buffer->size == 1);        

                obd_clear_data(iface);
                iface->device->send(iface->device,"atar");
                iface->device->recv(iface->device);
                obd_send(iface,"0101");
                obd_clear_data(iface);
                obd_recv(iface);
                assert(iface->vehicle->data_buffer->size == 3);
            }
        }
        
        obd_clear_data(iface);    
        iface->device->send(iface->device,"atsp B");
        iface->device->recv(iface->device);
        ((ELM327Device*)iface->device)->protocol = 0xB;    
        {
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);
            
            obd_clear_data(iface);
            iface->device->send(iface->device,"atsr E9");
            iface->device->recv(iface->device);
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);        

            obd_clear_data(iface);
            iface->device->send(iface->device,"atar");
            iface->device->recv(iface->device);
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);
        }   
        {
            obd_clear_data(iface);    
            iface->device->send(iface->device,"atcm FF0");
            iface->device->recv(iface->device);
            obd_clear_data(iface);    
            iface->device->send(iface->device,"atcf 710");
            iface->device->recv(iface->device);        
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);  
        }
        {
            obd_clear_data(iface);    
            iface->device->send(iface->device,"atcra");
            iface->device->recv(iface->device);
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);
            obd_clear_data(iface);    
            iface->device->send(iface->device,"atcra 7EX");
            iface->device->recv(iface->device);
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->data_buffer->size == 2);
        }        
    }
}
void ensureReplayCommands() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    usleep(SIM_START_WAIT_MS);
    final OBDIFace* iface = port_open(strdup(elm327->device_location));
    obd_clear_data(iface);
    obd_send(iface, "0101");
    obd_recv(iface);
    assert(iface->vehicle->data_buffer->size == 1);
    obd_send(iface, "");
    obd_recv(iface);
    assert(iface->vehicle->data_buffer->size == 2);
}
void anyCommandShouldReplyUnknown() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    usleep(SIM_START_WAIT_MS);
    final OBDIFace* iface = port_open(strdup(elm327->device_location));
    obd_clear_data(iface);
    obd_send(iface, "azrer");
    obd_recv(iface);
    Serial * serial = (Serial*)iface->device;
    buffer_ensure_termination(serial->recv_buffer);
    assert(strncmp("?", serial->recv_buffer->buffer, 1) == 0);
}
void emptyOrTooShortCommandShouldNotTriggerSegFault() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    usleep(SIM_START_WAIT_MS);
    final OBDIFace* iface = port_open(strdup(elm327->device_location));
    final Serial * port = (Serial*)iface->device;
    char buffer[] = "a";
    int bytes_sent = write(port->implementation->fdtty,buffer,strlen(buffer));
    obd_recv(iface);
}
bool testSIM() {
    emptyOrTooShortCommandShouldNotTriggerSegFault();
    anyCommandShouldReplyUnknown();
    ensureReplayCommands();
    {
        log_msg(LOG_INFO, "Random generate gives different values over two different runs");
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        final Serial* serial = (Serial*)iface->device;
        iface->device->send(iface->device,"0104");
        obd_clear_data(iface);
        iface->device->recv(iface->device);
        Buffer * response = buffer_copy(serial->recv_buffer);
        iface->device->send(iface->device,"0104");
        obd_clear_data(iface);
        iface->device->recv(iface->device);
        assert( ! buffer_cmp(response, serial->recv_buffer));
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        final Serial* serial = (Serial*)iface->device;
        iface->device->send(iface->device,"atcaf0");
        iface->device->recv(iface->device);
        iface->device->send(iface->device,"ath1");
        iface->device->recv(iface->device);
        obd_clear_data(iface);
        iface->device->send(iface->device,"030902");
        iface->device->recv(iface->device);
        assert(strstr(serial->recv_buffer->buffer, "<DATA ERROR") != null);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        final Serial* serial = (Serial*)iface->device;
        obd_clear_data(iface);
        iface->device->send(iface->device,"unknown");
        iface->device->recv(iface->device);
        buffer_dump(serial->recv_buffer);
        assert(strstr(serial->recv_buffer->buffer, "?") != null);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        obd_clear_data(iface);
        iface->device->send(iface->device,"ath0");
        iface->device->recv(iface->device);
        obd_clear_data(iface);
        iface->device->send(iface->device,"0902");
        iface->device->recv(iface->device);
        final Serial* serial = (Serial*)iface->device;
        assert(strstr(serial->recv_buffer->buffer, "019") != null);
        assert(strstr(serial->recv_buffer->buffer, "0:") != null);
        assert(strstr(serial->recv_buffer->buffer, "1:") != null);
        assert(strstr(serial->recv_buffer->buffer, "2:") != null);
        assert(strstr(serial->recv_buffer->buffer, "4902") != null);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        obd_clear_data(iface);
        iface->device->send(iface->device,"atbrd 12");
        obd_clear_data(iface);
        iface->device->recv(iface->device); // OK
        final Serial* serial = (Serial*)iface->device;
        assert(strstr(serial->recv_buffer->buffer, "OK") != null);
        obd_clear_data(iface);
        iface->device->recv(iface->device); // ATI
        obd_clear_data(iface);
        iface->device->send(iface->device,"\r");
        obd_clear_data(iface);
        iface->device->recv(iface->device); // OK + Prompt
    }
    testSIM_1();
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        obd_clear_data(iface);
        iface->device->send(iface->device,"atcea 12");
        iface->device->recv(iface->device);
        obd_clear_data(iface);
        iface->device->send(iface->device,"ats1");
        iface->device->recv(iface->device);
        obd_send(iface,"0101");
        obd_clear_data(iface);
        assert(obd_recv(iface) != OBD_RECV_ERROR);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        obd_clear_data(iface);
        iface->device->send(iface->device,"atd");
        int recv = iface->device->recv(iface->device);
        printf("recv=%d\n", recv);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        obd_clear_data(iface);
        obd_send(iface, "0900");
        obd_clear_data(iface);
        obd_recv(iface);
        assert(buffer_cmp(iface->vehicle->data_buffer->list[0], buffer_from_ascii_hex("4900FFFFFFFF")));
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->device_location));
        obd_clear_data(iface);
        obd_send(iface, "0902");
        obd_clear_data(iface);
        obd_recv(iface);
        Buffer_list_dump(iface->vehicle->data_buffer);
        assert(17 < iface->vehicle->data_buffer->list[0]->size);
    }
    return true;
}
