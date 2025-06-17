import os
from ctypes import CDLL
import platform
from ctypes import *

byte = c_ubyte
char_p = c_char_p
bool = c_int
c_int_p = POINTER(c_int)
c_byte_p = c_char_p

def load_lib():
    def get_lib_path():
        base_dir = os.path.join(os.path.dirname(__file__), "libs")
        system = platform.system().lower()
        machine = platform.machine().lower()

        if system == "windows":
            ext = "dll"
        elif system == "darwin":
            ext = "dylib"
        else:
            ext = "so"

        lib_name = f"libautodiag-{system}-{machine}.{ext}"
        return os.path.join(base_dir, lib_name)
    

    lib_path = get_lib_path()
    lib = CDLL(lib_path)

    installation_folder_resolve = os.path.join(os.path.dirname(__file__), "data")
    lib.installation_folder_resolve_override.argtypes = [c_char_p]
    lib.installation_folder_resolve_override(installation_folder_resolve.encode())

    return lib

lib = load_lib()