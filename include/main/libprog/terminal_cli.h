#ifndef __TERMINAL_CLI_H
#define __TERMINAL_CLI_H

#include <stdio.h>
#include "libautodiag/lib.h"
#include "libprog/config.h"
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/doip/device.h"

int terminal_cli_main(int argc, char *argv[]);

#endif