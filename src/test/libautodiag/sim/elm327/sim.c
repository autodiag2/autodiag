#include "libTest.h"

void testSimELM327_1() {

    SimELM327* elm327 = sim_elm327_new();
    list_SimECU_append(elm327->ecus,sim_ecu_new(0xE9));
    list_SimECU_append(elm327->ecus,sim_ecu_new(0x1A));        
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));

    {
        int sz = 4;
        char cmds[][100] = {"ate0","ATE0","at e0","AT E0"};
        for(int i = 0; i < sz; i++) {
            printf("i=%d cmd=%s\n", i, cmds[i]);
            iface->device->send(iface->device,cmds[i]);
            viface_clear_data(iface);
            assert(iface->device->recv(iface->device) == SERIAL_RESPONSE_OK);
        }
    }
    {
        for(int i = 1; i <= 9; i ++) {
            char *setProto;
            asprintf(&setProto,"atsp %01x", i);
            viface_clear_data(iface);
            iface->device->send(iface->device,setProto);
            iface->device->recv(iface->device);
            ((ELM327Device*)iface->device)->protocol = i;
            free(setProto);
            {
                viface_send_str(iface,"0101");
                viface_clear_data(iface);
                viface_recv(iface);
                assert(iface->vehicle->data_buffer->size == 3);

                viface_clear_data(iface);
                iface->device->send(iface->device,"atsr E9");
                iface->device->recv(iface->device);
                viface_send_str(iface,"0101");
                viface_clear_data(iface);
                viface_recv(iface);
                assert(iface->vehicle->data_buffer->size == 1);        

                viface_clear_data(iface);
                iface->device->send(iface->device,"atar");
                iface->device->recv(iface->device);
                viface_send_str(iface,"0101");
                viface_clear_data(iface);
                viface_recv(iface);
                assert(iface->vehicle->data_buffer->size == 3);
            }
        }
        
        viface_clear_data(iface);    
        iface->device->send(iface->device,"atsp B");
        iface->device->recv(iface->device);
        ((ELM327Device*)iface->device)->protocol = 0xB;    
        {
            viface_send_str(iface,"0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);
            
            viface_clear_data(iface);
            iface->device->send(iface->device,"atsr E9");
            iface->device->recv(iface->device);
            viface_send_str(iface,"0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);        

            viface_clear_data(iface);
            iface->device->send(iface->device,"atar");
            iface->device->recv(iface->device);
            viface_send_str(iface,"0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);
        }   
        {
            viface_clear_data(iface);    
            iface->device->send(iface->device,"atcm FF0");
            iface->device->recv(iface->device);
            viface_clear_data(iface);    
            iface->device->send(iface->device,"atcf 710");
            iface->device->recv(iface->device);        
            viface_send_str(iface,"0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);  
        }
        {
            viface_clear_data(iface);    
            iface->device->send(iface->device,"atcra");
            iface->device->recv(iface->device);
            viface_send_str(iface,"0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 3);
            viface_clear_data(iface);    
            iface->device->send(iface->device,"atcra 7EX");
            iface->device->recv(iface->device);
            viface_send_str(iface,"0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 2);
        }        
    }
}
void ensureDisplayWithSpacesIsCorrect() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    final Serial * serial = iface->device;
    assert(elm327_printing_of_spaces(iface->device, true));
    iface->device->send(iface->device, "0100");
    viface_clear_data(iface);
    iface->device->recv(iface->device);
    buffer_ensure_termination(serial->recv_buffer);
    assert(strstr(serial->recv_buffer->buffer, "7E8") != null);
}
void ensureWithoutHeadersDontSendNull() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    final Serial * serial = iface->device;
    elm327_printing_of_spaces(iface->device, true);
    elm_echo(serial, true);
    serial_query_at_command(serial, "h0");
    iface->device->send(iface->device, "0100");
    viface_clear_data(iface);
    iface->device->recv(iface->device);
    buffer_ensure_termination(serial->recv_buffer);
    assert(strstr(serial->recv_buffer->buffer, "null") == null);
}
void ensureReplayCommands() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    viface_clear_data(iface);
    viface_send_str(iface, "0101");
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 1);
    viface_send_str(iface, ""); // does it works when changing the pp value of carriage return ? for the emulator point of view not (to test on the real device)
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 2);
}
void anyCommandShouldReplyUnknown() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    viface_clear_data(iface);
    viface_send_str(iface, "azrer");
    viface_recv(iface);
    Serial * serial = (Serial*)iface->device;
    buffer_ensure_termination(serial->recv_buffer);
    assert(strncmp("?", serial->recv_buffer->buffer, 1) == 0);
}
void incomplete_string_return_after_20_secs() {
    SimELM327* elm327 = sim_elm327_new();       
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    final Serial* serial = (Serial*)iface->device;
    buffer_recycle(serial->recv_buffer);
    serial_send_internal(serial, "ati", 3);
    assert(serial_recv(serial) == DEVICE_RECV_NULL);
    usleep(20 + 3);
    assert(serial_recv_internal(serial) > 0);
    assert(strncmp(serial->recv_buffer->buffer, "?", 1) == 0);
}
bool testSimELM327() {
    ensureWithoutHeadersDontSendNull();
    ensureDisplayWithSpacesIsCorrect();
    //incomplete_string_return_after_20_secs();
    anyCommandShouldReplyUnknown();
    ensureReplayCommands();
    {
        log_msg(LOG_INFO, "Random generate gives different values over two different runs");
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        final Serial* serial = (Serial*)iface->device;
        iface->device->send(iface->device,"0104");
        viface_clear_data(iface);
        iface->device->recv(iface->device);
        Buffer * response = buffer_copy(serial->recv_buffer);
        iface->device->send(iface->device,"0104");
        viface_clear_data(iface);
        iface->device->recv(iface->device);
        assert(buffer_cmp(response, serial->recv_buffer) != 0);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        final Serial* serial = (Serial*)iface->device;
        iface->device->send(iface->device,"atcaf0");
        iface->device->recv(iface->device);
        iface->device->send(iface->device,"ath1");
        iface->device->recv(iface->device);
        viface_clear_data(iface);
        iface->device->send(iface->device,"030902");
        iface->device->recv(iface->device);
        assert(strstr(serial->recv_buffer->buffer, "<DATA ERROR") != null);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        final Serial* serial = (Serial*)iface->device;
        viface_clear_data(iface);
        iface->device->send(iface->device,"unknown");
        iface->device->recv(iface->device);
        buffer_dump(serial->recv_buffer);
        assert(strstr(serial->recv_buffer->buffer, "?") != null);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        viface_clear_data(iface);
        iface->device->send(iface->device,"ath0");
        iface->device->recv(iface->device);
        viface_clear_data(iface);
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
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        viface_clear_data(iface);
        iface->device->send(iface->device,"atbrd 12");
        viface_clear_data(iface);
        iface->device->recv(iface->device); // OK
        final Serial* serial = (Serial*)iface->device;
        assert(strstr(serial->recv_buffer->buffer, "OK") != null);
        viface_clear_data(iface);
        iface->device->recv(iface->device); // ATI
        viface_clear_data(iface);
        iface->device->send(iface->device,"\r");
        viface_clear_data(iface);
        iface->device->recv(iface->device); // OK + Prompt
    }
    testSimELM327_1();
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        viface_clear_data(iface);
        iface->device->send(iface->device,"atcea 12");
        iface->device->recv(iface->device);
        viface_clear_data(iface);
        iface->device->send(iface->device,"ats1");
        iface->device->recv(iface->device);
        viface_send_str(iface,"0101");
        viface_clear_data(iface);
        assert(viface_recv(iface) != VIFACE_RECV_ERROR);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        viface_clear_data(iface);
        iface->device->send(iface->device,"atd");
        int recv = iface->device->recv(iface->device);
        printf("recv=%d\n", recv);
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        viface_clear_data(iface);
        viface_send_str(iface, "0900");
        viface_clear_data(iface);
        viface_recv(iface);
        assert(0 == buffer_cmp(iface->vehicle->data_buffer->list[0], buffer_from_ascii_hex("4900FFFFFFFFFF")));
    }
    {
        SimELM327* elm327 = sim_elm327_new();       
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        viface_clear_data(iface);
        viface_send_str(iface, "0902");
        viface_clear_data(iface);
        viface_recv(iface);
        list_Buffer_dump(iface->vehicle->data_buffer);
        assert(17 < iface->vehicle->data_buffer->list[0]->size);
    }
    return true;
}
