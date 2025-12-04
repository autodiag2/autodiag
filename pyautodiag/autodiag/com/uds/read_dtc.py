from autodiag.libloader import *
from autodiag.com.vehicle_interface import VehicleIFace
from autodiag.model.vehicle import *
from autodiag.com.obd import *
from autodiag.model.dtc import *

class UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION(c_int):
    NUMBER_OF_DTC_BY_STATUS_MASK = 0x01
    DTC_BY_STATUS_MASK = 0x02
    DTC_SNAPSHOT_IDENTIFICATION = 0x03
    DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER = 0x04
    DTC_STORED_DATA_BY_RECORD_NUMBER = 0x05
    DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER = 0x06
    NUMBER_OF_DTC_BY_SEVERITY_MASK_RECORD = 0x07
    DTC_BY_SEVERITY_MASK_RECORD = 0x08
    SEVERITY_INFORMATION_OF_DTC = 0x09
    SUPPORTED_DTC = 0x0A
    FIRST_FAILED_DTC = 0x0B
    FIRST_CONFIRMED_DTC = 0x0C
    MOST_RECENT_FAILED_DTC = 0x0D
    MOST_RECENT_CONFIRMED_DTC = 0x0E
    MIRROR_MEMORY_DTC_BY_STATUS_MASK = 0x0F
    MIRROR_MEMORY_DTC_BY_DTC_NUMBER = 0x10
    NUMBER_OF_MIRROR_MEMORY_DTC_BY_STATUS_MASK = 0x11
    NUMBER_OF_EMISSIONS_OBD_DTC_BY_STATUS_MASK = 0x12
    EMISSIONS_OBD_DTC_BY_STATUS_MASK = 0x13
    DTC_FAULT_DETECTION_COUNTER = 0x14
    DTC_WITH_PERMANENT_STATUS = 0x15
    DTC_EXTENDED_DATA_RECORD_BY_RECORD_NUMBER = 0x16
    USER_DEFINED_MEMORY_DTC_BY_STATUS_MASK = 0x17
    USER_DEFINED_MEMORY_DTC_SNAPSHOT_BY_NUMBER = 0x18
    USER_DEFINED_MEMORY_DTC_RECORD_BY_NUMBER = 0x19
    WWH_OBD_DTC_BY_STATUS_MASK_RECORD = 0x42
    WWH_OBD_DTCS_WITH_PERMANENT_STATUS = 0x55

class UDS_DTC_STATUS(c_uint8):
    TestFailed = 0b00000001
    TestFailedThisOperationCycle = 0b00000010
    PendingDTC = 0b00000100
    ConfirmedDTC = 0b00001000
    TestNotCompletedSinceLastClear = 0b00010000
    TestFailedSinceLastClear = 0b00100000
    TestNotCompletedThisOperationCycle = 0b01000000
    WarningIndicatorRequested = 0b10000000

lib.uds_dtc_status_to_string.argtypes = [UDS_DTC_STATUS]
lib.uds_dtc_status_to_string.restype = c_char_p

class UDS_DTC(Structure):

    _fields_ = [
        ("dtc", DTC),
        ("status", c_uint8),
    ]

    def __new__(cls):
        lib.UDS_DTC_new.restype = POINTER(cls)
        ptr = lib.UDS_DTC_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def __str__(self): return lib.UDS_DTC_to_string(byref(self)).decode()
    def explanation(self): return lib.UDS_DTC_explanation(byref(self)).decode()

lib.uds_dtc_dump.argtypes = [POINTER(UDS_DTC)]
lib.uds_dtc_dump.restype = None

lib.UDS_DTC_new_from.restype = POINTER(UDS_DTC)
lib.UDS_DTC_to_string.argtypes = [POINTER(UDS_DTC)]
lib.UDS_DTC_to_string.restype = c_char_p
lib.UDS_DTC_explanation.argtypes = [POINTER(UDS_DTC)]
lib.UDS_DTC_explanation.restype = c_char_p

class list_UDS_DTC(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(UDS_DTC)),
        ("Status_Availability_Mask", c_byte),
        ("ecu", POINTER(ECU))
    ]

lib.uds_read_all_dtcs.argtypes = [POINTER(VehicleIFace), POINTER(Vehicle)]
lib.uds_read_all_dtcs.restype = POINTER(list_UDS_DTC)
