from autodiag.sim.ecu.ecu import SimECU

ecuEmulation = SimECU(0x70)
print("SimECU created with address:", hex(ecuEmulation.address))

ecuEmulation = SimECU(0x71)
print("SimECU created with address:", hex(ecuEmulation.address))

from autodiag.installation import installation_folder_resolve
print("Installation folder:", installation_folder_resolve("ui"))

from autodiag.log import *
log_set_level(LOG_DEBUG)
log_msg("This is an info message", LOG_INFO)
log_msg("This is a warning message", LOG_WARNING)
log_msg("This is an error message", LOG_ERROR)
log_msg("This is a debug message", LOG_DEBUG)