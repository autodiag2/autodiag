#ifndef __AD_CLI_HELPERS_H
#define __AD_CLI_HELPERS_H

#include "libautodiag/lib.h"

#ifndef COMPILE_COMPAT
#   include "libprog/sim_ecu_generator_gui.h"
    AD_LIST_H(SimECUGeneratorGui)
#endif
/**
 * Print a list of available env vars
 */
void ad_print_env_vars();

#endif