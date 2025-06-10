from pyautodiag.libautodiag.com.obd.vehicle import Vehicle

from pyautodiag.libautodiag.log import *


v = Vehicle()
v.ecu_add(b'\x01')
v.ecu_add(b'\x02')
log_set_level(LOG_DEBUG)
v.dump()