#include "libprog/doip_cli.h"

int main(int argc, char**argv) {
    log_set_from_env();

    return sim_doip_cli_main(argc,argv);    
}