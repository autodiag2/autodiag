#include "libTest.h"

#define SIM_START_WAIT_MS 100e3
void testSIM_1() {

    ELM327emulation* elm327 = elm327_sim_new();
    ECUEmulation_list_append(elm327->ecus,ecu_emulation_new(0xE9));
    ECUEmulation_list_append(elm327->ecus,ecu_emulation_new(0x1A));        
    elm327_sim_loop_start(elm327);
    usleep(SIM_START_WAIT_MS);
    final OBDIFace* iface = port_open(strdup(elm327->port_name));

    {
        int sz = 4;
        char cmds[][100] = {"ate0","ATE0","at e0","AT E0"};
        for(int i = 0; i < sz; i++) {
            printf("i=%d cmd=%s\n", i, cmds[i]);
            iface->device->send(DEVICE(iface->device),cmds[i]);
            obd_clear_data(iface);
            assert(iface->device->recv(DEVICE(iface->device)) == SERIAL_RESPONSE_OK);
        }
    }
    {
        for(int i = 1; i <= 9; i ++) {
            char *setProto;
            asprintf(&setProto,"atsp %01x", i);
            obd_clear_data(iface);
            iface->device->send(DEVICE(iface->device),setProto);
            iface->device->recv(DEVICE(iface->device));
            ((ELM327Device*)iface->device)->protocol = i;
            free(setProto);
            {
                obd_send(iface,"0101");
                obd_clear_data(iface);
                obd_recv(iface);
                assert(iface->vehicle->obd_data_buffer->size == 3);

                obd_clear_data(iface);
                iface->device->send(DEVICE(iface->device),"atsr E9");
                iface->device->recv(DEVICE(iface->device));
                obd_send(iface,"0101");
                obd_clear_data(iface);
                obd_recv(iface);
                assert(iface->vehicle->obd_data_buffer->size == 1);        

                obd_clear_data(iface);
                iface->device->send(DEVICE(iface->device),"atar");
                iface->device->recv(DEVICE(iface->device));
                obd_send(iface,"0101");
                obd_clear_data(iface);
                obd_recv(iface);
                assert(iface->vehicle->obd_data_buffer->size == 3);
            }
        }
        
        obd_clear_data(iface);    
        iface->device->send(DEVICE(iface->device),"atsp B");
        iface->device->recv(DEVICE(iface->device));
        ((ELM327Device*)iface->device)->protocol = 0xB;    
        {
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->obd_data_buffer->size == 3);
            
            obd_clear_data(iface);
            iface->device->send(DEVICE(iface->device),"atsr E9");
            iface->device->recv(DEVICE(iface->device));
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->obd_data_buffer->size == 3);        

            obd_clear_data(iface);
            iface->device->send(DEVICE(iface->device),"atar");
            iface->device->recv(DEVICE(iface->device));
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->obd_data_buffer->size == 3);
        }   
        {
            obd_clear_data(iface);    
            iface->device->send(DEVICE(iface->device),"atcm FF0");
            iface->device->recv(DEVICE(iface->device));
            obd_clear_data(iface);    
            iface->device->send(DEVICE(iface->device),"atcf 710");
            iface->device->recv(DEVICE(iface->device));        
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->obd_data_buffer->size == 1);  
        }
        {
            obd_clear_data(iface);    
            iface->device->send(DEVICE(iface->device),"atcra");
            iface->device->recv(DEVICE(iface->device));
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->obd_data_buffer->size == 3);
            obd_clear_data(iface);    
            iface->device->send(DEVICE(iface->device),"atcra 7EX");
            iface->device->recv(DEVICE(iface->device));
            obd_send(iface,"0101");
            obd_clear_data(iface);
            obd_recv(iface);
            assert(iface->vehicle->obd_data_buffer->size == 2);
        }        
    }
}

bool testSIM() {
    {
        log_msg(LOG_INFO, "Random generate gives different values over two different runs");
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        final Serial* serial = (Serial*)iface->device;
        iface->device->send(DEVICE(iface->device),"0104");
        obd_clear_data(iface);
        iface->device->recv(DEVICE(iface->device));
        Buffer * response = buffer_copy(serial->recv_buffer);
        iface->device->send(DEVICE(iface->device),"0104");
        obd_clear_data(iface);
        iface->device->recv(DEVICE(iface->device));
        assert( ! buffer_cmp(response, serial->recv_buffer));
    }
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        final Serial* serial = (Serial*)iface->device;
        iface->device->send(DEVICE(iface->device),"atcaf0");
        iface->device->recv(DEVICE(iface->device));
        iface->device->send(DEVICE(iface->device),"ath1");
        iface->device->recv(DEVICE(iface->device));
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"030902");
        iface->device->recv(DEVICE(iface->device));
        assert(strstr(serial->recv_buffer->buffer, "<DATA ERROR") != null);
    }
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        final Serial* serial = (Serial*)iface->device;
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"unknown");
        iface->device->recv(DEVICE(iface->device));
        buffer_dump(serial->recv_buffer);
        assert(strstr(serial->recv_buffer->buffer, "?") != null);
    }
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"ath0");
        iface->device->recv(DEVICE(iface->device));
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"0902");
        iface->device->recv(DEVICE(iface->device));
        final Serial* serial = (Serial*)iface->device;
        assert(strstr(serial->recv_buffer->buffer, "019") != null);
        assert(strstr(serial->recv_buffer->buffer, "0:") != null);
        assert(strstr(serial->recv_buffer->buffer, "1:") != null);
        assert(strstr(serial->recv_buffer->buffer, "2:") != null);
        assert(strstr(serial->recv_buffer->buffer, "4902") != null);
    }
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"atbrd 12");
        obd_clear_data(iface);
        iface->device->recv(DEVICE(iface->device)); // OK
        final Serial* serial = (Serial*)iface->device;
        assert(strstr(serial->recv_buffer->buffer, "OK") != null);
        obd_clear_data(iface);
        iface->device->recv(DEVICE(iface->device)); // ATI
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"\r");
        obd_clear_data(iface);
        iface->device->recv(DEVICE(iface->device)); // OK + Prompt
    }
    testSIM_1();
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"atcea 12");
        iface->device->recv(DEVICE(iface->device));
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"ats1");
        iface->device->recv(DEVICE(iface->device));
        obd_send(iface,"0101");
        obd_clear_data(iface);
        assert(obd_recv(iface) != OBD_RECV_ERROR);
    }
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        obd_clear_data(iface);
        iface->device->send(DEVICE(iface->device),"atd");
        int recv = iface->device->recv(DEVICE(iface->device));
        printf("recv=%d\n", recv);
    }
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        obd_clear_data(iface);
        obd_send(iface, "0900");
        obd_clear_data(iface);
        obd_recv(iface);
        assert(buffer_cmp(iface->vehicle->obd_data_buffer->list[0], buffer_from_ascii_hex("4900FFFFFFFF")));
    }
    {
        ELM327emulation* elm327 = elm327_sim_new();       
        elm327_sim_loop_start(elm327);
        usleep(SIM_START_WAIT_MS);
        final OBDIFace* iface = port_open(strdup(elm327->port_name));
        obd_clear_data(iface);
        obd_send(iface, "0902");
        obd_clear_data(iface);
        obd_recv(iface);
        BufferList_dump(iface->vehicle->obd_data_buffer);
        assert(17 < iface->vehicle->obd_data_buffer->list[0]->size);
    }
    return true;
}
