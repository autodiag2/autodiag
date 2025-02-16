#ifndef __ELM327_CLI_H
#define __ELM327_CLI_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "sim.h"

void elm327_sim_cli_display_help();
int elm327_sim_cli_main(int argc, char **argv, int starti);

#endif
