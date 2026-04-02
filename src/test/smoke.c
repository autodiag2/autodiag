#include "libTest.h"

bool testExprSmoke();

int main(int argc, char **argv) {

    log_set_from_env();
    sim_elm327_non_volatile_wipe_out();
    tf_init();

    printf(" ==== SMOKE TEST ==== \n");
    
    runIfSelected(testExprSmoke, null);

    printf(" ==== SMOKE TEST END ====\n");

    sim_elm327_non_volatile_wipe_out();

    return 0;
}