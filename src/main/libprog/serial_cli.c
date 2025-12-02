#include "libprog/serial_cli.h"

PRINT_MODULAR(serial_cli_help,
    "\n"
    "Serial client"
    "\n"
    " -h                : display this help\n"
    " -b <baud rate>    : set the baud rate\n"
    " -n [name]         : set or list serial port by name (comport, pseudo terminals, named pipes)\n"
    " -l [log level]    : set or list the log level\n"
    "\n"
)

int serial_cli_main(int argc, char *argv[]) {
    
    config_init();
    config_load();
    module_init_serial();

    argForEach() {
        if ( argIs("-h") || argIs("help") || argIs("--help") ) {
            serial_cli_help("");
            return 0;
        } else if argIs("-b") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("-b require the baud rate as argument\n");
                return 1;
            } else {
                config.com.serial.baud_rate = atoi(arg);
            }
        } else if argIs("-n") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("Ports available:\n");
                for(int i = 0; i < list_serial.size; i++) {
                    Serial * port = list_serial.list[i];
                    printf(" %s\n", port->location);
                }
                return 0;
            } else {
                config.com.serial.device_location = strdup(arg);
            }
        } else if argIs("-l") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("log levels:\n");
                printf("none\n");
                printf("error\n");                        
                printf("warning\n");                        
                printf("info\n");                        
                printf("debug\n"); 
                return 0; 
            } else {
                config.log.level = log_level_from_str(arg);
                log_set_level(config.log.level);
            }  
        } else {
            printf("Unknown argument '%s', aborting\n", argCurrent());
            return 1;
        }
    }

    config_onchange();

    Serial * serial = list_serial_get_selected();
    if ( serial == null || serial->status != SERIAL_STATE_READY ) {
        printf("Serial port seem not open\n");
        return 1;
    }

    while(true) {
        char command[1000] = {0};
        printf("> ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command)-1] = 0;
        if ( serial->send(CAST_DEVICE(serial), command) == DEVICE_ERROR ) {
            printf("Error while sending the command '%s'\n", command);
        } else {
            serial->clear_data(CAST_DEVICE(serial));
            serial->recv(CAST_DEVICE(serial));
            {
                final char * result = bytes_to_hexdump(serial->recv_buffer->buffer, serial->recv_buffer->size);
                if ( result == null ) {
                    printf("No data received from the device\n");
                } else {
                    printf("%s\n",result);
                    free(result);
                }
            }
        }
    }

    module_shutdown_serial();
    
    return 0;
}