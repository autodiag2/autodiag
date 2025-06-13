from ctypes import *
from autodiag.libloader import load_lib
from autodiag.com.obd.obd import OBDIFace

lib = load_lib()

class SAEJ1979():

    def __init__(self, iface):
        self.iface = iface

    def is_pid_supported(self, pid, freezed=False): return lib.saej1979_data_is_pid_supported(byref(self.iface), freezed, pid)
    def number_of_dtc(self, freezed=False): return lib.saej1979_data_number_of_dtc(byref(self.iface), freezed)
    def mil_status(self, freezed=False): return lib.saej1979_data_mil_status(byref(self.iface), freezed)
    def freeze_frame(self): return lib.saej1979_data_freeze_frame(byref(self.iface))
    def engine_type(self, freezed=False): return lib.saej1979_data_engine_type(byref(self.iface), freezed)
    def engine_type_str(self, freezed=False): return lib.saej1979_data_engine_type_as_string(byref(self.iface), freezed).decode()
    def engine_load(self, freezed=False): return lib.saej1979_data_engine_load(byref(self.iface), freezed)
    def coolant_temp(self, freezed=False): return lib.saej1979_data_engine_coolant_temperature(byref(self.iface), freezed)
    def fuel_trim_stft_b1(self, freezed=False): return lib.saej1979_data_short_term_fuel_trim_bank_1(byref(self.iface), freezed)
    def fuel_trim_ltft_b1(self, freezed=False): return lib.saej1979_data_long_term_fuel_trim_bank_1(byref(self.iface), freezed)
    def fuel_trim_stft_b2(self, freezed=False): return lib.saej1979_data_short_term_fuel_trim_bank_2(byref(self.iface), freezed)
    def fuel_trim_ltft_b2(self, freezed=False): return lib.saej1979_data_long_term_fuel_trim_bank_2(byref(self.iface), freezed)
    def fuel_pressure(self, freezed=False): return lib.saej1979_data_fuel_pressure(byref(self.iface), freezed)
    def intake_pressure(self, freezed=False): return lib.saej1979_data_intake_manifold_pressure(byref(self.iface), freezed)
    def max_oxygen_sensor_current(self, freezed=False): return lib.saej1979_data_max_oxygen_sensor_current(byref(self.iface), freezed)
    def rpm(self, freezed=False): return lib.saej1979_data_engine_speed(byref(self.iface), freezed)
    def speed(self, freezed=False): return lib.saej1979_data_vehicle_speed(byref(self.iface), freezed)
    def timing_advance(self, freezed=False): return lib.saej1979_data_timing_advance_cycle_1(byref(self.iface), freezed)
    def intake_air_temp(self, freezed=False): return lib.saej1979_data_intake_air_temperature(byref(self.iface), freezed)
    def maf_air_flow(self, freezed=False): return lib.saej1979_data_maf_air_flow_rate(byref(self.iface), freezed)
    def throttle_position(self, freezed=False): return lib.saej1979_data_throttle_position(byref(self.iface), freezed)
    def fuel_system_status(self, freezed=False):
        arr = lib.saej1979_data_fuel_system_status(byref(self.iface), freezed)
        return [arr[0].decode(), arr[1].decode()]
    def rpm(self, freezed=False): return lib.saej1979_data_engine_speed(byref(self.iface), freezed)
    def speed(self, freezed=False): return lib.saej1979_data_vehicle_speed(byref(self.iface), freezed)
    def timing_advance(self, freezed=False): return lib.saej1979_data_timing_advance_cycle_1(byref(self.iface), freezed)
    def intake_air_temperature(self, freezed=False): return lib.saej1979_data_intake_air_temperature(byref(self.iface), freezed)
    def maf_air_flow_rate(self, freezed=False): return lib.saej1979_data_maf_air_flow_rate(byref(self.iface), freezed)
    def throttle_position(self, freezed=False): return lib.saej1979_data_throttle_position(byref(self.iface), freezed)
    def secondary_air_status(self, freezed=False): return lib.saej1979_data_secondary_air_status(byref(self.iface), freezed)
    def oxygen_sensors_present(self, sensor_i, freezed=False): return lib.saej1979_data_oxygen_sensors_present(byref(self.iface), freezed, sensor_i)
    def oxygen_sensor_voltage(self, sensor_i, freezed=False): return lib.saej1979_data_oxygen_sensor_voltage(byref(self.iface), freezed, sensor_i)
    def oxygen_sensor_trim(self, sensor_i, freezed=False): return lib.saej1979_data_oxygen_sensor_trim(byref(self.iface), freezed, sensor_i)
    def obd_standard(self, freezed=False): return lib.saej1979_data_obd_standard(byref(self.iface), freezed)
    def obd_standard_as_string(self, freezed=False): return lib.saej1979_data_obd_standard_as_string(byref(self.iface), freezed).decode()
    def oxygen_sensors_present_2(self, sensor_i, freezed=False): return lib.saej1979_data_oxygen_sensors_present_2(byref(self.iface), freezed, sensor_i)
    def powertakeoff_status(self, freezed=False): return lib.saej1979_data_powertakeoff_status(byref(self.iface), freezed)
    def seconds_since_engine_start(self, freezed=False): return lib.saej1979_data_seconds_since_engine_start(byref(self.iface), freezed)
    def time_since_engine_start(self, freezed=False):
        res = lib.saej1979_data_time_since_engine_start(byref(self.iface), freezed)
        return res.decode() if res else None
    def distance_since_mil_activated(self, freezed=False): return lib.saej1979_data_distance_since_mil_activated(byref(self.iface), freezed)
    def frp_relative(self, freezed=False): return lib.saej1979_data_frp_relative(byref(self.iface), freezed)
    def frp_widerange(self, freezed=False): return lib.saej1979_data_frp_widerange(byref(self.iface), freezed)
    def oxygen_sensor_air_fuel_equiv_ratio(self, sensor_i, freezed=False): return lib.saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(byref(self.iface), freezed, sensor_i)
    def oxygen_sensor_voltage_ext_range_ratio(self, sensor_i, freezed=False): return lib.saej1979_data_oxygen_sensor_voltage_ext_range(byref(self.iface), freezed, sensor_i)
    def commanded_egr(self, freezed=False): return lib.saej1979_data_commanded_egr(byref(self.iface), freezed)
    def egr_error(self, freezed=False): return lib.saej1979_data_egr_error(byref(self.iface), freezed)
    def commanded_evap_purge(self, freezed=False): return lib.saej1979_data_commanded_evap_purge(byref(self.iface), freezed)
    def fuel_tank_level_input(self, freezed=False): return lib.saej1979_data_fuel_tank_level_input(byref(self.iface), freezed)
    def warm_ups_since_ecu_reset(self, freezed=False): return lib.saej1979_data_warm_ups_since_ecu_reset(byref(self.iface), freezed)
    def distance_since_ecu_reset(self, freezed=False): return lib.saej1979_data_distance_since_ecu_reset(byref(self.iface), freezed)
    def evap_system_vapor_pressure(self, freezed=False): return lib.saej1979_data_evap_system_vapor_pressure(byref(self.iface), freezed)
    def baro_pressure_absolute(self, freezed=False): return lib.saej1979_data_baro_pressure_absolute(byref(self.iface), freezed)
    def oxygen_sensor_current(self, sensor_i, freezed=False): return lib.saej1979_data_oxygen_sensor_current(byref(self.iface), freezed, sensor_i)
    def catalyst_temperature(self, sensor_i, freezed=False): return lib.saej1979_data_catalyst_tempature(byref(self.iface), freezed, sensor_i)
    def catalyst_temperature_with_bank(self, bank_i, sensor_i, freezed=False): return lib.saej1979_data_catalyst_tempature_with_bank(byref(self.iface), freezed, bank_i, sensor_i)
    def ecu_voltage(self, freezed=False): return lib.saej1979_data_ecu_voltage(byref(self.iface), freezed)
    def absolute_engine_load(self, freezed=False): return lib.saej1979_data_absolute_engine_load(byref(self.iface), freezed)
    def commanded_air_fuel_equivalence_ratio(self, freezed=False): return lib.saej1979_data_commanded_air_fuel_equivalence_ratio(byref(self.iface), freezed)
    def relative_throttle_position(self, freezed=False): return lib.saej1979_data_relative_throttle_position(byref(self.iface), freezed)
    def ambient_air_temperature(self, freezed=False): return lib.saej1979_data_ambient_air_temperature(byref(self.iface), freezed)
    def absolute_throttle_position_B(self, freezed=False): return lib.saej1979_data_absolute_throttle_position_B(byref(self.iface), freezed)
    def absolute_throttle_position_C(self, freezed=False): return lib.saej1979_data_absolute_throttle_position_C(byref(self.iface), freezed)
    def accelerator_pedal_position_D(self, freezed=False): return lib.saej1979_data_accelerator_pedal_position_D(byref(self.iface), freezed)
    def accelerator_pedal_position_E(self, freezed=False): return lib.saej1979_data_accelerator_pedal_position_E(byref(self.iface), freezed)
    def accelerator_pedal_position_F(self, freezed=False): return lib.saej1979_data_accelerator_pedal_position_F(byref(self.iface), freezed)
    def commanded_throttle_actuator(self, freezed=False): return lib.saej1979_data_commanded_throttle_actuator(byref(self.iface), freezed)
    def time_run_mil_on(self, freezed=False): return lib.saej1979_data_time_run_mil_on(byref(self.iface), freezed)
    def time_since_trouble_codes_cleared(self, freezed=False): return lib.saej1979_data_time_since_trouble_codes_cleared(byref(self.iface), freezed)
    def max_fuel_air_equiv_ratio(self, freezed=False): return lib.saej1979_data_max_fuel_air_equiv_ratio(byref(self.iface), freezed)
    def max_oxygen_sensor_voltage(self, freezed=False): return lib.saej1979_data_max_oxygen_sensor_voltage(byref(self.iface), freezed)
    def max_intake_manifold_absolute_pressure(self, freezed=False): return lib.saej1979_data_max_intake_manifold_absolute_pressure(byref(self.iface), freezed)
    def max_air_flow_rate_from_maf_sensor(self, freezed=False): return lib.saej1979_data_max_air_flow_rate_from_maf_sensor(byref(self.iface), freezed)
    def fuel_type_as_string(self, freezed=False): return lib.saej1979_data_fuel_type_as_string(byref(self.iface), freezed).decode()
    def fuel_type(self, freezed=False): return lib.saej1979_data_fuel_type(byref(self.iface), freezed)
    def ethanol_fuel_percent(self, freezed=False): return lib.saej1979_data_ethanol_fuel_percent(byref(self.iface), freezed)
    def absolute_evap_system_vapor_pressure(self, freezed=False): return lib.saej1979_data_absolute_evap_system_vapor_pressure(byref(self.iface), freezed)
    def relative_evap_system_vapor_pressure(self, freezed=False): return lib.saej1979_data_relative_evap_system_vapor_pressure(byref(self.iface), freezed)
    def short_term_secondary_oxygen_sensor_trim(self, bank_i, freezed=False): return lib.saej1979_data_short_term_secondary_oxygen_sensor_trim(byref(self.iface), freezed, bank_i)
    def long_term_secondary_oxygen_sensor_trim(self, bank_i, freezed=False): return lib.saej1979_data_long_term_secondary_oxygen_sensor_trim(byref(self.iface), freezed, bank_i)
    def fuel_rail_absolute_pressure(self, freezed=False): return lib.saej1979_data_fuel_rail_absolute_pressure(byref(self.iface), freezed)
    def relative_accelerator_pedal_position(self, freezed=False): return lib.saej1979_data_relative_accelerator_pedal_position(byref(self.iface), freezed)
    def hybrid_battery_pack_remaining_life(self, freezed=False): return lib.saej1979_data_hybrid_battery_pack_remaining_life(byref(self.iface), freezed)
    def engine_oil_temperature(self, freezed=False): return lib.saej1979_data_engine_oil_temperature(byref(self.iface), freezed)
    def fuel_injection_timing(self, freezed=False): return lib.saej1979_data_fuel_injection_timing(byref(self.iface), freezed)
    def engine_fuel_rate(self, freezed=False): return lib.saej1979_data_engine_fuel_rate(byref(self.iface), freezed)
    def actual_engine_percent_torque(self, freezed=False): return lib.saej1979_data_actual_engine_percent_torque(byref(self.iface), freezed)
    def driver_demand_engine_percent_torque(self, freezed=False): return lib.saej1979_data_driver_demand_engine_percent_torque(byref(self.iface), freezed)
    def engine_reference_torque(self, freezed=False): return lib.saej1979_data_engine_reference_torque(byref(self.iface), freezed)
    def engine_percent_torque_data_idle(self, freezed=False): return lib.saej1979_data_engine_percent_torque_data_idle(byref(self.iface), freezed)
    def engine_percent_torque_data_point_1(self, freezed=False): return lib.saej1979_data_engine_percent_torque_data_point_1(byref(self.iface), freezed)
    def engine_percent_torque_data_point_2(self, freezed=False): return lib.saej1979_data_engine_percent_torque_data_point_2(byref(self.iface), freezed)
    def engine_percent_torque_data_point_3(self, freezed=False): return lib.saej1979_data_engine_percent_torque_data_point_3(byref(self.iface), freezed)
    def engine_percent_torque_data_point_4(self, freezed=False): return lib.saej1979_data_engine_percent_torque_data_point_4(byref(self.iface), freezed)
    def maf_sensor_present(self, sensor_i, freezed=False): return lib.saej1979_data_maf_sensor_present(byref(self.iface), freezed, sensor_i)
    def maf_sensor(self, sensor_i, freezed=False): return lib.saej1979_data_maf_sensor(byref(self.iface), freezed, sensor_i)
    def engine_coolant_temperature_sensor(self, sensor_i, freezed=False): return lib.saej1979_data_engine_coolant_temperature_sensor(byref(self.iface), freezed, sensor_i)
    def engine_coolant_temperature_sensor_present(self, sensor_i, freezed=False): return lib.saej1979_data_engine_coolant_temperature_sensor_present(byref(self.iface), freezed, sensor_i)
    def engine_intake_air_temperature_sensor(self, sensor_i, freezed=False): return lib.saej1979_data_engine_intake_air_temperature_sensor(byref(self.iface), freezed, sensor_i)
    def engine_intake_air_temperature_sensor_present(self, sensor_i, freezed=False): return lib.saej1979_data_engine_intake_air_temperature_sensor_present(byref(self.iface), freezed, sensor_i)
    def egt_sensor_present(self, sensor_i, freezed=False): return lib.saej1979_egt_sensor_present(byref(self.iface), freezed, sensor_i)
    def egt_sensor_temperature(self, sensor_i, freezed=False): return lib.saej1979_egt_sensor_temperature(byref(self.iface), freezed, sensor_i)
    def diesel_particulate_filter_temperature(self, freezed=False): return lib.saej1979_data_diesel_particulate_filter_temperature(byref(self.iface), freezed)
    def engine_friction_percent_torque(self, freezed=False): return lib.saej1979_data_engine_friction_percent_torque(byref(self.iface), freezed)
    def cylinder_fuel_rate(self, freezed=False): return lib.saej1979_data_cylinder_fuel_rate(byref(self.iface), freezed)
    def transmission_actual_gear_present(self, freezed=False): return lib.saej1979_data_transmission_actual_gear_present(byref(self.iface), freezed)
    def transmission_actual_gear(self, freezed=False): return lib.saej1979_data_transmission_actual_gear(byref(self.iface), freezed)
    def commanded_diesel_exhaust_fluid_dosing_present(self, freezed=False): return lib.saej1979_data_commanded_diesel_exhaust_fluid_dosing_present(byref(self.iface), freezed)
    def commanded_diesel_exhaust_fluid_dosing(self, freezed=False): return lib.saej1979_data_commanded_diesel_exhaust_fluid_dosing(byref(self.iface), freezed)
    def odometer(self, freezed=False): return lib.saej1979_data_odometer(byref(self.iface), freezed)
    def abs_switch_present(self, freezed=False): return lib.saej1979_data_abs_switch_present(byref(self.iface), freezed)
    def abs_switch(self, freezed=False): return lib.saej1979_data_abs_switch(byref(self.iface), freezed)

