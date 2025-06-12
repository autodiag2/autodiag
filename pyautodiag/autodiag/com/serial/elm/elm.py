from ctypes import *
from autodiag.libloader import load_lib
from autodiag.buffer import Buffer
from autodiag.com.serial.serial import Serial
from autodiag.com.obd.device import Device
from autodiag.com.obd.obd import OBDIFace
from autodiag.com.obd.vehicle import Vehicle
from autodiag.lib import *
from autodiag.log import *

lib = load_lib()

class ELMDevice(Structure):
    _fields_ = [
        ('serial', Serial),
        ('printing_of_spaces', c_bool),
        ('configure', CFUNCTYPE(c_bool, POINTER(Device))),
    ]

    def guess_response(self, buffer: bytes) -> int:
        return lib.elm_guess_response(buffer)

    def linefeeds(self, state: bool) -> int:
        return lib.elm_linefeeds(byref(self.serial), state)

    def echo(self, state: bool) -> int:
        return lib.elm_echo(byref(self.serial), state)

    def print_id(self) -> str:
        return lib.elm_print_id(byref(self.serial)).decode()

    def ascii_to_bin(self, ascii_buf: Buffer) -> Buffer:
        return lib.elm_ascii_to_bin(byref(self), byref(ascii_buf)).contents

    def ascii_to_bin_str(self, ascii: bytes, end_ptr: bytes) -> Buffer:
        return lib.elm_ascii_to_bin_str(byref(self), ascii, end_ptr).contents

    def ascii_to_bin_internal(self, bin_buf: Buffer, ascii: bytes, end_ptr: bytes):
        lib.elm_ascii_to_bin_internal(self.printing_of_spaces, byref(bin_buf), ascii, end_ptr)

    def ascii_from_bin(self, bin_buf: Buffer) -> str:
        return lib.elm_ascii_from_bin(self.printing_of_spaces, byref(bin_buf)).decode()

    def standard_obd_message_parse_response(self, vehicle: Vehicle) -> bool:
        return lib.elm_standard_obd_message_parse_response(byref(self), byref(vehicle))
    
    @staticmethod
    def open_from_serial(serial: Serial) -> OBDIFace:
        if not serial:
            return None
        if lib.serial_open(byref(serial)) == GENERIC_FUNCTION_ERROR:
            log_msg(LOG_ERROR, "Error while opening OBD port")
            return None

        iface = ELMDevice.open_from_serial_internal(byref(serial))
        if iface is None:
            log_msg(LOG_DEBUG, "Configuration has failed, resetting and trying one more time")
            lib.serial_query_at_command(byref(serial), b"d")
            lib.serial_reset_to_default(byref(serial))
            iface = ELMDevice.open_from_serial_internal(byref(serial))
            if iface is None:
                log_msg(LOG_DEBUG, "Configuration has failed, device maybe not using AT&T default, trying one more time")
                lib.elm_echo(byref(serial), False)
                serial.echo = False
                lib.elm_linefeeds(byref(serial), False)
                serial.eol = b"\r"
                iface = ELMDevice.open_from_serial_internal2(byref(serial))
        if iface is None:
            log_msg(LOG_WARNING, "Everything has been tried but iface config has failed")
        return iface


    @staticmethod
    def open_from_serial_internal2(port: POINTER(Serial)) -> OBDIFace:
        iface = None
        response = lib.elm_print_id(port)
        interfaceConfigured = False
        if not response:
            interfaceConfigured = False
        else:
            resp_str = response.decode()

            if resp_str.startswith("ELM329"):
                iface = lib.obd_new_from_device(cast(lib.elm329_new_from_serial(port), POINTER(Device)))
            elif resp_str.startswith("ELM327"):
                iface = lib.obd_new_from_device(cast(lib.elm327_new_from_serial(port), POINTER(Device)))
            elif resp_str.startswith("ELM323"):
                iface = lib.obd_new_from_device(cast(lib.elm323_new_from_serial(port), POINTER(Device)))
            elif resp_str.startswith("ELM322"):
                iface = lib.obd_new_from_device(cast(lib.elm322_new_from_serial(port), POINTER(Device)))
            elif resp_str.startswith("ELM320"):
                iface = lib.obd_new_from_device(cast(lib.elm320_new_from_serial(port), POINTER(Device)))

            if iface is None:
                interfaceConfigured = False
            else:
                elm = cast(iface.contents.device, POINTER(ELMDevice)).contents
                interfaceConfigured = elm.configure(byref(iface.contents.device.contents))
                lib.free(port)

        log_msg(LOG_WARNING, "No updates to the serial list made")

        if interfaceConfigured:
            return iface.contents
        else:
            if iface:
                iface.contents.device = None
                lib.obd_free(iface)
            return None


    @staticmethod
    def open_from_serial_internal(port: POINTER(Serial)) -> OBDIFace:
        if lib.elm_echo(port, False) == DEVICE_ERROR:
            log_msg(LOG_ERROR, "Error while turn echo off")
            return None
        if lib.elm_linefeeds(port, False) == DEVICE_ERROR:
            log_msg(LOG_ERROR, "Error while turn line feeds off")
            return None

        return ELMDevice.open_from_serial_internal2(port)

    def debug(self):
        lib.elm_debug.argtypes = [POINTER(ELMDevice)]
        lib.elm_debug.restype = None
        lib.elm_debug(pointer(self))

    def debug_from_python(self):
        print("ElmDevice: {")
        self.serial.debug_from_python()
        print(f"  printing_of_spaces: {self.printing_of_spaces}")
        print(f"  configure: {addr(self.configure)}")
        print("}")

