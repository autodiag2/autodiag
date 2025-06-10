from ctypes import *

def addr(f): return f"0x{cast(f, c_void_p).value:x}" if f else "None"