#include "libprog/cli_helpers.h"

#ifndef COMPILE_COMPAT
    int SimECUGeneratorGui_cmp(SimECUGeneratorGui*g1, SimECUGeneratorGui*g2) {
        return g1 - g2;
    }
    AD_LIST_SRC(SimECUGeneratorGui)
#endif

void ad_print_env_vars() {
    printf("Runtime environment variables\n");
    printf(" AUTODIAG_LOG_LEVEL         - log messages with defined log level (none,info,warning,error,debug)\n");
    printf(" AUTODIAG_LOG_TIMESTAMP     - log messages with timestamp (true, false and require log level)\n");
    printf(" AUTODIAG_LOG_CODE_LOCATION - log messages with source code location (true, false and require log level)\n");
    printf(" AUTODIAG_LOG_COLOR         - log messages with colors (true, false and require log level)\n");
}