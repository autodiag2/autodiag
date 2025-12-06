#ifndef __ELM327_CLI_H
#define __ELM327_CLI_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "libautodiag/lib.h"
#include "libautodiag/sim/elm327/sim_ecu.h"
#include "libautodiag/sim/ecu/generator.h"
#include "libprog/sim_ecu_generator_gui.h"
#include "libautodiag/sim/sim.h"

void sim_elm327_cli_display_help();
int sim_elm327_cli_main(int argc, char **argv);

#endif
