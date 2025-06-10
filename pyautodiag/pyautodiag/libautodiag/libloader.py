import os
from ctypes import CDLL
import platform

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
    return CDLL(lib_path)

