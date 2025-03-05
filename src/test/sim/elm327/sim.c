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
        assert(buffer_cmp(iface->vehicle->obd_data_buffer->list[0], ascii_to_bin_buffer("4900FFFFFFFF")));
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
