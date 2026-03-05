#ifndef __AD_ELM327_CLI_H
#define __AD_ELM327_CLI_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "libautodiag/lib.h"
#include "libautodiag/sim/ecu/ecu.h"
#include "libautodiag/sim/ecu/generator.h"
#include "libprog/cli_helpers.h"
#include "libautodiag/sim/sim.h"

int sim_elm327_cli_main(int argc, char **argv);

#endif
