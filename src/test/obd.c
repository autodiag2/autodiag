#include <stdio.h>
#include "com/serial/obd/obd.h"

void show_odb_strings() {
    for(int i = 0; i < 10; i++) {
        printf("%s\n", obd_get_protocol_string(OBD_INTERFACE_ELM327, i));
    }
}

int main() {
    show_odb_strings();
    return 0;
}
