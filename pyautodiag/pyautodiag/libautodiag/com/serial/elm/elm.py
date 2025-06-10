from ctypes import *
from pyautodiag.libautodiag.libloader import load_lib
from pyautodiag.libautodiag.buffer import Buffer
from pyautodiag.libautodiag.com.serial.serial import Serial
from pyautodiag.libautodiag.com.obd.device import Device
from pyautodiag.libautodiag.com.obd.obd import OBDIFace
from pyautodiag.libautodiag.com.obd.vehicle import Vehicle
from pyautodiag.libautodiag.lib import addr

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
        iface_ptr = lib.elm_open_from_serial(byref(serial))
        if not iface_ptr:
            return None
        return iface_ptr.contents

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

# function not bound to an instance
lib.elm_guess_response.argtypes = [c_char_p]
lib.elm_guess_response.restype = c_int

lib.elm_linefeeds.argtypes = [POINTER(Serial), c_bool]
lib.elm_linefeeds.restype = c_int

lib.elm_echo.argtypes = [POINTER(Serial), c_bool]
lib.elm_echo.restype = c_int

lib.elm_print_id.argtypes = [POINTER(Serial)]
lib.elm_print_id.restype = c_char_p

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
