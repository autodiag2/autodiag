#include "libprog/elm327_cli.h"

int main(int argc, char**argv) {
    log_set_from_env();

    return sim_elm327_cli_main(argc,argv);    
}
