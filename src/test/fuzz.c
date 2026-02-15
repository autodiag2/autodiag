#include "libTest.h"

bool fuzzSimELM327();

int main(int argc, char **argv) {

    log_set_from_env();
    sim_elm327_non_volatile_wipe_out();
    tf_init();

    printf(" ==== FUZZ TEST ==== \n");
    
    runIfSelected(fuzzSimELM327, null);

    printf(" ==== FUZZ TEST END ====\n");

    sim_elm327_non_volatile_wipe_out();

    return 0;
}