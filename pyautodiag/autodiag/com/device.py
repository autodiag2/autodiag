from autodiag.libloader import *

DeviceSendFunc = CFUNCTYPE(c_int, c_void_p, c_char_p)
DeviceRecvFunc = CFUNCTYPE(c_int, c_void_p)
DeviceOpenFunc = CFUNCTYPE(None, c_void_p)
DeviceCloseFunc = CFUNCTYPE(None, c_void_p)
DeviceDescribeFunc = CFUNCTYPE(c_char_p, c_void_p)
DeviceParseFunc = CFUNCTYPE(c_bool, c_void_p, c_void_p)  # Vehicle* as c_void_p
DeviceClearFunc = CFUNCTYPE(None, c_void_p)
DeviceLockFunc = CFUNCTYPE(None, c_void_p)
DeviceUnlockFunc = CFUNCTYPE(None, c_void_p)

class Device(Structure):
    _fields_ = [
        ("type", c_char_p),
        ("send", DeviceSendFunc),
        ("recv", DeviceRecvFunc),
        ("open", DeviceOpenFunc),
        ("close", DeviceCloseFunc),
        ("describe_communication_layer", DeviceDescribeFunc),
        ("parse_data", DeviceParseFunc),
        ("clear_data", DeviceClearFunc),
        ("lock", DeviceLockFunc),
        ("unlock", DeviceUnlockFunc),
    ]

DEVICE_ERROR = -1
DEVICE_RECV_DATA = -2
DEVICE_RECV_NULL = -3
DEVICE_RECV_DATA_UNAVAILABLE = -4