#ifndef __AD_TERMINAL_CLI_H
#define __AD_TERMINAL_CLI_H

#include <stdio.h>
#include "libautodiag/lib.h"
#include "libprog/config.h"
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/doip/device.h"
#include "libprog/cli_helpers.h"

int terminal_cli_main(int argc, char *argv[]);

#endif