lib.saej1979_data_number_of_dtc.restype = c_int
lib.saej1979_data_mil_status.restype = c_bool
lib.saej1979_data_engine_type.restype = c_int
lib.saej1979_data_engine_type_as_string.restype = c_char_p
lib.saej1979_data_engine_load.restype = c_double
lib.saej1979_data_engine_coolant_temperature.restype = c_int
lib.saej1979_data_short_term_fuel_trim_bank_1.restype = c_int
lib.saej1979_data_long_term_fuel_trim_bank_1.restype = c_int
lib.saej1979_data_short_term_fuel_trim_bank_2.restype = c_int
lib.saej1979_data_long_term_fuel_trim_bank_2.restype = c_int
lib.saej1979_data_fuel_pressure.restype = c_int
lib.saej1979_data_intake_manifold_pressure.restype = c_int
lib.saej1979_data_engine_speed.restype = c_double
lib.saej1979_data_vehicle_speed.restype = c_int
lib.saej1979_data_timing_advance_cycle_1.restype = c_double
lib.saej1979_data_intake_air_temperature.restype = c_int
lib.saej1979_data_maf_air_flow_rate.restype = c_double
lib.saej1979_data_throttle_position.restype = c_double
lib.saej1979_data_fuel_system_status.restype = POINTER(c_char_p)
lib.saej1979_data_is_pid_supported.restype = c_bool
lib.saej1979_data_is_pid_supported.argtypes = [POINTER(OBDIFace), c_bool, c_int]
lib.saej1979_data_freeze_frame.restype = c_bool
lib.saej1979_data_freeze_frame.argtypes = [POINTER(OBDIFace)]
lib.saej1979_data_fuel_system_status_code_to_str.restype = c_char_p
lib.saej1979_data_fuel_system_status_code_to_str.argtypes = [c_int]
lib.saej1979_data_fuel_system_status.restype = POINTER(c_char_p)
lib.saej1979_data_fuel_system_status.argtypes = [POINTER(OBDIFace), c_bool]
lib.saej1979_data_engine_load.restype = c_double
lib.saej1979_data_engine_load.argtypes = [POINTER(OBDIFace), c_bool]
lib.saej1979_data_engine_coolant_temperature.restype = c_int
lib.saej1979_data_engine_coolant_temperature.argtypes = [POINTER(OBDIFace), c_bool]
lib.saej1979_data_short_term_fuel_trim_bank_1.restype = c_int
lib.saej1979_data_short_term_fuel_trim_bank_1.argtypes = [POINTER(OBDIFace), c_bool]
lib.saej1979_data_long_term_fuel_trim_bank_1.restype = c_int
lib.saej1979_data_long_term_fuel_trim_bank_1.argtypes = [POINTER(OBDIFace), c_bool]
lib.saej1979_data_short_term_fuel_trim_bank_2.restype = c_int
lib.saej1979_data_short_term_fuel_trim_bank_2.argtypes = [POINTER(OBDIFace), c_bool]
lib.saej1979_data_long_term_fuel_trim_bank_2.restype = c_int
lib.saej1979_data_long_term_fuel_trim_bank_2.argtypes = [POINTER(OBDIFace), c_bool]
lib.saej1979_data_fuel_pressure.restype = c_int
lib.saej1979_data_fuel_pressure.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_oxygen_sensor_voltage_ext_range.restype = c_double
lib.saej1979_data_oxygen_sensor_voltage_ext_range.argtypes = [POINTER(OBDIFace), c_bool, c_int]
lib.saej1979_data_intake_manifold_pressure.restype = c_int
lib.saej1979_data_intake_manifold_pressure.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_max_oxygen_sensor_current.restype = c_int
lib.saej1979_data_max_oxygen_sensor_current.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_speed.restype = c_double
lib.saej1979_data_engine_speed.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_vehicle_speed.restype = c_int
lib.saej1979_data_vehicle_speed.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_timing_advance_cycle_1.restype = c_double
lib.saej1979_data_timing_advance_cycle_1.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_intake_air_temperature.restype = c_int
lib.saej1979_data_intake_air_temperature.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_maf_air_flow_rate.restype = c_double
lib.saej1979_data_maf_air_flow_rate.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_throttle_position.restype = c_double
lib.saej1979_data_throttle_position.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_secondary_air_status.restype = c_int
lib.saej1979_data_secondary_air_status.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_oxygen_sensors_present.restype = c_bool
lib.saej1979_data_oxygen_sensors_present.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_oxygen_sensor_voltage.restype = c_double
lib.saej1979_data_oxygen_sensor_voltage.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_oxygen_sensor_trim.restype = c_double
lib.saej1979_data_oxygen_sensor_trim.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_obd_standard.restype = c_int
lib.saej1979_data_obd_standard.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_obd_standard_as_string.restype = c_char_p
lib.saej1979_data_obd_standard_as_string.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_oxygen_sensors_present_2.restype = c_bool
lib.saej1979_data_oxygen_sensors_present_2.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_powertakeoff_status.restype = c_bool
lib.saej1979_data_powertakeoff_status.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_seconds_since_engine_start.restype = c_int
lib.saej1979_data_seconds_since_engine_start.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_time_since_engine_start.restype = c_char_p
lib.saej1979_data_time_since_engine_start.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_distance_since_mil_activated.restype = c_int
lib.saej1979_data_distance_since_mil_activated.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_frp_relative.restype = c_double
lib.saej1979_data_frp_relative.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_frp_widerange.restype = c_int
lib.saej1979_data_frp_widerange.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_oxygen_sensor_air_fuel_equiv_ratio.restype = c_double
lib.saej1979_data_oxygen_sensor_air_fuel_equiv_ratio.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_commanded_egr.restype = c_double
lib.saej1979_data_commanded_egr.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_egr_error.restype = c_double
lib.saej1979_data_egr_error.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_commanded_evap_purge.restype = c_double
lib.saej1979_data_commanded_evap_purge.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_fuel_tank_level_input.restype = c_double
lib.saej1979_data_fuel_tank_level_input.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_warm_ups_since_ecu_reset.restype = c_int
lib.saej1979_data_warm_ups_since_ecu_reset.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_distance_since_ecu_reset.restype = c_int
lib.saej1979_data_distance_since_ecu_reset.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_evap_system_vapor_pressure.restype = c_double
lib.saej1979_data_evap_system_vapor_pressure.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_baro_pressure_absolute.restype = c_int
lib.saej1979_data_baro_pressure_absolute.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_oxygen_sensor_current.restype = c_int
lib.saej1979_data_oxygen_sensor_current.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_catalyst_tempature.restype = c_int
lib.saej1979_data_catalyst_tempature.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_catalyst_tempature_with_bank.restype = c_int
lib.saej1979_data_catalyst_tempature_with_bank.argtypes = [POINTER(OBDIFace), c_bool, c_int, c_int]

