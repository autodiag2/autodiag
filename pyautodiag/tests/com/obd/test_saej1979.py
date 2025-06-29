
from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import *
from autodiag.com.obd.saej1979 import SAEJ1979
from autodiag.sim.elm327.sim_generators import *

def test_saej1979():

    temperature = 0
    @SimECUGenerator.CALLBACK_OBD_SIM_RESPONSE
    def custom_sim_ecu_generator_response(generator_ptr, response_ptr, binResponse, binRequest):
        hexString = binRequest.contents.to_hex_string()
        print(hexString)
        if hexString == "0105":
            binResponse.contents.append_byte(temperature + 40)
        elif hexString == "0101":
            # mil on, 2 dtcs
            A = 0b10000010
            binResponse.contents.append_byte(A)
            # Compression ingnition, first and last test present but only the first is complete
            B = 0b01001101
            binResponse.contents.append_byte(B)
            # No engine specific tests
            C = 0b00000000
            binResponse.contents.append_byte(C)
            D = 0b00000000
            binResponse.contents.append_byte(D)
        else:
            response_ptr[0] = c_char_p(b"OK")

    class CustomECUGenerator(SimECUGenerator):
        def __init__(self):
            self.context = None
            self.sim_ecu_generator_response = custom_sim_ecu_generator_response
            self.type = "custom".encode()
            
    log_set_level(LOG_DEBUG)
    emulation = SimELM327()
    emulation.set_ecu_and_generator(0xE8, CustomECUGenerator())
    emulation.loop(daemon=True)

    import time
    time.sleep(2)

    serial = Serial()
    serial.set_location(emulation.device_location)
    iface = OBDIFace.open_from_device(serial)

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
