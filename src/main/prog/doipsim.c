#include "libprog/doip_cli.h"

int main(int argc, char**argv) {
    log_set_from_env();

    ad_object_vehicle_signal_register_all();

    return sim_doip_cli_main(argc,argv);    
}