from ctypes import POINTER, Structure, CFUNCTYPE, c_bool, c_char_p, c_int, c_void_p

from autodiag.buffer import Buffer
from autodiag.libloader import *

DeviceSendFunc = CFUNCTYPE(c_int, c_void_p, c_char_p)
DeviceRecvFunc = CFUNCTYPE(c_int, c_void_p)
DeviceOpenFunc = CFUNCTYPE(c_int, c_void_p)
DeviceCloseFunc = CFUNCTYPE(None, c_void_p)
DeviceDescribeCommunicationLayerFunc = CFUNCTYPE(c_char_p, c_void_p)
DeviceDescribeStateFunc = CFUNCTYPE(c_char_p, c_void_p)
DeviceParseFunc = CFUNCTYPE(c_bool, c_void_p, c_void_p)
DeviceSetFilterByAddressFunc = CFUNCTYPE(c_bool, c_void_p, c_void_p)
DeviceClearFunc = CFUNCTYPE(None, c_void_p)
DeviceLockFunc = CFUNCTYPE(None, c_void_p)
DeviceUnlockFunc = CFUNCTYPE(None, c_void_p)
DeviceFreeFunc = CFUNCTYPE(None, c_void_p)

AD_DEVICE_TYPE_AUTO = 0
AD_DEVICE_TYPE_SERIAL = 1
AD_DEVICE_TYPE_DOIP = 2

AD_DEVICE_STATE_UNDEFINED = 0
AD_DEVICE_STATE_READY = 1
AD_DEVICE_STATE_NOT_READY = 2
AD_DEVICE_STATE_DEVICE_SPECIFIC = 3

def _addr(x):
    if not x:
        return "None"
    try:
        return hex(cast(x, c_void_p).value)
    except:
        try:
            return hex(addressof(x.contents))
        except:
            return str(x)

class Device(Structure):
    _fields_ = [
        ("type", c_int),
        ("state", c_int),
        ("recv_buffer", POINTER(Buffer)),
        ("location", c_char_p),
        ("detected", c_bool),
        ("send", DeviceSendFunc),
        ("recv", DeviceRecvFunc),
        ("open", DeviceOpenFunc),
        ("close", DeviceCloseFunc),
        ("describe_communication_layer", DeviceDescribeCommunicationLayerFunc),
        ("describe_state", DeviceDescribeStateFunc),
        ("parse_data", DeviceParseFunc),
        ("set_filter_by_address", DeviceSetFilterByAddressFunc),
        ("clear_data", DeviceClearFunc),
        ("lock", DeviceLockFunc),
        ("unlock", DeviceUnlockFunc),
        ("free", DeviceFreeFunc),
    ]
    
    def debug_from_python(self):
        print("Device: {")
        print(f"    self: {hex(addressof(self))}")
        print(f"    type: {self.type}")
        print(f"    state: {self.state}")
        print(f"    recv_buffer: {_addr(self.recv_buffer)}")
        print(f"    location: {self.location.decode('utf-8') if self.location else 'None'}")
        print(f"    detected: {int(self.detected)}")
        print(f"    send: {_addr(self.send)}")
        print(f"    recv: {_addr(self.recv)}")
        print(f"    open: {_addr(self.open)}")
        print(f"    close: {_addr(self.close)}")
        print(f"    describe_communication_layer: {_addr(self.describe_communication_layer)}")
        print(f"    describe_state: {_addr(self.describe_state)}")
        print(f"    parse_data: {_addr(self.parse_data)}")
        print(f"    set_filter_by_address: {_addr(self.set_filter_by_address)}")
        print(f"    clear_data: {_addr(self.clear_data)}")
        print(f"    lock: {_addr(self.lock)}")
        print(f"    unlock: {_addr(self.unlock)}")
        print(f"    free: {_addr(self.free)}")
        print("}")

    def debug(self):
        lib.ad_device_debug.argtypes = [POINTER(Device)]
        lib.ad_device_debug.restype = None

DEVICE_ERROR = -1
DEVICE_RECV_DATA = -2
DEVICE_RECV_NULL = -3
DEVICE_RECV_DATA_UNAVAILABLE = -4