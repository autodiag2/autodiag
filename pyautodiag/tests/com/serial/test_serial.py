from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.device import *

def test_serial():
    emulation = SimELM327()
    emulation.loop(daemon=True)

    serial = Serial()
    location = "abcd"
    serial.set_location(location)
    assert serial.location.decode() == location

    serial.set_location(emulation.device_location)
    assert serial.open() == 0

    serial.close()
    assert serial.open() == 0

    eol = 2
    assert serial.send("ok") == 2 + eol
    assert serial.recv() == DEVICE_RECV_DATA

    assert serial.describe_status() != ""

