from autodiag.libloader import *
from autodiag.com.vehicle_interface import VehicleIFace
from autodiag.buffer import *
from autodiag.com.uds.read_dtc import *

class UDSService(c_int):
    DIAGNOSTIC_SESSION_CONTROL = 0x10
    ECU_RESET = 0x11
    CLEAR_DIAGNOSTIC_INFORMATION = 0x14
    READ_DTC_INFORMATION = 0x19
    READ_DATA_BY_IDENTIFIER = 0x22
    READ_MEMORY_BY_ADDRESS = 0x23
    READ_SCALING_DATA_BY_IDENTIFIER = 0x24
    SECURITY_ACCESS = 0x27
    COMMUNICATION_CONTROL = 0x28
    AUTHENTICATION = 0x29
    READ_DATA_BY_PERIOD_IDENTIFIER = 0x2A
    DYNAMICALLY_DEFINE_DATA_IDENTIFIER = 0x2C
    WRITE_DATA_BY_IDENTIFIER = 0x2E
    INPUT_OUTPUT_CONTROL_BY_IDENTIFIER = 0x2F
    ROUTINE_CONTROL = 0x31
    REQUEST_DOWNLOAD = 0x34
    REQUEST_UPLOAD = 0x35
    TRANSFER_DATA = 0x36
    REQUEST_TRANSFER_EXIT = 0x37
    REQUEST_FILE_TRANSFER = 0x38
    WRITE_MEMORY_BY_ADDRESS = 0x3D
    TESTER_PRESENT = 0x3E
    ACCESS_TIMING_PARAMETERS = 0x83
    SECURED_DATA_TRANSMISSION = 0x84
    CONTROL_DTC_SETTING = 0x85
    RESPONSE_ON_EVENT = 0x86
    LINK_CONTROL = 0x87
    NEGATIVE_RESPONSE = 0x7F

class UDS_NRC(c_int):
    GENERAL_REJECT = 0x10
    SERVICE_NOT_SUPPORTED = 0x11
    SUBFUNCTION_NOT_SUPPORTED = 0x12
    IncorrectMessageLengthOrInvalidFormat = 0x13
    RESPONSE_TOO_LONG = 0x14
    BUSY_REPEAT_REQUEST = 0x21
    CONDITIONS_NOT_CORRECT = 0x22
    REQUEST_SEQUENCE_ERROR = 0x24
    NO_RESPONSE_FROM_SUBNET = 0x25
    FAILURE_PREVENTS_EXECUTION = 0x26
    REQUEST_OUT_OF_RANGE = 0x31
    SECURITY_ACCESS_DENIED = 0x33
    INVALID_KEY = 0x35
    EXCEEDED_NUMBER_OF_ATTEMPTS = 0x36
    REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37
    UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70
    TRANSFER_DATA_SUSPENDED = 0x71
    PROGRAMMING_FAILURE = 0x72
    WRONG_BLOCK_SEQUENCE_COUNTER = 0x73
    REQUEST_RECEIVED_RESPONSE_PENDING = 0x78
    SUBFUNCTION_NOT_SUPPORTED_IN_SESSION = 0x7E
    SERVICE_NOT_SUPPORTED_IN_SESSION = 0x7F
    RPM_TOO_HIGH = 0x81
    RPM_TOO_LOW = 0x82
    ENGINE_RUNNING = 0x83
    ENGINE_NOT_RUNNING = 0x84
    ENGINE_RUN_TIME_TOO_LOW = 0x85
    TEMPERATURE_TOO_HIGH = 0x86
    TEMPERATURE_TOO_LOW = 0x87
    SPEED_TOO_HIGH = 0x88
    SPEED_TOO_LOW = 0x89
    THROTTLE_PEDAL_TOO_HIGH = 0x8A
    THROTTLE_PEDAL_TOO_LOW = 0x8B
    TRANSMISSION_RANGE_NOT_IN_NEUTRAL = 0x8C
    TRANSMISSION_RANGE_NOT_IN_DEAR = 0x8D
    BRAKE_SWITCHES_NOT_CLOSED = 0x8F
    SHIFTER_LEVEL_NOT_IN_PARK = 0x90
    TORK_CONVERTER_CLUTCH_LOCKED = 0x91
    VOLTAGE_TOO_HIGH = 0x92
    VOLTAGE_TOO_LOW = 0x93

