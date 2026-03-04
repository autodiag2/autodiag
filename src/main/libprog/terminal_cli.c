#include "libprog/terminal_cli.h"

PRINT_MODULAR(terminal_cli_help,
    "\n"
    "Terminal client"
    "\n"
    " -h                : display this help\n"
    " -l [log level]    : set or list the log level\n"
    " -n [name]         : set or list terminal location (comport, pseudo terminals, named pipes, doip)\n"
    " -t [type]         : set or list the type of device to connect to (serial: default, doip)\n"
    " Serial options:\n"
    "  -b <baud rate>    : set the baud rate\n"
    "\n"
)

int terminal_cli_main(int argc, char *argv[]) {
    
    config_init();
    config_load();
    object_DeviceTable * device_table = config.ephemere.device_table;
    device_table_fill(device_table);

    argForEach() {
        if ( argIs("-h") || argIs("help") || argIs("--help") ) {
            terminal_cli_help("");
            return 0;
        } else if argIs("-b") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("-b require the baud rate as argument\n");
                return 1;
            } else {
                config.com.device.serial.baud_rate = atoi(arg);
            }
        } else if argIs("-n") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("devices available:\n");
                for(int i = 0; i < device_table->list->size; i++) {
                    Device * device = device_table->list->list[i];
                    printf(" %s (%s)\n", device->location, device_type_as_string(device->type));
                }
                return 0;
            } else {
                config.com.device.location = strdup(arg);
            }
        } else if argIs("-t") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("device types:\n");
                printf("serial\n");
                printf("doip\n");
                return 0;
            } else {
                if ( strcasecmp(arg, "serial") == 0 ) {
                    config.com.device.type = AD_DEVICE_TYPE_SERIAL;
                } else if ( strcasecmp(arg, "doip") == 0 ) {
                    config.com.device.type = AD_DEVICE_TYPE_DOIP;
                } else {
                    printf("Unknown device type '%s', aborting\n", arg);
                    return 1;
                }
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

    final Device * device = device_table_get_selected(device_table);
    if ( device == null ) {
        printf("No device selected, aborting\n");
        return 1;
    }

    while(true) {
        if ( device->type == AD_DEVICE_TYPE_DOIP ) {
            object_DoIPDevice * doip_device = (object_DoIPDevice *) device;
            if ( doip_device->status != DEVICE_DOIP_STATUS_OPEN ) {
                printf("Error while receiving data from the device\n");
                return 1;
            }
        } else if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
            Serial * serial = (Serial *) device;
            if ( serial->status != SERIAL_STATE_READY ) {
                printf("Error while receiving data from the device\n");
                return 1;
            }
        } else {
            printf("Unsupported device type\n");
            return 1;
        }
        char command[1000] = {0};
        printf("> ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command)-1] = 0;
        if ( device->send(device, command) == DEVICE_ERROR ) {
            printf("Error while sending the command '%s'\n", command);
        } else {
            device->clear_data(device);
            device->recv(device);
            {
                char * result = null;
                if ( device->type == AD_DEVICE_TYPE_DOIP ) {
                    object_DoIPDevice * doip_device = (object_DoIPDevice *) device;
                    result = buffer_to_hexdump(doip_device->recv_buffer);
                } else if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
                    Serial * serial = (Serial *) device;
                    result = buffer_to_hexdump(serial->recv_buffer);
                } else {
                    printf("Unsupported device type\n");
                    return 1;
                }
                if ( result == null || strlen(result) == 0 ) {
                    printf("No data received from the device\n");
                } else {
                    printf("%s\n",result);
                    free(result);
                }
            }
        }
    }

    device_table_close_selected(config.ephemere.device_table);
    device_table_free(config.ephemere.device_table);
    
    return 0;
}