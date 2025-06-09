import os
from ctypes import CDLL

def load_lib():
    here = os.path.dirname(os.path.abspath(__file__))
    so_path = os.path.abspath(os.path.join(here, "libautodiag.so"))
    return CDLL(so_path)

