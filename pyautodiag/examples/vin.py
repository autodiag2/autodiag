from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import *
from autodiag.sim.elm327.sim_generators import SimECUGeneratorCitroenC5X7

emulation = SimELM327()
emulation.set_ecu_and_generator(0xE8, SimECUGeneratorCitroenC5X7())
emulation.loop(daemon=True)

serial = Serial()
serial.set_location(emulation.device_location)
iface = OBDIFace.open_from_device(serial)
print(f"vin={iface.vehicle.contents.vin.contents.to_ascii()}")
print(f"country={iface.vehicle.contents.country.decode()}")
print(f"manufacturer={iface.vehicle.contents.manufacturer.decode()}")
print(f"year={iface.vehicle.contents.year}")
