
from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.vehicle_interface import VehicleIFace
from autodiag.log import *
from autodiag.com.uds.uds import *
from autodiag.sim.ecu.generator import *

def test_UDS():
    #log_set_level(LOG_DEBUG)
    emulation = SimELM327()
    emulation.set_ecu_and_generator(0xE8, GeneratorCitroenC5X7())
    emulation.loop(daemon=True)

    serial = Serial()
    serial.set_location(emulation.device_location)
    iface = VehicleIFace.open_from_device(serial)

    uds = UDS(iface)
    assert uds.is_enabled()

    assert uds.request_session(UDS_SESSION.DEFAULT)
    assert uds.request_session(UDS_SESSION.PROGRAMMING)
    assert uds.request_session(UDS_SESSION.EXTENDED_DIAGNOSTIC)
    assert uds.request_session(UDS_SESSION.SYSTEM_SAFETY_DIAGNOSTIC)

    assert uds.tester_present()

    dtcs = uds.read_dtcs()
    assert 0 < len(dtcs)
    result = 0b11
    for dtc in dtcs:
        dtc.debug_from_python()
        dtc_bin_str = bytes(dtc.dtc.data).hex()
        if dtc_bin_str == "0103":
            result &= 0b10
        if dtc_bin_str == "0104":
            result &= 0b01
    
    assert result == 0b00

    assert uds.clear_dtcs()

    assert 0 == len(uds.read_dtcs())