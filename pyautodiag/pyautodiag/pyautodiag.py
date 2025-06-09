import os
from ctypes import CDLL

lib_path = os.path.join(os.path.dirname(__file__), "libautodiag.so")
lib = CDLL(lib_path)