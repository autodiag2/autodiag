from autodiag.libloader import *

class Buffer(Structure):
    _fields_ = [
        ("size_allocated", c_int),
        ("size", c_int),
        ("buffer", POINTER(c_ubyte))
    ]

    def __new__(cls):
        lib.buffer_new.restype = POINTER(Buffer)
        ptr = lib.buffer_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def dump(self): lib.buffer_dump(byref(self))
    def free(self): lib.buffer_free(byref(self))
    def copy(self): return lib.buffer_copy(byref(self)).contents
    def append(self, other): lib.buffer_append(byref(self), byref(other))
    def append_bytes(self, data): lib.buffer_append_bytes(byref(self), (c_ubyte * len(data))(*data), len(data))
    def append_byte(self, b): lib.buffer_append_byte(byref(self), b)
    def append_str(self, s): lib.buffer_append_str(byref(self), c_char_p(s.encode()))
    def prepend(self, other): lib.buffer_prepend(byref(self), byref(other))
    def prepend_byte(self, b): lib.buffer_prepend_byte(byref(self), b)
    def prepend_bytes(self, data): lib.buffer_prepend_bytes(byref(self), (c_ubyte * len(data))(*data), len(data))
    def ensure_capacity(self, sz): return lib.buffer_ensure_capacity(byref(self), sz)
    def ensure_termination(self): lib.buffer_ensure_termination(byref(self))
    def equals(self, other): return lib.buffer_equals(byref(self), byref(other))
    def cmp(self, other): return lib.buffer_cmp(byref(self), byref(other))
    def padding(self, until, pad): lib.buffer_padding(byref(self), until, pad)
    def to_hex_string(self): return lib.buffer_to_hex_string(byref(self)).decode()
    def to_ascii(self): return lib.buffer_to_ascii(byref(self)).decode()
    def get_free_space(self): return lib.buffer_get_free_space(byref(self))
    def extract_first(self): return lib.buffer_extract_0(byref(self))
    def recycle(self): lib.buffer_recycle(byref(self))

    @classmethod
    def from_ascii_hex(cls, s):
        ptr = lib.buffer_from_ascii_hex(s.encode())
        return ptr.contents
    @classmethod
    def from_ascii(cls, s):
        ptr = lib.buffer_from_ascii(s.encode())
        return ptr.contents
    @classmethod
    def new_random(cls, sz):
        ptr = lib.buffer_new_random(sz)
        return ptr.contents
    @classmethod
    def new_random_with_seed(cls, sz, seed):
        seed_c = c_uint(seed)
        ptr = lib.buffer_new_random_with_seed(sz, byref(seed_c))
        return ptr.contents
    @classmethod
    def new_cycle(cls, sz, percent):
        ptr = lib.buffer_new_cycle(sz, percent)
        return ptr.contents

lib.buffer_new.argtypes = []
lib.buffer_new.restype = POINTER(Buffer)
lib.buffer_copy.argtypes = [POINTER(Buffer)]
lib.buffer_copy.restype = POINTER(Buffer)
lib.buffer_from_ascii_hex.argtypes = [c_char_p]
lib.buffer_from_ascii_hex.restype = POINTER(Buffer)
lib.buffer_from_ascii.argtypes = [c_char_p]
lib.buffer_from_ascii.restype = POINTER(Buffer)
lib.buffer_to_hex_string.argtypes = [POINTER(Buffer)]
lib.buffer_to_hex_string.restype = c_char_p
lib.buffer_to_ascii.argtypes = [POINTER(Buffer)]
lib.buffer_to_ascii.restype = c_char_p
lib.buffer_new_random.argtypes = [c_int]
lib.buffer_new_random.restype = POINTER(Buffer)
lib.buffer_new_random_with_seed.argtypes = [c_int, POINTER(c_uint)]
lib.buffer_new_random_with_seed.restype = POINTER(Buffer)
lib.buffer_new_cycle.argtypes = [c_int, c_int]
lib.buffer_new_cycle.restype = POINTER(Buffer)
lib.buffer_extract_0.argtypes = [POINTER(Buffer)]
lib.buffer_extract_0.restype = c_ubyte
lib.buffer_cmp.argtypes = [POINTER(Buffer), POINTER(Buffer)]
lib.buffer_cmp.restype = c_int
lib.buffer_equals.argtypes = [POINTER(Buffer), POINTER(Buffer)]
lib.buffer_equals.restype = bool
lib.buffer_ensure_capacity.argtypes = [POINTER(Buffer), c_int]
lib.buffer_ensure_capacity.restype = bool
lib.buffer_get_free_space.argtypes = [POINTER(Buffer)]
lib.buffer_get_free_space.restype = c_int
lib.buffer_padding.argtypes = [POINTER(Buffer), c_int, c_ubyte]
lib.buffer_free.argtypes = [POINTER(Buffer)]   
lib.buffer_dump.argtypes = [POINTER(Buffer)]
lib.buffer_recycle.argtypes = [POINTER(Buffer)]

class list_Buffer(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(POINTER(Buffer)))
    ]

    def empty(self): lib.list_Buffer_empty(byref(self))
    def dump(self): lib.list_Buffer_dump(byref(self))

lib.list_Buffer_empty.argtypes = [POINTER(list_Buffer)]
lib.list_Buffer_dump.argtypes = [POINTER(list_Buffer)]