class ELM320Device(ELMDevice):
    _fields_ = [
        ('inheritance', ELMDevice)
    ]
class ELM322Device(ELMDevice):
    _fields_ = [
        ('inheritance', ELMDevice)
    ]
class ELM323Device(ELMDevice):
    _fields_ = [
        ('inheritance', ELMDevice)
    ]
class ELM327Device(ELMDevice):
    _fields_ = [
        ('inheritance', ELMDevice),
        ('protocol', c_int)
    ]
class ELM329Device(ELMDevice):
    _fields_ = [
        ('inheritance', ELMDevice),
        ('protocol', c_int)
    ]

# function not bound to an instance
lib.elm_guess_response.argtypes = [c_char_p]
lib.elm_guess_response.restype = c_int

lib.elm_linefeeds.argtypes = [POINTER(Serial), c_bool]
lib.elm_linefeeds.restype = c_int

lib.elm_echo.argtypes = [POINTER(Serial), c_bool]
lib.elm_echo.restype = c_int

lib.elm_print_id.argtypes = [POINTER(Serial)]
lib.elm_print_id.restype = c_char_p

lib.elm329_new_from_serial.argtypes = [POINTER(Serial)]
lib.elm329_new_from_serial.restype = POINTER(ELM329Device)

lib.elm327_new_from_serial.argtypes = [POINTER(Serial)]
lib.elm327_new_from_serial.restype = POINTER(ELM327Device)

lib.elm323_new_from_serial.argtypes = [POINTER(Serial)]
lib.elm323_new_from_serial.restype = POINTER(ELM323Device)

lib.elm322_new_from_serial.argtypes = [POINTER(Serial)]
lib.elm322_new_from_serial.restype = POINTER(ELM322Device)

lib.elm320_new_from_serial.argtypes = [POINTER(Serial)]
lib.elm320_new_from_serial.restype = POINTER(ELM320Device)

lib.elm_open_from_serial.argtypes = [POINTER(Serial)]
lib.elm_open_from_serial.restype = POINTER(OBDIFace)

lib.elm_ascii_to_bin.argtypes = [POINTER(ELMDevice), POINTER(Buffer)]
lib.elm_ascii_to_bin.restype = POINTER(Buffer)

lib.elm_ascii_to_bin_str.argtypes = [POINTER(ELMDevice), c_char_p, c_char_p]
lib.elm_ascii_to_bin_str.restype = POINTER(Buffer)

lib.elm_ascii_to_bin_internal.argtypes = [c_bool, POINTER(Buffer), c_char_p, c_char_p]
lib.elm_ascii_to_bin_internal.restype = None

lib.elm_ascii_from_bin.argtypes = [c_bool, POINTER(Buffer)]
lib.elm_ascii_from_bin.restype = c_char_p

lib.elm_standard_obd_message_parse_response.argtypes = [POINTER(ELMDevice), POINTER(Vehicle)]
lib.elm_standard_obd_message_parse_response.restype = c_bool

lib.obd_new_from_device.argtypes = [POINTER(Device)]
lib.obd_new_from_device.restype = POINTER(OBDIFace)