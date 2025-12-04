from autodiag.libloader import *

def inspect_class_memory(cls, base_offset=0, prefix=""):
    for name, ctype in cls._fields_:
        offset = base_offset + getattr(cls, name).offset
        size = sizeof(ctype)

        if issubclass(ctype, Structure):
            inspect_class_memory(ctype, offset, prefix + name + ".")
        else:
            print(f"{prefix + name} sz={size} offset={offset}")