lib.uds_service_to_string.argtypes = [UDSService]
lib.uds_service_to_string.restype = c_char_p

lib.uds_nrc_to_string.argtypes = [UDS_NRC]
lib.uds_nrc_to_string.restype = c_char_p

class READ_DATA_BY_IDENTIFIER_DID(c_int):
    bootSoftwareIdentificationDataIdentifier = 0xF180
    applicationSoftwareIdentificationDataIdentifier = 0xF181
    applicationDataIdentification = 0xF182
    bootSoftwareFingerprint = 0xF183
    applicationSoftwareFingerprint = 0xF184
    applicationDataFingerprint = 0xF185
    Active_Diagnostic_Session_Data_Identifier_information = 0xF186
    manufacturerSparePartNumber = 0xF187
    manufacturerECUSoftwareNumber = 0xF188
    manufacturerECUSoftwareVersion = 0xF189
    identifierOfSystemSupplier = 0xF18A
    ECUManufacturingDate = 0xF18B
    ECUSerialNumber = 0xF18C
    SupportedFunctionnalUnit = 0xF18D
    ManufacturerKitAssemblyPartNumber = 0xF18E
    VIN = 0xF190
    system_supplier_ECU_hardware_number = 0xF192
    system_supplier_ECU_hardware_version_number = 0xF193
    system_supplier_ECU_software_number = 0xF194
    system_supplier_ECU_software_version_number = 0xF195
    exhaust_regulation_type_approval_number = 0xF196
    system_name_engine_type = 0xF197
    repair_shop_code_tester_serial_number = 0xF198
    programming_date = 0xF199
    ECU_installation_date = 0xF19D
    ODX_file = 0xF19E

class READ_DATA_BY_IDENTIFIER_WWH_OBD_DID(c_int):
    READ_DATA_BY_IDENTIFIER_WWH_OBD_DID_VIN = 0xF802

lib.uds_read_data_by_identifier.argtypes = [POINTER(VehicleIFace), c_int]
lib.uds_read_data_by_identifier.restype = POINTER(list_Buffer)

class UDS_SESSION(c_ubyte):
    DEFAULT = 0x01
    PROGRAMMING = 0x02
    EXTENDED_DIAGNOSTIC = 0x03
    SYSTEM_SAFETY_DIAGNOSTIC = 0x04

lib.uds_request_session_cond.argtypes = [POINTER(VehicleIFace), c_ubyte]
lib.uds_request_session_cond.restype = c_bool

lib.uds_is_enabled.argtypes = [POINTER(VehicleIFace)]
lib.uds_is_enabled.restype = c_bool

lib.uds_tester_present.argtypes = [POINTER(VehicleIFace), c_bool]
lib.uds_tester_present.restype = c_bool

lib.uds_clear_dtcs.argtypes = [POINTER(VehicleIFace)]
lib.uds_clear_dtcs.restype = c_bool

class UDS():
    """
        Communicate over interface with UDS
    """

    def __init__(self, iface):
        self.iface = iface

    @staticmethod
    def service_to_string(service_id): return lib.uds_service_to_string(service_id).decode()
    @staticmethod
    def nrc_to_string(nrc): return lib.uds_nrc_to_string(nrc).decode()

    def request_session(self, session: UDS_SESSION) -> bool: return lib.uds_request_session_cond(byref(self.iface), session)
    def is_enabled(self): return lib.uds_is_enabled(byref(self.iface))
    def tester_present(self): return lib.uds_tester_present(byref(self.iface), True)
    def clear_dtcs(self): return lib.uds_clear_dtcs(byref(self.iface))
    def read_dtcs(self) -> Array[DTC]: 
        list_dtc_ptr = lib.uds_read_all_dtcs(byref(self.iface), None)
        list_dtc = list_dtc_ptr.contents
        dtcs = []
        for i in range(list_dtc.size):
            dtcs.append(list_dtc.list[i])
        return dtcs
