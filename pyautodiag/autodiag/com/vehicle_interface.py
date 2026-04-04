from ctypes import POINTER, Structure, byref, c_bool, c_int, c_uint64, c_void_p, cast

from autodiag.buffer import Buffer
from autodiag.com.device import Device
from autodiag.libloader import *
from autodiag.model.vehicle import Vehicle

VIFaceState_READY = 0
VIFaceState_NOT_READY = 1
VIFaceState_RECONNECTING = 2

VIFACE_RECV_ERROR = -1
VIFACE_RECV_UNSUPPORTED = -2

class UDSStruct(Structure):
    _fields_ = [
        ("enabled", c_bool),
        ("tester_present_timer", c_void_p),
    ]

class ConnectionCheckingStruct(Structure):
    _fields_ = [
        ("enabled", c_bool),
        ("last_activity_ms", c_uint64),
        ("activity_threshold_ms", c_uint64),
        ("activity_poll_ms", c_int),
        ("activity_thread", c_void_p),
        ("disable", c_void_p),
        ("update_last_activity", c_void_p),
        ("probe", c_void_p),
        ("should_probe", c_void_p),
        ("start", c_void_p),
        ("stop", c_void_p),
    ]

class ConnectionStruct(Structure):
    _fields_ = [
        ("checking", ConnectionCheckingStruct),
        ("_state", c_int),
        ("onConnectionStateChanged", c_void_p),
        ("set_state", c_void_p),
    ]

class InternalStruct(Structure):
    _fields_ = [
        ("onRequest", c_void_p),
        ("onResponse", c_void_p),
    ]

class VehicleIFace(Structure):
    _fields_ = [
        ("device", POINTER(Device)),
        ("vehicle", POINTER(Vehicle)),
        ("uds", UDSStruct),
        ("connection", ConnectionStruct),
        ("lock", c_void_p),
        ("unlock", c_void_p),
        ("send", c_void_p),
        ("recv", c_void_p),
        ("clear_data", c_void_p),
        ("internal", InternalStruct),
    ]

    def __new__(cls):
        ptr = lib.viface_new()
        if not ptr:
            raise MemoryError("Failed to create VehicleIFace")
        return ptr.contents

    @property
    def state(self) -> int:
        return self.connection._state

    def lock_iface(self):
        lib.viface_lock(byref(self))

    def unlock_iface(self):
        lib.viface_unlock(byref(self))

    def send(self, request: Buffer) -> int:
        return lib.viface_send(byref(self), byref(request))

    def send_str(self, request: str) -> int:
        return lib.viface_send_str(byref(self), request.encode())

    def recv_data(self) -> int:
        return lib.viface_recv(byref(self))

    def clear_data_iface(self):
        lib.viface_clear_data(byref(self))

    def close(self):
        lib.viface_close(byref(self))

    def discover_vehicle(self):
        lib.viface_discover_vehicle(byref(self))

    def free(self):
        lib.viface_free(byref(self))

    @staticmethod
    def open_from_device(device: Device):
        ptr = lib.viface_open_from_device(cast(byref(device), POINTER(Device)))
        if not ptr:
            return None
        return ptr.contents

lib.viface_new.argtypes = []
lib.viface_new.restype = POINTER(VehicleIFace)

lib.viface_free.argtypes = [POINTER(VehicleIFace)]
lib.viface_free.restype = None

lib.viface_lock.argtypes = [POINTER(VehicleIFace)]
lib.viface_lock.restype = None

lib.viface_unlock.argtypes = [POINTER(VehicleIFace)]
lib.viface_unlock.restype = None

lib.viface_close.argtypes = [POINTER(VehicleIFace)]
lib.viface_close.restype = None

lib.viface_send.argtypes = [POINTER(VehicleIFace), POINTER(Buffer)]
lib.viface_send.restype = c_int

lib.viface_send_str.argtypes = [POINTER(VehicleIFace), c_char_p]
lib.viface_send_str.restype = c_int

lib.viface_recv.argtypes = [POINTER(VehicleIFace)]
lib.viface_recv.restype = c_int

lib.viface_clear_data.argtypes = [POINTER(VehicleIFace)]
lib.viface_clear_data.restype = None

lib.viface_discover_vehicle.argtypes = [POINTER(VehicleIFace)]
lib.viface_discover_vehicle.restype = None

lib.viface_open_from_device.argtypes = [POINTER(Device)]
lib.viface_open_from_device.restype = POINTER(VehicleIFace)

lib.viface_open_from_iface_device.argtypes = [POINTER(VehicleIFace), POINTER(Device)]
lib.viface_open_from_iface_device.restype = c_bool

lib.viface_fill_infos_from_vin.argtypes = [POINTER(VehicleIFace)]
lib.viface_fill_infos_from_vin.restype = None

lib.viface_dump.argtypes = [POINTER(VehicleIFace)]
lib.viface_dump.restype = None

lib.viface_recv_filter_add.argtypes = [POINTER(VehicleIFace), POINTER(Buffer)]
lib.viface_recv_filter_add.restype = None

lib.viface_recv_filter_rm.argtypes = [POINTER(VehicleIFace), POINTER(Buffer)]
lib.viface_recv_filter_rm.restype = c_bool

lib.viface_recv_filter_clear.argtypes = [POINTER(VehicleIFace)]
lib.viface_recv_filter_clear.restype = None

lib.obd_standard_parse_buffer.argtypes = [POINTER(Vehicle), POINTER(Buffer)]
lib.obd_standard_parse_buffer.restype = c_bool

def standard_parse_buffer(vehicle: Vehicle, buffer: Buffer) -> bool:
    return lib.obd_standard_parse_buffer(byref(vehicle), byref(buffer))