#ifndef __AD_DOIP_CLI_H
#define __AD_DOIP_CLI_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "libautodiag/lib.h"
#include "libautodiag/sim/ecu/ecu.h"
#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/sim/sim.h"
#include "libprog/cli_helpers.h"

int sim_doip_cli_main(int argc, char **argv);

#endif