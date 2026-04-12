from autodiag.libloader import *
from autodiag.com.device import Device
from autodiag.lib import addr

GuessResponseFunc = CFUNCTYPE(c_int, char_p)

class SerialState(c_int):
    pass

class SerialImplementation(Structure):
    pass

class Serial(Structure):
    _fields_ = [
        ("device", Device),
        ("serial_state", SerialState),
        ("echo", bool),
        ("baud_rate", c_int),
        ("eol", char_p),
        ("timeout", c_int),
        ("timeout_seq", c_int),
        ("guess_response", GuessResponseFunc),
        ("implementation", POINTER(SerialImplementation))
    ]

    def __new__(cls):
        lib.ad_serial_new.restype = POINTER(Serial)
        ptr = lib.ad_serial_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def set_location(self, location):
        if isinstance(location, bytes):
            location = location.decode()
        if not isinstance(location, str):
            raise TypeError("location must be a string")
        self.device.location = location.encode()

    def init(self):
        lib.ad_serial_init.argtypes = [POINTER(Serial)]
        lib.ad_serial_init.restype = None
        lib.ad_serial_init(pointer(self))

    def open(self):
        lib.ad_serial_open.argtypes = [POINTER(Serial)]
        lib.ad_serial_open.restype = c_int
        return lib.ad_serial_open(pointer(self))

    def close(self):
        lib.ad_serial_close.argtypes = [POINTER(Serial)]
        lib.ad_serial_close.restype = None
        lib.ad_serial_close(pointer(self))

    def lock(self):
        lib.ad_serial_lock.argtypes = [POINTER(Serial)]
        lib.ad_serial_lock.restype = None
        lib.ad_serial_lock(pointer(self))

    def unlock(self):
        lib.ad_serial_unlock.argtypes = [POINTER(Serial)]
        lib.ad_serial_unlock.restype = None
        lib.ad_serial_unlock(pointer(self))

    def dump(self):
        lib.ad_serial_dump.argtypes = [POINTER(Serial)]
        lib.ad_serial_dump.restype = None
        lib.ad_serial_dump(pointer(self))

    def debug(self):
        lib.ad_serial_debug.argtypes = [POINTER(Serial)]
        lib.ad_serial_debug.restype = None
        lib.ad_serial_debug(pointer(self))

    def debug_from_python(self):
        print("Serial: {")
        self.device.debug_from_python()
        print(f"  echo: {self.echo}")
        print(f"  baud_rate: {self.baud_rate}")
        print(f"  eol: {self.eol.decode('utf-8') if self.eol else None}")
        print(f"  timeout: {self.timeout}")
        print(f"  timeout_seq: {self.timeout_seq}")
        print("}")

    def free(self):
        lib.ad_serial_free.argtypes = [POINTER(Serial)]
        lib.ad_serial_free.restype = None
        lib.ad_serial_free(pointer(self))

    def send(self, command: str) -> int:
        lib.ad_serial_send.argtypes = [POINTER(Serial), char_p]
        lib.ad_serial_send.restype = c_int
        return lib.ad_serial_send(pointer(self), command.encode('utf-8'))

    def recv(self) -> int:
        lib.ad_serial_recv.argtypes = [POINTER(Serial)]
        lib.ad_serial_recv.restype = c_int
        return lib.ad_serial_recv(pointer(self))

    def recv_internal(self) -> int:
        lib.ad_serial_recv_internal.argtypes = [POINTER(Serial)]
        lib.ad_serial_recv_internal.restype = c_int
        return lib.ad_serial_recv_internal(pointer(self))

    def describe_communication_layer(self) -> str:
        lib.ad_serial_describe_communication_layer.argtypes = [POINTER(Serial)]
        lib.ad_serial_describe_communication_layer.restype = char_p
        res = lib.ad_serial_describe_communication_layer(pointer(self))
        return res.decode('utf-8') if res else None
    
    def describe_state(self) -> str:
        lib.ad_serial_describe_state.argtypes = [POINTER(Serial)]
        lib.ad_serial_describe_state.restype = char_p
        res = lib.ad_serial_describe_state(pointer(self))
        return res.decode('utf-8') if res else None

    def query_at_command(self, cmd: str, *args) -> bool:
        lib.ad_serial_query_at_command.argtypes = [POINTER(Serial), char_p]
        lib.ad_serial_query_at_command.restype = bool
        command = cmd % args if args else cmd
        return lib.ad_serial_query_at_command(pointer(self), command.encode('utf-8'))

    def send_at_command(self, cmd: str, *args) -> bool:
        lib.ad_serial_send_at_command.argtypes = [POINTER(Serial), char_p]
        lib.ad_serial_send_at_command.restype = bool
        command = cmd % args if args else cmd
        return lib.ad_serial_send_at_command(pointer(self), command.encode('utf-8'))

    def reset_to_default(self):
        lib.ad_serial_reset_to_default.argtypes = [POINTER(Serial)]
        lib.ad_serial_reset_to_default.restype = None
        lib.ad_serial_reset_to_default(pointer(self))