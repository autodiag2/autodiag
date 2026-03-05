#include <libautodiag/com/serial/serial.h>
#include <libautodiag/com/obd/obd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char ** argv) {
    char * device_location = NULL;
    if ( 1 < argc ) {
            device_location = argv[1];
    } else {
            assert(argc == 1);
            printf("Usage: %s <device location pseudo tty, network>\n", argv[0]);
            return 1;
    }
    final Serial * serial = serial_new();
    serial->location = strdup(device_location);
    printf("serial selected location: %s\n", serial->location);
    VehicleIFace * viface = viface_open_from_device(AD_DEVICE(serial));

    viface_send(viface, ad_buffer_from_ascii_hex("0100"));
    viface_clear_data(viface);
    viface_recv(viface);

    printf("received: \n");
    for(int i = 0; i < viface->vehicle->data_buffer->size; i++) {
            char * received = ad_buffer_to_hex_string(viface->vehicle->data_buffer->list[i]);
            printf(" - %s\n", received);
    }

    return 0;
}