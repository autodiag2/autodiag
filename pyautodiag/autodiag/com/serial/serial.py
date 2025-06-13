from autodiag.libloader import *
from autodiag.buffer import Buffer
from autodiag.com.obd.device import Device
from autodiag.lib import addr

GuessResponseFunc = CFUNCTYPE(c_int, char_p)

class SerialStatus(c_int):
    pass

class SerialImplementation(Structure):
    pass

class Serial(Structure):
    _fields_ = [
        ("device", Device),
        ("echo", bool),
        ("baud_rate", c_int),
        ("status", SerialStatus),
        ("name", char_p),
        ("eol", char_p),
        ("timeout", c_int),
        ("timeout_seq", c_int),
        ("recv_buffer", POINTER(Buffer)),
        ("detected", bool),
        ("guess_response", GuessResponseFunc),
        ("implementation", POINTER(SerialImplementation))
    ]

    def __new__(cls):
        lib.serial_new.restype = POINTER(Serial)
        ptr = lib.serial_new()
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
        self.name = location.encode('utf-8')

    def init(self):
        lib.serial_init.argtypes = [POINTER(Serial)]
        lib.serial_init.restype = None
        lib.serial_init(pointer(self))

    def open(self):
        lib.serial_open.argtypes = [POINTER(Serial)]
        lib.serial_open.restype = c_int
        return lib.serial_open(pointer(self))

    def close(self):
        lib.serial_close.argtypes = [POINTER(Serial)]
        lib.serial_close.restype = None
        lib.serial_close(pointer(self))

    def lock(self):
        lib.serial_lock.argtypes = [POINTER(Serial)]
        lib.serial_lock.restype = None
        lib.serial_lock(pointer(self))

    def unlock(self):
        lib.serial_unlock.argtypes = [POINTER(Serial)]
        lib.serial_unlock.restype = None
        lib.serial_unlock(pointer(self))

    def dump(self):
        lib.serial_dump.argtypes = [POINTER(Serial)]
        lib.serial_dump.restype = None
        lib.serial_dump(pointer(self))

    def debug(self):
        lib.serial_debug.argtypes = [POINTER(Serial)]
        lib.serial_debug.restype = None
        lib.serial_debug(pointer(self))

    def debug_from_python(self):
        print("Serial: {")
        print("  Device:{")
        print(f"    send: {addr(self.device.send)}")
        print(f"    recv: {addr(self.device.recv)}")
        print(f"    open: {addr(self.device.open)}")
        print(f"    close: {addr(self.device.close)}")
        print(f"    describe_communication_layer: {addr(self.device.describe_communication_layer)}")
        print(f"    parse_data: {addr(self.device.parse_data)}")
        print(f"    clear_data: {addr(self.device.clear_data)}")
        print(f"    lock: {addr(self.device.lock)}")
        print(f"    unlock: {addr(self.device.unlock)}")
        print("  }")
        print(f"  echo: {self.echo}")
        print(f"  baud_rate: {self.baud_rate}")
        print(f"  status: {self.status}")
        print(f"  name: {self.name.decode('utf-8') if self.name else None}")
        print(f"  eol: {self.eol.decode('utf-8') if self.eol else None}")
        print(f"  timeout: {self.timeout}")
        print(f"  timeout_seq: {self.timeout_seq}")
        if self.recv_buffer:
            print(f"  recv_buffer: {self.recv_buffer.contents}")
        else:
            print("  recv_buffer: None")
        print(f"  detected: {self.detected}")
        if self.guess_response:
            print(f"  guess_response: {addr(self.guess_response)}")
        else:
            print("  guess_response: None")
        print("}")

    def free(self):
        lib.serial_free.argtypes = [POINTER(Serial)]
        lib.serial_free.restype = None
        lib.serial_free(pointer(self))

    def send(self, command: str) -> int:
        lib.serial_send.argtypes = [POINTER(Serial), char_p]
        lib.serial_send.restype = c_int
        return lib.serial_send(pointer(self), command.encode('utf-8'))

    def recv(self) -> int:
        lib.serial_recv.argtypes = [POINTER(Serial)]
        lib.serial_recv.restype = c_int
        return lib.serial_recv(pointer(self))

    def recv_internal(self) -> int:
        lib.serial_recv_internal.argtypes = [POINTER(Serial)]
        lib.serial_recv_internal.restype = c_int
        return lib.serial_recv_internal(pointer(self))
    
    def describe_status(self) -> str:
        lib.serial_describe_status.argtypes = [POINTER(Serial)]
        lib.serial_describe_status.restype = char_p
        return lib.serial_describe_status(pointer(self)).decode('utf-8')

    @staticmethod
    def strip_char(buffer: Buffer, char_to_strip: str):
        lib.serial_strip_char.argtypes = [POINTER(Buffer), char_p]
        lib.serial_strip_char.restype = None
        lib.serial_strip_char(pointer(buffer), char_to_strip.encode('utf-8'))

    @staticmethod
    def strip_char_internal(buffer: Buffer, char_to_strip: str, start: bool, end: bool):
        lib.serial_strip_char_internal.argtypes = [POINTER(Buffer), char_p, bool, bool]
        lib.serial_strip_char_internal.restype = None
        lib.serial_strip_char_internal(pointer(buffer), char_to_strip.encode('utf-8'), start, end)

    @staticmethod
    def status_to_string(status: SerialStatus) -> str:
        lib.serial_status_to_string.argtypes = [SerialStatus]
        lib.serial_status_to_string.restype = char_p
        res = lib.serial_status_to_string(status)
        return res.decode('utf-8') if res else None

    @staticmethod
    def at_command(fmt: str, *args) -> str:
        lib.at_command.argtypes = [char_p]
        lib.at_command.restype = char_p
        cmd = fmt % args
        res = lib.at_command(cmd.encode('utf-8'))
        return res.decode('utf-8') if res else None

    @staticmethod
    def at_command_boolean(cmd: str, state: bool) -> bool:
        lib.at_command_boolean.argtypes = [char_p, bool]
        lib.at_command_boolean.restype = bool
        return lib.at_command_boolean(cmd.encode('utf-8'), state)

    @staticmethod
    def is_command(command: str) -> bool:
        lib.at_is_command.argtypes = [char_p]
        lib.at_is_command.restype = bool
        return lib.at_is_command(command.encode('utf-8'))

    def describe_communication_layer(self) -> str:
        lib.serial_describe_communication_layer.argtypes = [POINTER(Serial)]
        lib.serial_describe_communication_layer.restype = char_p
        res = lib.serial_describe_communication_layer(pointer(self))
        return res.decode('utf-8') if res else None

    def query_at_command(self, cmd: str, *args) -> bool:
        lib.serial_query_at_command.argtypes = [POINTER(Serial), char_p]
        lib.serial_query_at_command.restype = bool
        command = cmd % args if args else cmd
        return lib.serial_query_at_command(pointer(self), command.encode('utf-8'))

    def send_at_command(self, cmd: str, *args) -> bool:
        lib.serial_send_at_command.argtypes = [POINTER(Serial), char_p]
        lib.serial_send_at_command.restype = bool
        command = cmd % args if args else cmd
        return lib.serial_send_at_command(pointer(self), command.encode('utf-8'))

    def reset_to_default(self):
        lib.serial_reset_to_default.argtypes = [POINTER(Serial)]
        lib.serial_reset_to_default.restype = None
        lib.serial_reset_to_default(pointer(self))

    @staticmethod
    def at_command_search(string: str, atcmd: str) -> bool:
        lib.serial_at_command_search.argtypes = [char_p, char_p]
        lib.serial_at_command_search.restype = bool
        return lib.serial_at_command_search(string.encode('utf-8'), atcmd.encode('utf-8'))

    @staticmethod
    def at_index_end(string: str, atcmd: str) -> int:
        lib.serial_at_index_end.argtypes = [char_p, char_p]
        lib.serial_at_index_end.restype = c_int
        return lib.serial_at_index_end(string.encode('utf-8'), atcmd.encode('utf-8'))
