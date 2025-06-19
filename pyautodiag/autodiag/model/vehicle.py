from autodiag.libloader import *
from autodiag.buffer import Buffer, Buffer_list
from autodiag.lib import *

class ECU_OBDService(Structure):
    _fields_ = [
        ('current_dtc', POINTER(Buffer_list)),
        ('pending_dtc', POINTER(Buffer_list)),
        ('permanent_dtc', POINTER(Buffer_list)),
        ('current_data', POINTER(Buffer_list)),
        ('freeze_frame_data', POINTER(Buffer_list)),
        ('tests_results', POINTER(Buffer_list)),
        ('tests_results_other', POINTER(Buffer_list)),
        ('clear_dtc', POINTER(Buffer_list)),
        ('request_vehicle_information', POINTER(Buffer_list)),
        ('control_operation', POINTER(Buffer_list)),
        ('none', POINTER(Buffer_list)),
    ]

class ECU(Structure):
    _fields_ = [
        ('obd_data_buffer', POINTER(Buffer_list)),
        ('address', POINTER(Buffer)),
        ('name', c_char_p),
        ('model', c_char_p),
        ('obd_service', ECU_OBDService),
    ]
    def debug_from_python(self):
        print("ECU debug: {")
        print(f"  address: {addr(self.address)}")
        print(f"  name: {self.name.decode('utf-8') if self.name else 'None'}")
        print(f"  obd_data_buffer: {addr(self.obd_data_buffer)}")
        print("  obd_service: {")
        for field_name, _ in ECU_OBDService._fields_:
            field_value = getattr(self.obd_service, field_name)
            print(f"    {field_name}: {addr(field_value) if field_value else 'None'}")
        print("  }")
        print("}")

lib.vehicle_ecu_new.restype = POINTER(ECU)
lib.vehicle_ecu_free.argtypes = [POINTER(ECU)]
lib.vehicle_ecu_empty.argtypes = [POINTER(ECU)]

class VehicleInternal(Structure):
    _fields_ = [
        ('directory', c_char_p)
    ]

class Vehicle(Structure):
    _fields_ = [
        ('ecus', POINTER(POINTER(ECU))),
        ('ecus_len', c_int),
        ('obd_data_buffer', POINTER(Buffer_list)),
        ('vin', POINTER(Buffer)),
        ('country', c_char_p),
        ('manufacturer', c_char_p),
        ('year', c_int),
        ('engine', c_char_p),
        ('internal', VehicleInternal)
    ]
    
    def __new__(cls):
        lib.vehicle_new.restype = POINTER(Buffer)
        ptr = lib.vehicle_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def dump(self): lib.vehicle_dump(byref(self))
    def debug(self): lib.vehicle_debug(byref(self))
    def debug_from_python(self):
        print("Vehicle debug: {")
        print(f"  ecus: {addr(self.ecus)}")
        print(f"  ecus_len: {self.ecus_len}")
        for i in range(self.ecus_len):
            ecu = self.ecus[i]
            if ecu:
                ecu.contents.debug_from_python()
        print(f"  obd_data_buffer: {addr(self.obd_data_buffer)}")
        print("}")
    def ecu_add(self, address_bytes: bytes):
        size = len(address_bytes)
        addr_array = (c_ubyte * size)(*address_bytes)
        return lib.vehicle_ecu_add(byref(self), addr_array, size).contents

    def ecu_add_if_not_in(self, address_bytes: bytes):
        size = len(address_bytes)
        addr_array = (c_ubyte * size)(*address_bytes)
        return lib.vehicle_ecu_add_if_not_in(byref(self), addr_array, size).contents

lib.vehicle_new.restype = POINTER(Vehicle)
lib.vehicle_free.argtypes = [POINTER(Vehicle)]

lib.vehicle_ecu_add.argtypes = [POINTER(Vehicle), POINTER(byte), c_int]
lib.vehicle_ecu_add.restype = POINTER(ECU)

lib.vehicle_ecu_add_if_not_in.argtypes = [POINTER(Vehicle), POINTER(byte), c_int]
lib.vehicle_ecu_add_if_not_in.restype = POINTER(ECU)

lib.vehicle_fill_global_obd_data_from_ecus.argtypes = [POINTER(Vehicle)]
lib.vehicle_dump.argtypes = [POINTER(Vehicle)]
lib.vehicle_debug.argtypes = [POINTER(Vehicle)]