lib.saej1979_data_ecu_voltage.restype = c_double
lib.saej1979_data_ecu_voltage.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_absolute_engine_load.restype = c_double
lib.saej1979_data_absolute_engine_load.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_commanded_air_fuel_equivalence_ratio.restype = c_double
lib.saej1979_data_commanded_air_fuel_equivalence_ratio.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_relative_throttle_position.restype = c_double
lib.saej1979_data_relative_throttle_position.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_ambient_air_temperature.restype = c_int
lib.saej1979_data_ambient_air_temperature.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_absolute_throttle_position_B.restype = c_double
lib.saej1979_data_absolute_throttle_position_B.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_absolute_throttle_position_C.restype = c_double
lib.saej1979_data_absolute_throttle_position_C.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_accelerator_pedal_position_D.restype = c_double
lib.saej1979_data_accelerator_pedal_position_D.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_accelerator_pedal_position_E.restype = c_double
lib.saej1979_data_accelerator_pedal_position_E.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_accelerator_pedal_position_F.restype = c_double
lib.saej1979_data_accelerator_pedal_position_F.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_commanded_throttle_actuator.restype = c_double
lib.saej1979_data_commanded_throttle_actuator.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_time_run_mil_on.restype = c_int
lib.saej1979_data_time_run_mil_on.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_time_since_trouble_codes_cleared.restype = c_int
lib.saej1979_data_time_since_trouble_codes_cleared.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_max_fuel_air_equiv_ratio.restype = c_int
lib.saej1979_data_max_fuel_air_equiv_ratio.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_max_oxygen_sensor_voltage.restype = c_int
lib.saej1979_data_max_oxygen_sensor_voltage.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_max_intake_manifold_absolute_pressure.restype = c_int
lib.saej1979_data_max_intake_manifold_absolute_pressure.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_max_air_flow_rate_from_maf_sensor.restype = c_int
lib.saej1979_data_max_air_flow_rate_from_maf_sensor.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_fuel_type_as_string.restype = c_char_p
lib.saej1979_data_fuel_type_as_string.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_fuel_type.restype = c_int
lib.saej1979_data_fuel_type.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_ethanol_fuel_percent.restype = c_double
lib.saej1979_data_ethanol_fuel_percent.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_absolute_evap_system_vapor_pressure.restype = c_double
lib.saej1979_data_absolute_evap_system_vapor_pressure.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_relative_evap_system_vapor_pressure.restype = c_int
lib.saej1979_data_relative_evap_system_vapor_pressure.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_short_term_secondary_oxygen_sensor_trim.restype = c_int
lib.saej1979_data_short_term_secondary_oxygen_sensor_trim.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_long_term_secondary_oxygen_sensor_trim.restype = c_int
lib.saej1979_data_long_term_secondary_oxygen_sensor_trim.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_fuel_rail_absolute_pressure.restype = c_int
lib.saej1979_data_fuel_rail_absolute_pressure.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_relative_accelerator_pedal_position.restype = c_double
lib.saej1979_data_relative_accelerator_pedal_position.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_hybrid_battery_pack_remaining_life.restype = c_double
lib.saej1979_data_hybrid_battery_pack_remaining_life.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_oil_temperature.restype = c_int
lib.saej1979_data_engine_oil_temperature.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_fuel_injection_timing.restype = c_double
lib.saej1979_data_fuel_injection_timing.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_fuel_rate.restype = c_double
lib.saej1979_data_engine_fuel_rate.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_actual_engine_percent_torque.restype = c_int
lib.saej1979_data_actual_engine_percent_torque.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_driver_demand_engine_percent_torque.restype = c_int
lib.saej1979_data_driver_demand_engine_percent_torque.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_reference_torque.restype = c_int
lib.saej1979_data_engine_reference_torque.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_percent_torque_data_idle.restype = c_int
lib.saej1979_data_engine_percent_torque_data_idle.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_percent_torque_data_point_1.restype = c_int
lib.saej1979_data_engine_percent_torque_data_point_1.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_percent_torque_data_point_2.restype = c_int
lib.saej1979_data_engine_percent_torque_data_point_2.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_percent_torque_data_point_3.restype = c_int
lib.saej1979_data_engine_percent_torque_data_point_3.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_percent_torque_data_point_4.restype = c_int
lib.saej1979_data_engine_percent_torque_data_point_4.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_maf_sensor_present.restype = c_bool
lib.saej1979_data_maf_sensor_present.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_maf_sensor.restype = c_int
lib.saej1979_data_maf_sensor.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_engine_coolant_temperature_sensor.restype = c_int
lib.saej1979_data_engine_coolant_temperature_sensor.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_engine_coolant_temperature_sensor_present.restype = c_bool
lib.saej1979_data_engine_coolant_temperature_sensor_present.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_engine_intake_air_temperature_sensor.restype = c_int
lib.saej1979_data_engine_intake_air_temperature_sensor.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_engine_intake_air_temperature_sensor_present.restype = c_bool
lib.saej1979_data_engine_intake_air_temperature_sensor_present.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_egt_sensor_present.restype = c_bool
lib.saej1979_egt_sensor_present.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_egt_sensor_temperature.restype = c_int
lib.saej1979_egt_sensor_temperature.argtypes = [POINTER(OBDIFace), c_bool, c_int]

lib.saej1979_data_diesel_particulate_filter_temperature.restype = c_int
lib.saej1979_data_diesel_particulate_filter_temperature.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_engine_friction_percent_torque.restype = c_int
lib.saej1979_data_engine_friction_percent_torque.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_cylinder_fuel_rate.restype = c_double
lib.saej1979_data_cylinder_fuel_rate.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_transmission_actual_gear_present.restype = c_bool
lib.saej1979_data_transmission_actual_gear_present.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_transmission_actual_gear.restype = c_int
lib.saej1979_data_transmission_actual_gear.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_commanded_diesel_exhaust_fluid_dosing_present.restype = c_bool
lib.saej1979_data_commanded_diesel_exhaust_fluid_dosing_present.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_commanded_diesel_exhaust_fluid_dosing.restype = c_int
lib.saej1979_data_commanded_diesel_exhaust_fluid_dosing.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_odometer.restype = c_double
lib.saej1979_data_odometer.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_abs_switch_present.restype = c_bool
lib.saej1979_data_abs_switch_present.argtypes = [POINTER(OBDIFace), c_bool]

lib.saej1979_data_abs_switch.restype = c_bool
lib.saej1979_data_abs_switch.argtypes = [POINTER(OBDIFace), c_bool]