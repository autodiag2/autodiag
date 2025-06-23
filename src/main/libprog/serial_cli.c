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
    
    config_load();
    module_init_serial();

    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, "hb:n:l:")) != -1) {
        switch (opt) {
            case 'h': {
                serial_cli_help("");
                return 0;
            } break;
            case 'b': {
                config.com.serial.baud_rate = atoi(optarg);
            } break;
            case 'n': {
                config.com.serial.device_location = strdup(optarg);
            } break;
            case 'l': {
                config.log.level = log_level_from_str(optarg);
                log_set_level(config.log.level);
            } break;
            case '?': {
                switch(optopt) {
                    case 'n': {
                        printf("Ports available:\n");
                        for(int i = 0; i < list_serial.size; i++) {
                            Serial * port = list_serial.list[i];
                            printf(" %s\n", port->location);
                        }
                        return 0;
                    } break;
                    case 'l': {
                        printf("log levels:\n");
                        printf("none\n");
                        printf("error\n");                        
                        printf("warning\n");                        
                        printf("info\n");                        
                        printf("debug\n");    
                        break; 
                    } break;
                    default: {
                        serial_cli_help("");
                        return 1;
                    }
                }
            } break;
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