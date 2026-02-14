#include "libprog/cli_helpers.h"

#ifndef COMPILE_COMPAT
    int SimECUGeneratorGui_cmp(SimECUGeneratorGui*g1, SimECUGeneratorGui*g2) {
        return g1 - g2;
    }
    AD_LIST_SRC(SimECUGeneratorGui)
#endif