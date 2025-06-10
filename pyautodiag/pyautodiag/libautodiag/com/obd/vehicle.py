from ctypes import *
from pyautodiag.libautodiag.libloader import load_lib
from pyautodiag.libautodiag.buffer import Buffer, BufferList

lib = load_lib()

from ctypes import *

byte = c_ubyte

class ECU_OBDService(Structure):
    _fields_ = [
        ('current_dtc', POINTER(BufferList)),
        ('pending_dtc', POINTER(BufferList)),
        ('permanent_dtc', POINTER(BufferList)),
        ('current_data', POINTER(BufferList)),
        ('freeze_frame_data', POINTER(BufferList)),
        ('tests_results', POINTER(BufferList)),
        ('tests_results_other', POINTER(BufferList)),
        ('clear_dtc', POINTER(BufferList)),
        ('request_vehicle_information', POINTER(BufferList)),
        ('control_operation', POINTER(BufferList)),
        ('none', POINTER(BufferList)),
    ]

class ECU(Structure):
    _fields_ = [
        ('obd_data_buffer', POINTER(BufferList)),
        ('address', POINTER(Buffer)),
        ('name', c_char_p),
        ('obd_service', ECU_OBDService),
    ]

lib.vehicle_ecu_new.restype = POINTER(ECU)
lib.vehicle_ecu_free.argtypes = [POINTER(ECU)]
lib.vehicle_ecu_empty.argtypes = [POINTER(ECU)]

class Vehicle(Structure):
    _fields_ = [
        ('ecus', POINTER(POINTER(ECU))),
        ('ecus_len', c_int),
        ('obd_data_buffer', POINTER(BufferList)),
    ]
    
    def __new__(cls):
        lib.vehicle_new.restype = POINTER(Buffer)
        ptr = lib.vehicle_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def dump(self): lib.vehicle_dump(byref(self))
    def ecu_add(self, address_bytes: bytes):
        size = len(address_bytes)
        addr_array = (c_ubyte * size)(*address_bytes)
        return lib.vehicle_ecu_add(byref(self), addr_array, size).contents

    def ecu_add_if_not_in(self, address_bytes: bytes):
        size = len(address_bytes)
        addr_array = (c_ubyte * size)(*address_bytes)
        return lib.vehicle_ecu_add_if_not_in(byref(self), addr_array, size).contents

lib.vehicle_new.restype = POINTER(Vehicle)
lib.vehicle_free.argtypes = [POINTER(Vehicle)]

lib.vehicle_ecu_add.argtypes = [POINTER(Vehicle), POINTER(byte), c_int]
lib.vehicle_ecu_add.restype = POINTER(ECU)

lib.vehicle_ecu_add_if_not_in.argtypes = [POINTER(Vehicle), POINTER(byte), c_int]
lib.vehicle_ecu_add_if_not_in.restype = POINTER(ECU)

lib.vehicle_fill_global_obd_data_from_ecus.argtypes = [POINTER(Vehicle)]
lib.vehicle_dump.argtypes = [POINTER(Vehicle)]
