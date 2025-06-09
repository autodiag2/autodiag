#ifndef __ELM327_CLI_H
#define __ELM327_CLI_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "libautodiag/lib.h"
#include "libautodiag/sim/elm327/sim.h"
#include "libautodiag/sim/elm327/sim_generators.h"
#include "libprog/sim_obd_generators.h"

void elm327_sim_cli_display_help();
int elm327_sim_cli_main(int argc, char **argv);

#endif
