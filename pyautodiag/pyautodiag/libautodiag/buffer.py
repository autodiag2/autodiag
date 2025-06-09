from ctypes import *
from pyautodiag.libautodiag.libloader import load_lib

lib = load_lib()

class Buffer(Structure):
    _fields_ = [
        ("size_allocated", c_int),
        ("size", c_int),
        ("buffer", POINTER(c_ubyte))
    ]
    
    def dump(self): lib.buffer_dump(self)
    def free(self): lib.buffer_free(self)
    def copy(self): return Buffer(lib.buffer_copy(self))
    def append(self, other): lib.buffer_append(self, other)
    def append_bytes(self, data): lib.buffer_append_bytes(self, (c_ubyte * len(data))(*data), len(data))
    def append_byte(self, b): lib.buffer_append_byte(self, b)
    def append_str(self, s): lib.buffer_append_str(self, c_char_p(s.encode()))
    def prepend(self, other): lib.buffer_prepend(self, other)
    def prepend_byte(self, b): lib.buffer_prepend_byte(self, b)
    def prepend_bytes(self, data): lib.buffer_prepend_bytes(self, (c_ubyte * len(data))(*data), len(data))
    def ensure_capacity(self, sz): return lib.buffer_ensure_capacity(self, sz)
    def ensure_termination(self): lib.buffer_ensure_termination(self)
    def equals(self, other): return lib.buffer_equals(self, other)
    def cmp(self, other): return lib.buffer_cmp(self, other)
    def padding(self, until, pad): lib.buffer_padding(self, until, pad)
    def to_ascii_hex(self): return lib.buffer_to_ascii_hex(self).decode()
    def to_ascii(self): return lib.buffer_to_ascii(self).decode()
    def to_string(self): return lib.buffer_to_string(self).decode()
    def get_free_space(self): return lib.buffer_get_free_space(self)
    def extract_first(self): return lib.buffer_extract_0(self)
    def get_free_space(self): return lib.buffer_get_free_space(self)
    def recycle(self): lib.buffer_recycle(self)

    @classmethod
    def from_ascii_hex(cls, s): return cls(lib.buffer_from_ascii_hex(s.encode()))
    @classmethod
    def from_ascii(cls, s): return cls(lib.buffer_from_ascii(s.encode()))
    @classmethod
    def from_string(cls, s): return cls(lib.buffer_from_string(s.encode()))
    @classmethod
    def new_random(cls, sz): return cls(lib.buffer_new_random(sz))
    @classmethod
    def new_random_with_seed(cls, sz, seed): 
        seed_ptr = pointer(c_uint(seed))
        return cls(lib.buffer_new_random_with_seed(sz, seed_ptr))
    @classmethod
    def new_cycle(cls, sz, percent): return cls(lib.buffer_new_cycle(sz, percent))

lib.buffer_new.restype = POINTER(Buffer)
lib.buffer_copy.argtypes = [POINTER(Buffer)]
lib.buffer_copy.restype = POINTER(Buffer)
lib.buffer_from_ascii_hex.argtypes = [c_char_p]
lib.buffer_from_ascii_hex.restype = POINTER(Buffer)
lib.buffer_from_ascii.argtypes = [c_char_p]
lib.buffer_from_ascii.restype = POINTER(Buffer)
lib.buffer_from_string.argtypes = [c_char_p]
lib.buffer_from_string.restype = POINTER(Buffer)
lib.buffer_to_ascii_hex.argtypes = [POINTER(Buffer)]
lib.buffer_to_ascii_hex.restype = c_char_p
lib.buffer_to_ascii.argtypes = [POINTER(Buffer)]
lib.buffer_to_ascii.restype = c_char_p
lib.buffer_to_string.argtypes = [POINTER(Buffer)]
lib.buffer_to_string.restype = c_char_p
lib.buffer_new_random.argtypes = [c_int]
lib.buffer_new_random.restype = POINTER(Buffer)
lib.buffer_new_random_with_seed.argtypes = [c_int, POINTER(c_uint)]
lib.buffer_new_random_with_seed.restype = POINTER(Buffer)
lib.buffer_new_cycle.argtypes = [c_int, c_int]
lib.buffer_new_cycle.restype = POINTER(Buffer)
lib.buffer_extract_0.argtypes = [POINTER(Buffer)]
lib.buffer_extract_0.restype = c_ubyte
lib.buffer_cmp.argtypes = [POINTER(Buffer), POINTER(Buffer)]
lib.buffer_cmp.restype = c_bool
lib.buffer_equals.argtypes = [POINTER(Buffer), POINTER(Buffer)]
lib.buffer_equals.restype = c_bool
lib.buffer_ensure_capacity.argtypes = [POINTER(Buffer), c_int]
lib.buffer_ensure_capacity.restype = c_bool
lib.buffer_get_free_space.argtypes = [POINTER(Buffer)]
lib.buffer_get_free_space.restype = c_int
lib.buffer_padding.argtypes = [POINTER(Buffer), c_int, c_ubyte]
lib.buffer_free.argtypes = [POINTER(Buffer)]   
lib.buffer_dump.argtypes = [POINTER(Buffer)]
lib.buffer_recycle.argtypes = [POINTER(Buffer)]