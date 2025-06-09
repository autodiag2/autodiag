#include "libprog/elm327_cli.h"

int main(int argc, char**argv) {
    log_set_from_env();

    return elm327_sim_cli_main(argc,argv);    
}
