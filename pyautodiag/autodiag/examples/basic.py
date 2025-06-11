from autodiag.libautodiag.sim.elm327.sim import ECUEmulation

ecuEmulation = ECUEmulation(0x70)
print("ECUEmulation created with address:", hex(ecuEmulation.address))

ecuEmulation = ECUEmulation(0x71)
print("ECUEmulation created with address:", hex(ecuEmulation.address))

from autodiag.libautodiag.installation import installation_folder
print("Installation folder:", installation_folder("ui"))

from autodiag.libautodiag.log import *
log_set_level(LOG_DEBUG)
log_msg(LOG_INFO, "This is an info message")
log_msg(LOG_WARNING, "This is a warning message")
log_msg(LOG_ERROR, "This is an error message")
log_msg(LOG_DEBUG, "This is a debug message")