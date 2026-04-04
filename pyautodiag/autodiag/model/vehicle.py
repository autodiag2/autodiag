from ctypes import POINTER, Structure, byref, c_char_p, c_int, c_void_p, cast

from autodiag.buffer import Buffer, list_Buffer
from autodiag.lib import *
from autodiag.libloader import *

VEHICLE_YEAR_EMPTY = -1

class ECU_OBDService(Structure):
    _fields_ = [
        ("current_dtc", POINTER(list_Buffer)),
        ("pending_dtc", POINTER(list_Buffer)),
        ("permanent_dtc", POINTER(list_Buffer)),
        ("current_data", POINTER(list_Buffer)),
        ("freeze_frame_data", POINTER(list_Buffer)),
        ("tests_results", POINTER(list_Buffer)),
        ("tests_results_other", POINTER(list_Buffer)),
        ("clear_dtc", POINTER(list_Buffer)),
        ("request_vehicle_information", POINTER(list_Buffer)),
        ("control_operation", POINTER(list_Buffer)),
        ("none", POINTER(list_Buffer)),
    ]

class ECU(Structure):
    _fields_ = [
        ("data_buffer", POINTER(list_Buffer)),
        ("address", POINTER(Buffer)),
        ("name", c_char_p),
        ("manufacturer", c_char_p),
        ("model", c_char_p),
        ("type", c_char_p),
        ("obd_service", ECU_OBDService),
    ]

    def debug_from_python(self):
        print("ECU debug: {")
        print(f"  address: {addr(self.address)}")
        print(f"  name: {self.name.decode('utf-8') if self.name else 'None'}")
        print(f"  manufacturer: {self.manufacturer.decode('utf-8') if self.manufacturer else 'None'}")
        print(f"  model: {self.model.decode('utf-8') if self.model else 'None'}")
        print(f"  type: {self.type.decode('utf-8') if self.type else 'None'}")
        print(f"  data_buffer: {addr(self.data_buffer)}")
        print("  obd_service: {")
        for field_name, _ in ECU_OBDService._fields_:
            field_value = getattr(self.obd_service, field_name)
            print(f"    {field_name}: {addr(field_value) if field_value else 'None'}")
        print("  }")
        print("}")

class VehicleEvents(Structure):
    _fields_ = [
        ("onECURegister", c_void_p),
        ("onFilterChange", c_void_p),
    ]

class VehicleInternal(Structure):
    _fields_ = [
        ("events", VehicleEvents),
        ("filter", POINTER(list_Buffer)),
    ]

class Vehicle(Structure):
    _fields_ = [
        ("ecus", c_void_p),
        ("data_buffer", POINTER(list_Buffer)),
        ("vin", POINTER(Buffer)),
        ("country", c_char_p),
        ("manufacturer", c_char_p),
        ("model", c_char_p),
        ("year", c_int),
        ("engine", c_char_p),
        ("engine_manufacturer", c_char_p),
        ("internal", VehicleInternal),
    ]

    def __new__(cls):
        ptr = lib.vehicle_new()
        if not ptr:
            raise MemoryError("Failed to create Vehicle instance")
        return ptr.contents

    def dump(self):
        lib.vehicle_dump(byref(self))

    def debug(self):
        lib.vehicle_debug(byref(self))

    def debug_from_python(self):
        print("Vehicle debug: {")
        print(f"  ecus: {addr(self.ecus)}")
        print(f"  data_buffer: {addr(self.data_buffer)}")
        print(f"  vin: {addr(self.vin)}")
        print(f"  country: {self.country.decode('utf-8') if self.country else 'None'}")
        print(f"  manufacturer: {self.manufacturer.decode('utf-8') if self.manufacturer else 'None'}")
        print(f"  model: {self.model.decode('utf-8') if self.model else 'None'}")
        print(f"  year: {self.year}")
        print(f"  engine: {self.engine.decode('utf-8') if self.engine else 'None'}")
        print(f"  engine_manufacturer: {self.engine_manufacturer.decode('utf-8') if self.engine_manufacturer else 'None'}")
        print(f"  filter: {addr(self.internal.filter)}")
        print("}")

    def ecu_add(self, address_bytes: bytes):
        size = len(address_bytes)
        addr_array = (byte * size)(*address_bytes)
        ptr = lib.vehicle_ecu_add(byref(self), addr_array, size)
        if not ptr:
            return None
        return ptr.contents

    def ecu_add_if_not_in(self, address_bytes: bytes):
        size = len(address_bytes)
        addr_array = (byte * size)(*address_bytes)
        ptr = lib.vehicle_ecu_add_if_not_in(byref(self), addr_array, size)
        if not ptr:
            return None
        return ptr.contents

lib.vehicle_new.argtypes = []
lib.vehicle_new.restype = POINTER(Vehicle)

lib.vehicle_free.argtypes = [POINTER(Vehicle)]
lib.vehicle_free.restype = None

lib.vehicle_fill_global_data_buffer_from_ecus.argtypes = [POINTER(Vehicle)]
lib.vehicle_fill_global_data_buffer_from_ecus.restype = None

lib.vehicle_dump.argtypes = [POINTER(Vehicle)]
lib.vehicle_dump.restype = None

lib.vehicle_debug.argtypes = [POINTER(Vehicle)]
lib.vehicle_debug.restype = None

lib.vehicle_ecu_add.argtypes = [POINTER(Vehicle), POINTER(byte), c_int]
lib.vehicle_ecu_add.restype = POINTER(ECU)

lib.vehicle_ecu_add_if_not_in.argtypes = [POINTER(Vehicle), POINTER(byte), c_int]
lib.vehicle_ecu_add_if_not_in.restype = POINTER(ECU)

lib.vehicle_ecu_empty.argtypes = [POINTER(ECU)]
lib.vehicle_ecu_empty.restype = None

lib.vehicle_ecu_empty_duplicated_info.argtypes = [POINTER(ECU)]
lib.vehicle_ecu_empty_duplicated_info.restype = None

lib.vehicle_ecu_debug.argtypes = [POINTER(ECU)]
lib.vehicle_ecu_debug.restype = None

lib.vehicle_search_ecu_by_address.argtypes = [POINTER(Vehicle), POINTER(Buffer)]
lib.vehicle_search_ecu_by_address.restype = POINTER(ECU)