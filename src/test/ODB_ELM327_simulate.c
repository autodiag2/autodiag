/**
 * Warning: Underlying mecanism is fifo so in so situations we can read a send.
 * Do not relying to much on this thing. 
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include "com/serial/serial.h"
#include "com/serial/at/obd/obd.h"

char * pathname = "/dev/ttyUSB7";
char * emulation_system = "fifo";
int fd;
void serial_write(char *msg) {
    char *real_msg = NULL;
    asprintf(&real_msg,"%s\r",msg);
    int ret = write(fd,real_msg,strlen(real_msg)+1);
    if ( ret == -1 ) {
        printf("Write %s error\n",msg);
    } else {
        printf("Write %s\n",msg);
    }
    free(real_msg);
}
void send_prompt() {
    serial_write(">");
}
int main(int argc, char **argv) {
    if ( 2 <= argc ) {
        pathname = argv[1];
        if ( 3 <= argc ) {
            emulation_system = "pts";
        }
    }
    if ( strcmp(emulation_system,"fifo") == 0 ) {
        fd = open(pathname,O_RDWR);
        if ( fd == -1 ) {
            if ( errno == ENOENT ) {
                if ( geteuid() != 0 ) {
                    if ( seteuid(0) == -1 ) {
                        perror("seteuid");
                        printf("Must be root to run this test\n");
                        return 1;
                    }
                }
                if ( mkfifo(pathname, O_RDWR) == -1 ) {
                    perror("Error\n");
                    return 1;
                } else {
                    if (chmod(pathname, 0x777) == -1) {
                        perror("chmod");
                        return 1;
                    } else {
                        
                    }
                }
            }
            fd = open(pathname,O_RDWR);
        }
    } else if ( strcmp(emulation_system,"pts") == 0 ) {
        
    } else {
        printf("Unknown system\n");
        return 1;
    }
    if ( fd == -1 ) {
        perror("open");
        return 1;
    } else {
        int buf_sz = 256;
        char buf[buf_sz];
        int ret;
        while(1) {
            if ( (ret = read(fd,buf,buf_sz)) == -1 ) {
                perror("read");
            } else {
                if ( strncmp(buf,"at",2) == 0 ) {
                    printf("at command\n");
                    switch(buf[2]) {
                        case 'l':
                            printf("line feeds to %d\n", buf[3]);
                            send_prompt();
                            break;
                        case 'e':
                            printf("setting echo to %d\n", buf[3]);
                            send_prompt();
                            break;
                        case 'i':
                            printf("ID command received\n");
                            printf("Sending ID=%s\n", OBD_INTERFACE_ID_ELM327);
                            serial_write(OBD_INTERFACE_ID_ELM327);
                            sleep(1);
                            send_prompt();
                            break;
                    }
                } else {
                    printf("Unknown command received %c%c%c%c\n", buf[0],buf[1],buf[2],buf[3]);
                    send_prompt();
                }
            }
        }
    }
}

