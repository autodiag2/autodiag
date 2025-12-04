
from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.vehicle_interface import VehicleIFace
from autodiag.log import *
from autodiag.com.obd.saej1979 import SAEJ1979
from autodiag.sim.ecu.generator import *

def test_saej1979():
    temperature = 0
    @Generator.CALLBACK_SIM_ECU_RESPONSE
    def custom_sim_ecu_generator_response(generator_ptr, binRequest_ptr, binResponse_ptr):
        binResponse = binResponse_ptr.contents
        hexString = binRequest_ptr.contents.to_hex_string()
        print(hexString, hexString == "0100", hexString == "0105", hexString == "0101")
        if hexString == "0105":
            binResponse.append_byte(0x41)
            binResponse.append_byte(0x05)
            binResponse.append_byte(temperature + 40)
        elif hexString == "0100":
            binResponse.append_byte(0x41)
            binResponse.append_byte(0x00)
            binResponse.append_byte(0xFF)
            binResponse.append_byte(0xFF)
            binResponse.append_byte(0xFF)
            binResponse.append_byte(0xFF)
        elif hexString == "0101":
            binResponse.append_byte(0x41)
            binResponse.append_byte(0x01)
            # mil on, 2 dtcs
            A = 0b10000010
            binResponse.append_byte(A)
            # Compression ingnition, first and last test present but only the first is complete
            B = 0b01001101
            binResponse.append_byte(B)
            # No engine specific tests
            C = 0b00000000
            binResponse.append_byte(C)
            D = 0b00000000
            binResponse.append_byte(D)
        else:
            # NO DATA
            pass

    class CustomECUGenerator(Generator):
        def __init__(self):
            self.context = None
            self.response = custom_sim_ecu_generator_response
            self.type = "custom".encode()
            
    log_set_level(LOG_DEBUG)
    emulation = SimELM327()
    emulation.set_ecu_and_generator(0xE8, CustomECUGenerator())
    emulation.loop(daemon=True)

    serial = Serial()
    serial.set_location(emulation.device_location)
    iface = VehicleIFace.open_from_device(serial)

    saej1979 = SAEJ1979(iface)
    assert saej1979.coolant_temp() == 0
    temperature = 2
    assert saej1979.coolant_temp() == 2
    temperature = 50
    assert saej1979.coolant_temp() == 50

    assert saej1979.number_of_dtc() == 2
    assert saej1979.mil_status() == True
    assert "ompression" in saej1979.engine_type_str()

    tests = saej1979.tests()
    test = tests.contents.list[0].contents
    print(f"test {test.name.decode()} is {test.completed}")
    assert test.completed == 1
    test = tests.contents.list[1].contents
    print(f"test {test.name.decode()} is {test.completed}")
    assert test.completed == 0
