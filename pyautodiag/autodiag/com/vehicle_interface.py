from autodiag.libloader import *
from autodiag.buffer import Buffer
from autodiag.model.vehicle import Vehicle
from autodiag.com.device import Device

class VehicleIFace(Structure):
    _fields_ = [
        ("device", POINTER(Device)),
        ("vehicle", POINTER(Vehicle)),
        ("state", c_int),
        ("uds_enabled", c_bool),
        ("uds_tester_present_timer", c_void_p)
    ]

    def __new__(cls):
        ptr = lib.viface_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def lock(self):
        lib.viface_lock(byref(self))

    def unlock(self):
        lib.viface_unlock(byref(self))

    def send(self, command: str) -> int:
        return lib.viface_send(byref(self), command.encode())

    def recv(self) -> int:
        return lib.viface_recv(byref(self))

    def clear_data(self):
        lib.viface_clear_data(byref(self))

    def close(self):
        lib.viface_close(byref(self))

    def discover_vehicle(self):
        lib.viface_discover_vehicle(byref(self))
    
    @staticmethod
    def open_from_device(device: Device) -> 'VehicleIFace':
        iface_ptr = lib.viface_open_from_device(cast(byref(device), POINTER(Device)))
        if not iface_ptr:
            return None
        iface = iface_ptr.contents
        return iface

lib.viface_new.argtypes = []
lib.viface_new.restype = POINTER(VehicleIFace)

lib.viface_open_from_device.argtypes = [POINTER(Device)]
lib.viface_open_from_device.restype = POINTER(VehicleIFace)

lib.viface_free.argtypes = [POINTER(VehicleIFace)]
lib.viface_lock.argtypes = [POINTER(VehicleIFace)]
lib.viface_unlock.argtypes = [POINTER(VehicleIFace)]
lib.viface_send.argtypes = [POINTER(VehicleIFace), c_char_p]
lib.viface_send.restype = c_int
lib.viface_recv.argtypes = [POINTER(VehicleIFace)]
lib.viface_recv.restype = c_int
lib.viface_clear_data.argtypes = [POINTER(VehicleIFace)]
lib.viface_close.argtypes = [POINTER(VehicleIFace)]
lib.viface_discover_vehicle.argtypes = [POINTER(VehicleIFace)]

lib.obd_standard_parse_buffer.argtypes = [POINTER(Vehicle), POINTER(Buffer)]
lib.obd_standard_parse_buffer.restype = c_bool

def standard_parse_buffer(vehicle: Vehicle, buffer: Buffer) -> bool:
    return lib.obd_standard_parse_buffer(byref(vehicle), byref(buffer))
