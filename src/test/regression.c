#include "com/serial/obd/obd.h"
#include "lib.h"

bool testOBD(OBDIFACE iface) {
    {
        OBDIFACE tmp = obd_new();
        tmp->printing_of_spaces = false;
        tmp->printing_of_headers = false;
        BUFFER buffer = buffer_new();
        buffer_append_str(buffer,"000102030405060708090A0B0C0D0E0F",32);
        BUFFER bin = obd_serial_ascii_to_bin(tmp,buffer);
        if ( bin->size_used != 16 ) return false;
        for(int i = 0; i < bin->size_used; i++) {
            if ( bin->buffer[i] != i ) return false;
        }
    }
    {
        OBDIFACE tmp = obd_new();
        tmp->printing_of_spaces = true;
        tmp->printing_of_headers = false;
        BUFFER buffer = buffer_new();
        buffer_append_str(buffer,"00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F",48);
        BUFFER bin = obd_serial_ascii_to_bin(tmp,buffer);
        if ( bin->size_used != 16 ) return false;
        for(int i = 0; i < bin->size_used; i++) {
            if ( bin->buffer[i] != i ) return false;
        }
    }
    return true;
}
bool testSAEJ1979(OBDIFACE iface) {
    {
        if ( ! saej1979_is_pid_supported(iface, 0) ) return false;
        if ( ! saej1979_is_pid_supported(iface, 1) ) return false;
        return true;
    }
}
int main() {
    final OBDIFACE iface = port_open_on_new_simulation();
    runTest("obd.c", testOBD, iface);
    runTest("saej1979", testSAEJ1979, iface);
    port_close_destruct_simulation(iface);
    return 0;
}
