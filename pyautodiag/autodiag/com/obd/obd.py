from autodiag.libloader import *
from autodiag.buffer import Buffer
from autodiag.model.vehicle import Vehicle
from autodiag.com.device import Device

class OBDIFace(Structure):
    _fields_ = [
        ("device", POINTER(Device)),
        ("vehicle", POINTER(Vehicle)),
    ]

    def __new__(cls, device=None):
        if device is None:
            ptr = lib.obd_new()
        else:
            ptr = lib.obd_new_from_device(byref(device))
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def lock(self):
        lib.obd_lock(byref(self))

    def unlock(self):
        lib.obd_unlock(byref(self))

    def send(self, command: str) -> int:
        return lib.obd_send(byref(self), command.encode())

    def recv(self) -> int:
        return lib.obd_recv(byref(self))

    def clear_data(self):
        lib.obd_clear_data(byref(self))

    def close(self):
        lib.obd_close(byref(self))

    def discover_vehicle(self):
        lib.obd_discover_vehicle(byref(self))
    
    @staticmethod
    def open_from_device(device: Device) -> 'OBDIFace':
        iface_ptr = lib.obd_open_from_device(cast(byref(device), POINTER(Device)))
        if not iface_ptr:
            return None
        iface = iface_ptr.contents
        return iface

lib.obd_new.argtypes = []
lib.obd_new.restype = POINTER(OBDIFace)

lib.obd_new_from_device.argtypes = [POINTER(Device)]
lib.obd_new_from_device.restype = POINTER(OBDIFace)

lib.obd_open_from_device.argtypes = [POINTER(Device)]
lib.obd_open_from_device.restype = POINTER(OBDIFace)

lib.obd_free.argtypes = [POINTER(OBDIFace)]
lib.obd_lock.argtypes = [POINTER(OBDIFace)]
lib.obd_unlock.argtypes = [POINTER(OBDIFace)]
lib.obd_send.argtypes = [POINTER(OBDIFace), c_char_p]
lib.obd_send.restype = c_int
lib.obd_recv.argtypes = [POINTER(OBDIFace)]
lib.obd_recv.restype = c_int
lib.obd_clear_data.argtypes = [POINTER(OBDIFace)]
lib.obd_close.argtypes = [POINTER(OBDIFace)]
lib.obd_discover_vehicle.argtypes = [POINTER(OBDIFace)]

lib.obd_standard_parse_buffer.argtypes = [POINTER(Vehicle), POINTER(Buffer)]
lib.obd_standard_parse_buffer.restype = c_bool

def standard_parse_buffer(vehicle: Vehicle, buffer: Buffer) -> bool:
    return lib.obd_standard_parse_buffer(byref(vehicle), byref(buffer))
