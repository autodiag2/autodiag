from autodiag.com.obd.vehicle import Vehicle

from autodiag.log import *


v = Vehicle()
v.ecu_add(b'\x01')
v.ecu_add(b'\x02')
log_set_level(LOG_DEBUG)
v.dump()