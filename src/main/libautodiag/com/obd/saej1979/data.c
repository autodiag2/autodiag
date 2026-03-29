#include "libautodiag/com/obd/saej1979/data.h"

void ad_saej1979_data_register_signals() {
    AD_SIGNAL_SAEJ1979_REGISTER("Number Of DTC",                            "${0}${1}01", 0,        127,        "$2 & 0x7F",                                 "Get number of DTCs currently flagged in ECU",                   "Engine, ECM",              "number_of_dtc",                                       "count")
    AD_SIGNAL_SAEJ1979_REGISTER("MIL Status",                               "${0}${1}01", 0,        1,          "($2 >> 7) & 1",                             "Get status of MIL/CEL",                                         "Engine, ECM",              "mil_status",                                          "bool")
    AD_SIGNAL_SAEJ1979_REGISTER("Type of engine",                           "${0}${1}01", 0,        1,          "($3 >> 3) & 1",                             "Get the type of engine (0: spark, 1: compression)",             "Engine, ECM",              "engine_spark_or_compression",                         "bool")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine Load",                              "${0}${1}04", 0,        100,        "$2 / 2.55",                                 "Calculated engine load",                                        "Engine, ECM",              "engine_load",                                         "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Coolant Temperature",                      "${0}${1}05", -40,      215,        "$2 - 40",                                   "Engine Coolant Temperature",                                    "Engine, ECM",              "coolant_temp",                                        "°C")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel Trim Short Term bank 1",              "${0}${1}06", -100,     99.2,       "$2 / 1.28 - 100",                           "Fuel Trim Short Term bank 1",                                   "Engine, ECM",              "fuel_trim_short_term_bank_1",                         "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel Trim Long Term bank 1",               "${0}${1}07", -100,     99.2,       "$2 / 1.28 - 100",                           "Fuel Trim Long Term bank 1",                                    "Engine, ECM",              "fuel_trim_long_term_bank_1",                          "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel Trim Short Term bank 2",              "${0}${1}08", -100,     99.2,       "$2 / 1.28 - 100",                           "Fuel Trim Short Term bank 2",                                   "Engine, ECM",              "fuel_trim_short_term_bank_2",                         "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel Trim Long Term bank 2",               "${0}${1}09", -100,     99.2,       "$2 / 1.28 - 100",                           "Fuel Trim Long Term bank 2",                                    "Engine, ECM",              "fuel_trim_long_term_bank_2",                          "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel Pressure",                            "${0}${1}0A", 0,        765,        "$2 * 3",                                    "Fuel Pressure",                                                 "Engine, ECM",              "fuel_pressure",                                       "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("Intake Manifold Pressure",                 "${0}${1}0B", 0,        255,        "$2",                                        "Intake manifold pressure",                                      "Engine, ECM",              "intake_manifold_pressure",                            "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine Speed",                             "${0}${1}0C", 0,        16383.75,   "($2 * 256 + $3) / 4.0",                     "Rotational speed of engine",                                    "Engine, ECM",              "engine_speed",                                        "rpm")
    AD_SIGNAL_SAEJ1979_REGISTER("Vehicle Speed",                            "${0}${1}0D", 0,        255,        "$2",                                        "Vehicle Speed",                                                 "Engine, ECM",              "vehicle_speed",                                       "km/h")
    AD_SIGNAL_SAEJ1979_REGISTER("Timing Advance Cycle 1",                   "${0}${1}0E", -64,      63.5,       "($2 / 2.0) - 64",                           "Timing Advance (Cyl. #1)",                                      "Engine, ECM",              "timing_advance_cycle_1",                              "°")
    AD_SIGNAL_SAEJ1979_REGISTER("Intake Air Temperature",                   "${0}${1}0F", -40,      215,        "$2 - 40",                                   "Intake air temperature",                                        "Engine, ECM",              "intake_air_temperature",                              "°C")
    AD_SIGNAL_SAEJ1979_REGISTER("MAF Air Flow Rate",                        "${0}${1}10", 0,        655.35,     "($2 * 256 + $3) / 100.0",                   "MAF air flow rate",                                             "Engine, ECM",              "maf_air_flow_rate",                                   "g/s")
    AD_SIGNAL_SAEJ1979_REGISTER("Throttle Position",                        "${0}${1}11", 0,        100,        "$2 / 2.55",                                 "Throttle position",                                             "Engine, ECM",              "throttle_position",                                   "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Power Takeoff Status",                     "${0}${1}1E", 0,        1,          "$2 & 1",                                    "Power takeoff status",                                          "Engine, ECM",              "powertakeoff_status",                                 "bool")
    AD_SIGNAL_SAEJ1979_REGISTER("Seconds Since Engine Start",               "${0}${1}1F", 0,        65535,      "$2 * 256 + $3",                             "Seconds since engine start",                                    "Engine, ECM",              "seconds_since_engine_start",                          "s")
    AD_SIGNAL_SAEJ1979_REGISTER("Distance Since MIL Activated",             "${0}${1}21", 0,        65535,      "$2 * 256 + $3",                             "Distance since MIL activated",                                  "Engine, ECM",              "distance_since_mil_activated",                        "km")
    AD_SIGNAL_SAEJ1979_REGISTER("FRP Relative",                             "${0}${1}22", 0,        5177.265,   "($2 * 256 + $3) * 0.079",                   "Fuel rail pressure relative to manifold vacuum",                "Engine, ECM",              "frp_relative",                                        "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("FRP Wide Range",                           "${0}${1}23", 0,        655350,     "($2 * 256 + $3) * 10",                      "Fuel rail pressure wide range",                                 "Engine, ECM",              "frp_widerange",                                       "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("Commanded EGR",                            "${0}${1}2C", 0,        100,        "$2 * (100.0 / 255)",                        "Commanded EGR",                                                 "Engine, ECM",              "commanded_egr",                                       "%")
    AD_SIGNAL_SAEJ1979_REGISTER("EGR Error",                                "${0}${1}2D", -100,     99.2,       "$2 * (100.0 / 128) - 100",                  "EGR error",                                                     "Engine, ECM",              "egr_error",                                           "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Commanded EVAP Purge",                     "${0}${1}2E", 0,        100,        "$2 * (100.0 / 255)",                        "Commanded EVAP purge",                                          "Engine, ECM",              "commanded_evap_purge",                                "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel Tank Level Input",                    "${0}${1}2F", 0,        100,        "$2 * (100.0 / 255)",                        "Fuel tank level input",                                         "Engine, ECM",              "fuel_tank_level_input",                               "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Warm Ups Since ECU Reset",                 "${0}${1}30", 0,        255,        "$2",                                        "Warm ups since ECU reset",                                      "Engine, ECM",              "warm_ups_since_ecu_reset",                            "count")
    AD_SIGNAL_SAEJ1979_REGISTER("Distance Since ECU Reset",                 "${0}${1}31", 0,        65535,      "$2 * 256 + $3",                             "Distance since ECU reset",                                      "Engine, ECM",              "distance_since_ecu_reset",                            "km")
    AD_SIGNAL_SAEJ1979_REGISTER("EVAP System Vapor Pressure",               "${0}${1}32", 0,        16383.75,   "($2 * 256 + $3) * 0.25",                    "EVAP system vapor pressure",                                    "Engine, ECM",              "evap_system_vapor_pressure",                          "Pa")
    AD_SIGNAL_SAEJ1979_REGISTER("Barometric Pressure Absolute",             "${0}${1}33", 0,        255,        "$2",                                        "Absolute barometric pressure",                                  "Engine, ECM",              "baro_pressure_absolute",                              "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("ECU Voltage",                              "${0}${1}42", 0,        65.535,     "($2 * 256 + $3) * 0.001",                   "Control module voltage",                                        "Engine, ECM",              "ecu_voltage",                                         "V")
    AD_SIGNAL_SAEJ1979_REGISTER("Absolute Engine Load",                     "${0}${1}43", 0,        25700,      "($2 * 256 + $3) * (100.0 / 255)",           "Absolute engine load",                                          "Engine, ECM",              "absolute_engine_load",                                "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Commanded Air Fuel Equivalence Ratio",     "${0}${1}44", 0,        2,          "($2 * 256 + $3) * (2.0 / 65536)",           "Commanded air fuel equivalence ratio",                          "Engine, ECM",              "commanded_air_fuel_equivalence_ratio",                "ratio")
    AD_SIGNAL_SAEJ1979_REGISTER("Relative Throttle Position",               "${0}${1}45", 0,        100,        "$2 * (100.0 / 255)",                        "Relative throttle position",                                    "Engine, ECM",              "relative_throttle_position",                          "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Ambient Air Temperature",                  "${0}${1}46", -40,      215,        "$2 - 40",                                   "Ambient air temperature",                                       "Engine, ECM",              "ambient_air_temperature",                             "°C")
    AD_SIGNAL_SAEJ1979_REGISTER("Max Fuel Air Equiv Ratio",                 "${0}${1}4F", 0,        255,        "$2",                                        "Maximum fuel air equivalence ratio",                            "Engine, ECM",              "max_fuel_air_equiv_ratio",                            "raw")
    AD_SIGNAL_SAEJ1979_REGISTER("Max Oxygen Sensor Voltage",                "${0}${1}4F", 0,        255,        "$3",                                        "Maximum oxygen sensor voltage",                                 "Engine, ECM",              "max_oxygen_sensor_voltage",                           "raw")
    AD_SIGNAL_SAEJ1979_REGISTER("Max Oxygen Sensor Current",                "${0}${1}4F", 0,        255,        "$4",                                        "Maximum oxygen sensor current",                                 "Engine, ECM",              "max_oxygen_sensor_current",                           "raw")
    AD_SIGNAL_SAEJ1979_REGISTER("Max Intake Manifold Absolute Pressure",    "${0}${1}4F", 0,        2550,       "$5 * 10",                                   "Maximum intake manifold absolute pressure",                     "Engine, ECM",              "max_intake_manifold_absolute_pressure",               "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel ethanol percent",                     "${0}${1}52", 0,        100,        "$2 * (100.0/255)",                          "Fuel ethanol percent",                                          "Engine, ECM",              "fuel_ethanol_percent",                                "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Absolute EVAP System Vapor Pressure",      "${0}${1}53", 0,        327.675,    "($2 * 256 + $3) / 200.0",                   "Absolute EVAP system vapor pressure",                           "Engine, ECM",              "absolute_evap_system_vapor_pressure",                 "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel rail absolute pressure",              "${0}${1}59", 0,        655350,     "($2 * 256 + $3) * 10",                      "Get the fuel rail absolute pressure",                           "Engine, ECM",              "fuel_rail_absolute_pressure",                         "kPa")
    AD_SIGNAL_SAEJ1979_REGISTER("Relative accelerator pedal position",      "${0}${1}5A", 0,        100,        "$2 / 2.55",                                 "Get the relative accelerator pedal position",                   "Engine, ECM",              "relative_accelerator_pedal_position",                 "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Hybrid battery pack remaining life",       "${0}${1}5B", 0,        100,        "$2 / 2.55",                                 "Get the hybrid battery pack remaining life",                    "Engine, ECM",              "hybrid_battery_pack_remaining_life",                  "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Fuel injection timing",                    "${0}${1}5D", -210,     301.992,    "(($2 * 256) + $3) / 128.0 - 210",           "Get the fuel injection timing",                                 "Engine, ECM",              "fuel_injection_timing",                               "°")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine fuel rate",                         "${0}${1}5E", 0,        3212.75,    "(($2 * 256) + $3) / 20.0",                  "Get the engine fuel rate",                                      "Engine, ECM",              "engine_fuel_rate",                                    "L/h")
    AD_SIGNAL_SAEJ1979_REGISTER("Emission requirements to which vehicle is designed", "${0}${1}5F", 0, 255, "$2", "Get the emission requirements to which vehicle is designed",                                               "Engine, ECM",              "emission_requirements_vehicle_designed",              "not found")
    AD_SIGNAL_SAEJ1979_REGISTER("Driver demand engine percent torque",      "${0}${1}61", -125,     130,        "$2 - 125",                                  "Get the driver demand engine percent torque",                   "Engine, ECM",              "driver_demand_engine_percent_torque",                 "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Actual engine percent torque",             "${0}${1}62", -125,     130,        "$2 - 125",                                  "Get the actual engine percent torque",                          "Engine, ECM",              "actual_engine_percent_torque",                        "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine reference torque",                  "${0}${1}63", 0,        65535,      "($2 * 256) + $3",                           "Get the engine reference torque",                               "Engine, ECM",              "engine_reference_torque",                             "N.m")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine percent torque data idle",          "${0}${1}64", -125,     130,        "$2 - 125",                                  "Get the engine percent torque data idle",                       "Engine, ECM",              "engine_percent_torque_data_idle",                     "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine percent torque data point 1",       "${0}${1}64", -125,     130,        "$3 - 125",                                  "Get the engine percent torque data point 1",                    "Engine, ECM",              "engine_percent_torque_data_point_1",                  "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine percent torque data point 2",       "${0}${1}64", -125,     130,        "$4 - 125",                                  "Get the engine percent torque data point 2",                    "Engine, ECM",              "engine_percent_torque_data_point_2",                  "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine percent torque data point 3",       "${0}${1}64", -125,     130,        "$5 - 125",                                  "Get the engine percent torque data point 3",                    "Engine, ECM",              "engine_percent_torque_data_point_3",                  "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine percent torque data point 4",       "${0}${1}64", -125,     130,        "$6 - 125",                                  "Get the engine percent torque data point 4",                    "Engine, ECM",              "engine_percent_torque_data_point_4",                  "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine Oil Temperature",                   "${0}${1}5C", -40,      210,        "$2 - 40",                                   "Engine oil temperature",                                        "Engine, ECM",              "engine_oil_temperature",                              "°C")
    AD_SIGNAL_SAEJ1979_REGISTER("Engine Friction Percent Torque",           "${0}${1}8E", -125,     130,        "$2 - 125",                                  "Engine friction percent torque",                                "Engine, ECM",              "engine_friction_percent_torque",                      "%")
    AD_SIGNAL_SAEJ1979_REGISTER("Cylinder Fuel Rate",                       "${0}${1}A2", 0,        2047.96875, "($2 * 256 + $3) / 32.0",                    "Cylinder fuel rate",                                            "Engine, ECM",              "cylinder_fuel_rate",                                  "mg/stroke")
    AD_SIGNAL_SAEJ1979_REGISTER("Odometer",                                 "${0}${1}A6", 0,        429296729.5,"($2 * 16777216 + $3 * 65536 + $4 * 256 + $5) / 10.0", "Odometer",                                            "Engine, ECM",              "odometer",                                            "km")
    AD_SIGNAL_SAEJ1979_REGISTER("ABS Switch Present",                       "${0}${1}A9", 0,        1,          "$2 & 1",                                    "ABS switch present",                                            "ABS, ECM",                 "abs_switch_present",                                  "bool")
    AD_SIGNAL_SAEJ1979_REGISTER("ABS Switch",                               "${0}${1}A9", 0,        1,          "$3 & 1",                                    "ABS switch",                                                    "ABS, ECM",                 "abs_switch",                                          "bool")
}
/**
 * Actual data extracted from wikipedia OBDII PIDs page
 */
char *saej1979_data_pid_code_to_str(final unsigned char pid) {
    switch(pid) {
        case 0x00: return strdup("PIDs supported [0x01-0x20]");
        case 0x01: return strdup("Monitor status since DTCs cleared. (Includes malfunction indicator lamp (MIL), status and number of DTCs, components tests, DTC readiness checks)");
        case 0x02: return strdup("DTC that caused feeze frame to be stored");
        case 0x03: return strdup("Fuel system status");
        case 0x04: return strdup("Calculated engine load");
        case 0x05: return strdup("Engine coolant temperature");
        case 0x06: return strdup("Short term fuel trim - Bank 1");
        case 0x07: return strdup("Long term fuel trim - Bank 1");
        case 0x08: return strdup("Short term fuel trim - Bank 2");
        case 0x09: return strdup("Long term fuel trim - Bank 2");
        case 0x0A: return strdup("Fuel pressure (Gauge pressure)");
        case 0x0B: return strdup("Intake manifold absolute pressure");
        case 0x0C: return strdup("Engine speed");
        case 0x0D: return strdup("Vehicle speed");
        case 0x0E: return strdup("Timing advance");
        case 0x0F: return strdup("Intake air temperature");
        case 0x10: return strdup("Mass air flow sensor (MAF) air flow rate");
        case 0x11: return strdup("Throttle position");
        case 0x12: return strdup("Commanded secondary air status");
        case 0x13: return strdup("Oxygen sensors present (in 2 banks)");
        case 0x14: return strdup("Oxygen Sensor 1");
        case 0x15: return strdup("Oxygen Sensor 2");
        case 0x16: return strdup("Oxygen Sensor 3");
        case 0x17: return strdup("Oxygen Sensor 4");
        case 0x18: return strdup("Oxygen Sensor 5");
        case 0x19: return strdup("Oxygen Sensor 6");
        case 0x1A: return strdup("Oxygen Sensor 7");
        case 0x1B: return strdup("Oxygen Sensor 8");
        case 0x1C: return strdup("OBD standards this vehicle comforms to");
        case 0x1D: return strdup("Oxygen sensors present (in 4 banks)");
        case 0x1E: return strdup("Auxiliary input status");
        case 0x1F: return strdup("Run time since engine start");
        case 0x20: return strdup("PIDs supported [0x21-0x40]");
        case 0x21: return strdup("Distance traveled with malfunction indicator lamp (MIL) on");
        case 0x22: return strdup("Fuel Rail Pressure (relative to manifold vacuum)");
        case 0x23: return strdup("Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)");
        case 0x24: return strdup("Oxygen Sensor 1");
        case 0x25: return strdup("Oxygen Sensor 2");
        case 0x26: return strdup("Oxygen Sensor 3");
        case 0x27: return strdup("Oxygen Sensor 4");
        case 0x28: return strdup("Oxygen Sensor 5");
        case 0x29: return strdup("Oxygen Sensor 6");
        case 0x2A: return strdup("Oxygen Sensor 7");
        case 0x2B: return strdup("Oxygen Sensor 8");
        case 0x2C: return strdup("Commanded EGR");
        case 0x2D: return strdup("EGR Error");
        case 0x2E: return strdup("Commanded evaporative purge");
        case 0x2F: return strdup("Fuel Tank Level Input");
        case 0x30: return strdup("Warm-ups since codes cleared");
        case 0x31: return strdup("Distance traveled since codes cleared");
        case 0x32: return strdup("Evap. System Vapor Pressure");
        case 0x33: return strdup("Absolute Barometric Pressure");
        case 0x34: return strdup("Oxygen Sensor 1");
        case 0x35: return strdup("Oxygen Sensor 2");
        case 0x36: return strdup("Oxygen Sensor 3");
        case 0x37: return strdup("Oxygen Sensor 4");
        case 0x38: return strdup("Oxygen Sensor 5");
        case 0x39: return strdup("Oxygen Sensor 6");
        case 0x3A: return strdup("Oxygen Sensor 7");
        case 0x3B: return strdup("Oxygen Sensor 8");
        case 0x3C: return strdup("Catalyst Temperature: Bank 1, Sensor 1");
        case 0x3D: return strdup("Catalyst Temperature: Bank 2, Sensor 1");
        case 0x3E: return strdup("Catalyst Temperature: Bank 1, Sensor 2");
        case 0x3F: return strdup("Catalyst Temperature: Bank 2, Sensor 2");
        case 0x40: return strdup("PIDs supported [0x41-0x60]");
        case 0x41: return strdup("Monitor status this drive cycle");
        case 0x42: return strdup("Control module voltage");
        case 0x43: return strdup("Absolute load value");
        case 0x44: return strdup("Commanded Air-Fuel Equivalence Ratio");
        case 0x45: return strdup("Relative throttle position");
        case 0x46: return strdup("Ambient air temperature");
        case 0x47: return strdup("Absolute throttle position B");
        case 0x48: return strdup("Absolute throttle position C");
        case 0x49: return strdup("Accelerator pedal position D");
        case 0x4A: return strdup("Accelerator pedal position E");
        case 0x4B: return strdup("Accelerator pedal position F");
        case 0x4C: return strdup("Commanded throttle actuator");
        case 0x4D: return strdup("Time run with MIL on");
        case 0x4E: return strdup("Time since trouble codes cleared");
        case 0x4F: return strdup("Maximum value for Fuel–Air equivalence ratio, oxygen sensor voltage, oxygen sensor current, and intake manifold absolute pressure");
        case 0x50: return strdup("Maximum value for air flow rate from mass air flow sensor");
        case 0x51: return strdup("Fuel Type");
        case 0x52: return strdup("Ethanol fuel %");
        case 0x53: return strdup("Absolute Evap system Vapor Pressure");
        case 0x54: return strdup("Evap system vapor pressure");
        case 0x55: return strdup("Short term secondary oxygen sensor trim, A: bank 1, B: bank 3");
        case 0x56: return strdup("Long term secondary oxygen sensor trim, A: bank 1, B: bank ");
        case 0x57: return strdup("Short term secondary oxygen sensor trim, A: bank 2, B: bank ");
        case 0x58: return strdup("Long term secondary oxygen sensor trim, A: bank 2, B: bank ");
        case 0x59: return strdup("Fuel rail absolute pressure");
        case 0x5A: return strdup("Relative accelerator pedal position");
        case 0x5B: return strdup("Hybrid battery pack remaining life");
        case 0x5C: return strdup("Engine oil temperature");
        case 0x5D: return strdup("Fuel injection timing");
        case 0x5E: return strdup("Engine fuel rate");
        case 0x5F: return strdup("Emission requirements to which vehicle is designed");
        case 0x60: return strdup("PIDs supported [61 - 80]");
        case 0x61: return strdup("Driver's demand engine - percent torque");
        case 0x62: return strdup("Actual engine - percent torque");
        case 0x63: return strdup("Engine reference torque");
        case 0x64: return strdup("Engine percent torque data");
        case 0x65: return strdup("Auxiliary input / output supported");
        case 0x66: return strdup("Mass air flow sensor");
        case 0x67: return strdup("Engine coolant temperature");
        case 0x68: return strdup("Intake air temperature sensor");
        case 0x69: return strdup("Actual EGR, Commanded EGR, and EGR Error");
        case 0x6A: return strdup("Commanded Diesel intake air flow control and relative intake air flow position");
        case 0x6B: return strdup("Exhaust gas recirculation temperature");
        case 0x6C: return strdup("Commanded throttle actuator control and relative throttle position");
        case 0x6D: return strdup("Fuel pressure control system");
        case 0x6E: return strdup("Injection pressure control system");
        case 0x6F: return strdup("Turbocharger compressor inlet pressure");
        case 0x70: return strdup("Boost pressure control");
        case 0x71: return strdup("Variable Geometry turbo (VGT) control");
        case 0x72: return strdup("Wastegate control");
        case 0x73: return strdup("Exhaust pressure");
        case 0x74: return strdup("Turbocharger RPM");
        case 0x75: return strdup("Turbocharger temperature");
        case 0x76: return strdup("Turbocharger temperature");
        case 0x77: return strdup("Charge air cooler temperature (CACT)");
        case 0x78: return strdup("Exhaust Gas temperature (EGT) Bank 1");
        case 0x79: return strdup("Exhaust Gas temperature (EGT) Bank 2");
        case 0x7A: return strdup("Diesel particulate filter (DPF) differential pressur");
        case 0x7B: return strdup("Diesel particulate filter (DPF)");
        case 0x7C: return strdup("Diesel Particulate filter (DPF) temperature");
        case 0x7D: return strdup("NOx NTE (Not-To-Exceed) control area status");
        case 0x7E: return strdup("PM NTE (Not-To-Exceed) control area status");
        case 0x7F: return strdup("Engine run time ");
        case 0x80: return strdup("PIDs supported [81 - A0]");
        case 0x81: return strdup("Engine run time for Auxiliary Emissions Control Device(AECD)");
        case 0x82: return strdup("Engine run time for Auxiliary Emissions Control Device(AECD)");
        case 0x83: return strdup("NOx sensor");
        case 0x84: return strdup("Manifold surface temperature");
        case 0x85: return strdup("NOx reagent system");
        case 0x86: return strdup("Particulate matter (PM) sensor");
        case 0x87: return strdup("Intake manifold absolute pressure");
        case 0x88: return strdup("SCR Induce System");
        case 0x89: return strdup("Run Time for AECD #11-#15");
        case 0x8A: return strdup("Run Time for AECD #16-#20");
        case 0x8B: return strdup("Diesel Aftertreatment");
        case 0x8C: return strdup("O2 Sensor (Wide Range)");
        case 0x8D: return strdup("Throttle Position G");
        case 0x8E: return strdup("Engine Friction - Percent Torque");
        case 0x8F: return strdup("PM Sensor Bank 1 & 2");
        case 0x90: return strdup("WWH-OBD Vehicle OBD System Information");
        case 0x91: return strdup("WWH-OBD Vehicle OBD System Information");
        case 0x92: return strdup("Fuel System Control");
        case 0x93: return strdup("WWH-OBD Vehicle OBD Counters support");
        case 0x94: return strdup("NOx Warning And Inducement System");
        case 0x98: return strdup("Exhaust Gas Temperature Sensor");
        case 0x99: return strdup("Exhaust Gas Temperature Sensor");
        case 0x9A: return strdup("Hybrid/EV Vehicle System Data, Battery, Voltage");
        case 0x9B: return strdup("Diesel Exhaust Fluid Sensor Data");
        case 0x9C: return strdup("O2 Sensor Data");
        case 0x9D: return strdup("Engine Fuel Rate");
        case 0x9E: return strdup("Engine Exhaust Flow Rate");
        case 0x9F: return strdup("Fuel System Percentage Use");
        case 0xA0: return strdup("PIDs supported [A1 - C0]");
        case 0xA1: return strdup("NOx Sensor Corrected Data");
        case 0xA2: return strdup("Cylinder Fuel Rate");
        case 0xA3: return strdup("Evap System Vapor Pressure");
        case 0xA4: return strdup("Transmission Actual Gear");
        case 0xA5: return strdup("Commanded Diesel Exhaust Fluid Dosing");
        case 0xA6: return strdup("Odometer ");
        case 0xA7: return strdup("NOx Sensor Concentration Sensors 3 and 4");
        case 0xA8: return strdup("NOx Sensor Corrected Concentration Sensors 3 and 4");
        case 0xA9: return strdup("ABS Disable Switch State");
        case 0xC0: return strdup("PIDs supported [C1 - E0]");    
    }
    return null;
}

char * saej1979_data_fuel_system_status_code_to_str(final int fuel_system_status_code) {
    switch(fuel_system_status_code) {
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_MOTOR_OFF: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_MOTOR_OFF);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_CLOSED_LOOP_USING_FEEDBACK: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_USING_FEEDBACK);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_SYSTEM_FAILURE: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_SYSTEM_FAILURE);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_CLOSED_LOOP_WITH_FEEDBACK_FAULT: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_WITH_FEEDBACK_FAULT);
        default:
            log_msg(LOG_ERROR, "Unknown fuel system status code");
            return null;
    }
}

char * saej1979_data_secondary_air_status_to_string(SAEJ1979_DATA_SECONDARY_AIR_STATUS data) {
    switch(data) {
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_UPSTREAM: return strdup("Upstream");
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_DOWNSTREAM: return strdup("Downstream of catalytic converter");
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_OUTSIDE_OR_OFF: return strdup("From the outside atmosphere or off");
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_PUMP: return strdup("Pump commanded on for diagnostics");
        default:
            return null;
    }
}

bool saej1979_data_is_pid_supported(final VehicleIFace* iface, int pid) {
    return saej1979_is_pid_supported(iface, 0x01, pid);
}
char* saej1979_data_engine_type_as_string(final VehicleIFace* iface, int dataFrameNumber) {
    return strdup(SAEJ1979_DATA_ENGINE_TYPES_STR[saej1979_data_engine_spark_or_compression(iface,dataFrameNumber)]);
}

#define saej1979_data_status_iterator(data) \
    if ( 3 < data->size ) { \
        result = (byte*)malloc(sizeof(byte) * data->size); \
        memcpy(result, data->buffer, sizeof(byte) * data->size); \
    }
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte*,
                        saej1979_data_status,
                        "01",
                        saej1979_data_status_iterator,
                        null
                    )
int SAEJ1979_DATA_Test_cmp(SAEJ1979_DATA_Test* e1, SAEJ1979_DATA_Test* e2) {
    return e1 - e2;
}
AD_LIST_SRC(SAEJ1979_DATA_Test)

ad_list_SAEJ1979_DATA_Test *saej1979_data_tests_generic(final VehicleIFace* iface, int dataFrameNumber, byte* (*data_accessor)(final VehicleIFace* iface, int dataFrameNumber)) {
    ad_list_SAEJ1979_DATA_Test *list = ad_list_SAEJ1979_DATA_Test_new();
    byte * buffer = data_accessor(iface,dataFrameNumber);
    if ( buffer != null ) {
        byte B = buffer[1];
        char *names1[100] = {"Misfire","Fuel System", "Components"};
        for (int i = 0; i < 3; i++) {
            if ( bitRetrieve(B, i) ) {
                SAEJ1979_DATA_Test * data = (SAEJ1979_DATA_Test*)malloc(sizeof(SAEJ1979_DATA_Test));
                data->name = strdup(names1[i]);
                data->completed = ! bitRetrieve(B, 4 + i);
                ad_list_SAEJ1979_DATA_Test_append(list, data);
            }
        }
        bool isCompressionIgnition = bitRetrieve(B, 3);
        byte C = buffer[2];
        byte D = buffer[3];
        char ** names;
        char * namesCompressionIgnition[100] = {
            "EGR and/or VVT System","PM filter monitoring",
            "Exhaust Gas Sensor","- Reserved -","Boost Pressure","- Reserved -",
            "NOx/SCR Monitor","NMHC Catalyst"
        }, * namesSparkIgnition[100] = {
            "EGR System","Oxygen Sensor Heater",
            "Oxygen Sensor", "A/C Refrigerant",
            "Secondary Air System", "Evaporative System",
            "Heated Catalyst", "Catalyst"
        };
        names = isCompressionIgnition ? namesCompressionIgnition : namesSparkIgnition;
        for(int i = 0; i < 8; i++ ) {
            if ( bitRetrieve(C, i) ) {
                SAEJ1979_DATA_Test * data = (SAEJ1979_DATA_Test*)malloc(sizeof(SAEJ1979_DATA_Test));
                data->name = strdup(names[i]);
                data->completed = ! bitRetrieve(D, i);
                ad_list_SAEJ1979_DATA_Test_append(list, data);
            }
        }
        free(buffer);
    }
    return list;
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte*,
                        saej1979_data_status_this_cycle,
                        "41",
                        saej1979_data_status_iterator,
                        null
                    )

ad_list_SAEJ1979_DATA_Test *saej1979_data_tests(final VehicleIFace* iface, int dataFrameNumber, bool thisDriveCycleOnly) {
    if ( thisDriveCycleOnly ) {
        return saej1979_data_tests_generic(iface, dataFrameNumber, saej1979_data_status_this_cycle);
    } else {
        return saej1979_data_tests_generic(iface, dataFrameNumber, saej1979_data_status);
    }
}
bool saej1979_data_freeze_frame(final VehicleIFace* iface) {
    viface_lock(iface);
    bool result = viface_send_str(iface, "0102") <= 0;
    viface_clear_data(iface);
    result = ( viface_recv(iface) < 0 );
    viface_unlock(iface);
    return result;
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        char**,
                        saej1979_data_fuel_system_status,
                        "03",
                        saej1979_data_fuel_system_status_iterator,
                        null
                    )


SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DATA_SECONDARY_AIR_STATUS,
                        saej1979_data_secondary_air_status,
                        "12",
                        saej1979_data_secondary_air_status_iterator,
                        SAEJ1979_DATA_SECONDARY_AIR_STATUS_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte,
                        saej1979_data_oxygen_sensors_present_generic,
                        "13",
                        saej1979_data_oxygen_sensors_present_generic_iterator,
                        0
                    )
bool saej1979_data_oxygen_sensors_present(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    byte b = saej1979_data_oxygen_sensors_present_generic(iface,dataFrameNumber);
    int bitmask = 1 >> (sensor_i-1);
    return (bitmask & b) != 0;
}

/**
 * Service 0114 0115 0116 0117 0118 0119 011A 011B
 */
#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_voltage_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_1_voltage,
                        "14"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_2_voltage,
                        "15"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_3_voltage,
                        "16"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_4_voltage,
                        "17"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_5_voltage,
                        "18"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_6_voltage,
                        "19"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_7_voltage,
                        "1A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_8_voltage,
                        "1B"
                    )
double saej1979_data_oxygen_sensor_voltage(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_voltage(iface,dataFrameNumber);
        case 2: return saej1979_data_oxygen_sensor_2_voltage(iface,dataFrameNumber);
        case 3: return saej1979_data_oxygen_sensor_3_voltage(iface,dataFrameNumber);
        case 4: return saej1979_data_oxygen_sensor_4_voltage(iface,dataFrameNumber);
        case 5: return saej1979_data_oxygen_sensor_5_voltage(iface,dataFrameNumber);
        case 6: return saej1979_data_oxygen_sensor_6_voltage(iface,dataFrameNumber);
        case 7: return saej1979_data_oxygen_sensor_7_voltage(iface,dataFrameNumber);
        case 8: return saej1979_data_oxygen_sensor_8_voltage(iface,dataFrameNumber);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_ERROR;
}
#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_trim_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_TRIM_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_1_trim,
                        "14"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_2_trim,
                        "15"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_3_trim,
                        "16"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_4_trim,
                        "17"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_5_trim,
                        "18"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_6_trim,
                        "19"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_7_trim,
                        "1A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_8_trim,
                        "1B"
                    )
double saej1979_data_oxygen_sensor_trim(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_trim(iface,dataFrameNumber);
        case 2: return saej1979_data_oxygen_sensor_2_trim(iface,dataFrameNumber);
        case 3: return saej1979_data_oxygen_sensor_3_trim(iface,dataFrameNumber);
        case 4: return saej1979_data_oxygen_sensor_4_trim(iface,dataFrameNumber);
        case 5: return saej1979_data_oxygen_sensor_5_trim(iface,dataFrameNumber);
        case 6: return saej1979_data_oxygen_sensor_6_trim(iface,dataFrameNumber);
        case 7: return saej1979_data_oxygen_sensor_7_trim(iface,dataFrameNumber);
        case 8: return saej1979_data_oxygen_sensor_8_trim(iface,dataFrameNumber);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_TRIM_ERROR;
}

byte saej1979_data_oxygen_sensors_present_bank1(final VehicleIFace* iface,int dataFrameNumber) {
    return 0xF & saej1979_data_oxygen_sensors_present_generic(iface,dataFrameNumber);
}
byte saej1979_data_oxygen_sensors_present_bank2(final VehicleIFace* iface,int dataFrameNumber) {
    return (0xF0 & saej1979_data_oxygen_sensors_present_generic(iface,dataFrameNumber)) >> 4;
}
char * saej1979_data_obd_standard_convert_to_string(final SAEJ1979_DATA_OBD_STANDARD standard) {
    switch(standard) {
        case SAEJ1979_DATA_OBD_STANDARD_OBD_UNKNOWN:                 return strdup("Unknown");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_II_CARB:                 return strdup("OBD-II as defined by CARB");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_EPA:                     return strdup("OBD as defined by EPA");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_AND_OBD_II:              return strdup("OBD and OBD-II");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_I:                       return strdup("OBD-I");
        case SAEJ1979_DATA_OBD_STANDARD_NOT_OBD:                     return strdup("Not OBD compliant");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD:                        return strdup("EOBD (Europe)");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD_II:             return strdup("EOBD and OBD-II");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD:                return strdup("EOBD and OBD");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD_AND_OBD_II:     return strdup("EOBD, OBD and OBD-II");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD:                        return strdup("JOBD (Japan)");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_OBD_II:             return strdup("JOBD and OBD II");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_EOBD:               return strdup("JOBD and EOBD");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_EOBD_AND_OBD_II:    return strdup("JOBD, EOBD, OBD II");
        case SAEJ1979_DATA_OBD_STANDARD_EMD:                         return strdup("Engine Manufacturer Diagnostics (EMD)");
        case SAEJ1979_DATA_OBD_STANDARD_EMD_ENHANCED:                return strdup("Engine Manufacturer Diagnostics Enhanced (EMD+)");
        case SAEJ1979_DATA_OBD_STANDARD_HEAVY_DUTY_OBD_C:            return strdup("Heavy Duty On-Board Diagnostics (Child/Partial) (HD OBD-C)");
        case SAEJ1979_DATA_OBD_STANDARD_HEAVY_DUTY_OBD:              return strdup("Heavy Duty On-Board Diagnostics (HD OBD)");
        case SAEJ1979_DATA_OBD_STANDARD_WWH_OBD:                     return strdup("World Wide Harmonized OBD (WWH OBD)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD:                     return strdup("Heavy Duty Euro Stage I without NOx control (HD EOBD-I)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_N:                   return strdup("Heavy Duty Euro OBD Start I with NOx control (HD EOBD-I N)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_II:                  return strdup("Heavy Duty Euro Stage II without NOx control (HD EOBD-II)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_II_N:                return strdup("Heavy Duty Euro OBD Start II with NOx control (HD EOBD-II N)");
        case SAEJ1979_DATA_OBD_STANDARD_OBDBR_1:                     return strdup("Brazil OBD Phase 1 (OBDBr-1)");
        case SAEJ1979_DATA_OBD_STANDARD_OBDBR_2:                     return strdup("Brazil OBD Phase 2 (OBDBr-2)");
        case SAEJ1979_DATA_OBD_STANDARD_KOBD:                        return strdup("Korean OBD (KOBD)");
        case SAEJ1979_DATA_OBD_STANDARD_IOBD:                        return strdup("India OBD I (IOBD I)");
        case SAEJ1979_DATA_OBD_STANDARD_IOBD_II:                     return strdup("India OBD II (IOBD II)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_VI:                  return strdup("Heavy Duty Euro OBD Stage VI (HD EOBD-IV)");
    }
    char *res;
    asprintf(&res, "Unknown (%d)", standard);
    return res;
}
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DATA_OBD_STANDARD,
                        saej1979_data_obd_standard,
                        "1C",
                        saej1979_data_obd_standard_iterator,
                        SAEJ1979_DATA_OBD_STANDARD_OBD_UNKNOWN
                    )
char * saej1979_data_obd_standard_as_string(final VehicleIFace* iface,int dataFrameNumber) {
    return saej1979_data_obd_standard_convert_to_string(saej1979_data_obd_standard(iface,dataFrameNumber));
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte,
                        saej1979_data_oxygen_sensors_present_2_generic,
                        "1D",
                        saej1979_data_oxygen_sensors_present_2_generic_iterator,
                        0
                    )
bool saej1979_data_oxygen_sensors_present_2(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    byte b = saej1979_data_oxygen_sensors_present_2_generic(iface,dataFrameNumber);

    int bitmask = 1 >> (sensor_i-1);

    return (bitmask & b) != 0;
}

char* saej1979_get_seconds_to_time(final int seconds) {
    final int hours = seconds/3600;
    final int minutes = (seconds % 3600)/60;
    final int secs = seconds % 60;
    char * res;
    if ( 0 < hours ) {
        asprintf(&res, "%d hours %d minutes %d seconds", hours, minutes, secs);
    } else {
        if ( 0 < minutes ) {
            asprintf(&res, "%d minutes %d seconds", minutes, secs);
        } else {
            asprintf(&res, "%d seconds", secs);
        }
    }
    return res;
}
char* saej1979_data_time_since_engine_start(final VehicleIFace* iface, int dataFrameNumber) {
    final int seconds = saej1979_data_seconds_since_engine_start(iface,dataFrameNumber);
    return seconds == -1 ? null : saej1979_get_seconds_to_time(seconds);
}

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_1_air_fuel_equiv_ratio,
                        "24"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_2_air_fuel_equiv_ratio,
                        "25"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_3_air_fuel_equiv_ratio,
                        "26"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_4_air_fuel_equiv_ratio,
                        "27"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_5_air_fuel_equiv_ratio,
                        "28"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_6_air_fuel_equiv_ratio,
                        "29"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_7_air_fuel_equiv_ratio,
                        "2A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_8_air_fuel_equiv_ratio,
                        "2B"
                    )
double saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_air_fuel_equiv_ratio(iface,dataFrameNumber);
        case 2: return saej1979_data_oxygen_sensor_2_air_fuel_equiv_ratio(iface,dataFrameNumber);
        case 3: return saej1979_data_oxygen_sensor_3_air_fuel_equiv_ratio(iface,dataFrameNumber);
        case 4: return saej1979_data_oxygen_sensor_4_air_fuel_equiv_ratio(iface,dataFrameNumber);
        case 5: return saej1979_data_oxygen_sensor_5_air_fuel_equiv_ratio(iface,dataFrameNumber);
        case 6: return saej1979_data_oxygen_sensor_6_air_fuel_equiv_ratio(iface,dataFrameNumber);
        case 7: return saej1979_data_oxygen_sensor_7_air_fuel_equiv_ratio(iface,dataFrameNumber);
        case 8: return saej1979_data_oxygen_sensor_8_air_fuel_equiv_ratio(iface,dataFrameNumber);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_ERROR;
}
#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_voltage_ext_range_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_1_voltage_ext_range,
                        "24"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_2_voltage_ext_range,
                        "25"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_3_voltage_ext_range,
                        "26"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_4_voltage_ext_range,
                        "27"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_5_voltage_ext_range,
                        "28"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_6_voltage_ext_range,
                        "29"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_7_voltage_ext_range,
                        "2A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_8_voltage_ext_range,
                        "2B"
                    )
double saej1979_data_oxygen_sensor_voltage_ext_range(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_voltage_ext_range(iface,dataFrameNumber);
        case 2: return saej1979_data_oxygen_sensor_2_voltage_ext_range(iface,dataFrameNumber);
        case 3: return saej1979_data_oxygen_sensor_3_voltage_ext_range(iface,dataFrameNumber);
        case 4: return saej1979_data_oxygen_sensor_4_voltage_ext_range(iface,dataFrameNumber);
        case 5: return saej1979_data_oxygen_sensor_5_voltage_ext_range(iface,dataFrameNumber);
        case 6: return saej1979_data_oxygen_sensor_6_voltage_ext_range(iface,dataFrameNumber);
        case 7: return saej1979_data_oxygen_sensor_7_voltage_ext_range(iface,dataFrameNumber);
        case 8: return saej1979_data_oxygen_sensor_8_voltage_ext_range(iface,dataFrameNumber);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_ERROR;
}

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        int, sym, obd_request, \
                        saej1979_data_oxygen_sensor_current_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_1,
                        "34"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_2,
                        "35"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_3,
                        "36"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_4,
                        "37"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_5,
                        "38"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_6,
                        "39"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_7,
                        "3A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_8,
                        "3B"
                    )
int saej1979_data_oxygen_sensor_current(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_current_1(iface,dataFrameNumber);
        case 2: return saej1979_data_oxygen_sensor_current_2(iface,dataFrameNumber);
        case 3: return saej1979_data_oxygen_sensor_current_3(iface,dataFrameNumber);
        case 4: return saej1979_data_oxygen_sensor_current_4(iface,dataFrameNumber);
        case 5: return saej1979_data_oxygen_sensor_current_5(iface,dataFrameNumber);
        case 6: return saej1979_data_oxygen_sensor_current_6(iface,dataFrameNumber);
        case 7: return saej1979_data_oxygen_sensor_current_7(iface,dataFrameNumber);
        case 8: return saej1979_data_oxygen_sensor_current_8(iface,dataFrameNumber);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_ERROR;
}

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        int, sym, obd_request, \
                        saej1979_data_catalyst_temperature_generic_iterator, SAEJ1979_DATA_CATALYST_TEMPERATURE_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_1_temperature,
                        "3C"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_2_temperature,
                        "3D"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_3_temperature,
                        "3E"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_4_temperature,
                        "3F"
                    )
int saej1979_data_catalyst_tempature_with_bank(final VehicleIFace* iface, int dataFrameNumber, final int bank_i, final int sensor_i) {
    if ( sensor_i == 1 ) {
        if ( bank_i == 1 ) {
            return saej1979_data_catalyst_1_temperature(iface,dataFrameNumber);            
        } else if ( bank_i == 2 ) {
            return saej1979_data_catalyst_2_temperature(iface,dataFrameNumber);        
        }
    } else if ( sensor_i == 2 ) {
        if ( bank_i == 1 ) {
            return saej1979_data_catalyst_3_temperature(iface,dataFrameNumber);            
        } else if ( bank_i == 2 ) {
            return saej1979_data_catalyst_4_temperature(iface,dataFrameNumber);        
        }
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_CATALYST_TEMPERATURE_ERROR;
}

int saej1979_data_catalyst_tempature(final VehicleIFace* iface, int dataFrameNumber, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_catalyst_1_temperature(iface,dataFrameNumber);
        case 2: return saej1979_data_catalyst_2_temperature(iface,dataFrameNumber);
        case 3: return saej1979_data_catalyst_3_temperature(iface,dataFrameNumber);
        case 4: return saej1979_data_catalyst_4_temperature(iface,dataFrameNumber);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_CATALYST_TEMPERATURE_ERROR;
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_absolute_throttle_position_B,
                        "47")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_absolute_throttle_position_C,
                        "48")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_accelerator_pedal_position_D,
                        "49")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_accelerator_pedal_position_E,
                        "4A")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_accelerator_pedal_position_F,
                        "4B")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_commanded_throttle_actuator,
                        "4C")
#define saej1979_data_time_run_mil_on_iterator(data) \
    if ( 1 < data->size ) \
        result = ad_buffer_to_be16(data);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_time_run_mil_on,
                        "4D",
                        saej1979_data_time_run_mil_on_iterator,
                        SAEJ1979_DATA_TIME_RUN_MIL_ON_ERROR
                    )
#define saej1979_data_time_since_trouble_codes_cleared_iterator(data) \
    if ( 1 < data->size ) \
        result = ad_buffer_to_be16(data);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_time_since_trouble_codes_cleared,
                        "4E",
                        saej1979_data_time_since_trouble_codes_cleared_iterator,
                        SAEJ1979_DATA_TIME_SINCE_TROUBLE_CODES_CLEARED_ERROR
                    )
#define saej1979_data_max_air_flow_rate_from_maf_sensor_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0] * 10;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_max_air_flow_rate_from_maf_sensor,
                        "50",
                        saej1979_data_max_air_flow_rate_from_maf_sensor_iterator,
                        SAEJ1979_DATA_MAX_AIR_FLOW_RATE_FROM_MAF_SENSOR_ERROR
                    )

char * saej1979_data_fuel_type_convert_to_string(SAEJ1979_DATA_FUEL_TYPE t) {
    switch(t) {
        default:
        case SAEJ1979_DATA_FUEL_TYPE_NOT_AVALIABLE:                 break;
        case SAEJ1979_DATA_FUEL_TYPE_GASOLINE:                      return strdup("Gasoline");
        case SAEJ1979_DATA_FUEL_TYPE_METHANOL:                      return strdup("Methanol");
        case SAEJ1979_DATA_FUEL_TYPE_ETHANOL:                       return strdup("Ethanol");
        case SAEJ1979_DATA_FUEL_TYPE_DIESEL:                        return strdup("Diesel");
        case SAEJ1979_DATA_FUEL_TYPE_LPG:                           return strdup("LPG");
        case SAEJ1979_DATA_FUEL_TYPE_CNG:                           return strdup("CNG");
        case SAEJ1979_DATA_FUEL_TYPE_PROPANE:                       return strdup("Propane");
        case SAEJ1979_DATA_FUEL_TYPE_ELECTRIC:                      return strdup("Electric");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_GASOLINE:               return strdup("Bifuel running Gasoline");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_METHANOL:               return strdup("Bifuel running Methanol");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ETHANOL:                return strdup("Bifuel running Ethanol");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_LPG:                    return strdup("Bifuel running LPG");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_CNG:                    return strdup("Bifuel running CNG");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_PROPANE:                return strdup("Bifuel running Propane");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ELECTRICITY:            return strdup("Bifuel running Electricity");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ELECTRICITY_COMBUSTION: return strdup("Bifuel running electric and combustion engine");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_GASOLINE:               return strdup("Hybrid Gasoline");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_ETHANOL:                return strdup("Hybrid Ethanol");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_DIESEL:                 return strdup("Hybrid Diesel");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_ELECTRIC:               return strdup("Hybrid Electric");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_ELECTRIC_COMBUSTION:    return strdup("Hybrid running electric and combustion engine");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_REGENERATIVE:           return strdup("Hybrid Regenerative");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_DIESEL:                 return strdup("Bifuel runnig diesel");
    }
    return strdup("Not available");
}
#define saej1979_data_fuel_type_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0];
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DATA_FUEL_TYPE,saej1979_data_fuel_type,
                        "51",
                        saej1979_data_fuel_type_iterator,
                        SAEJ1979_DATA_FUEL_TYPE_NOT_AVALIABLE
                    )
char * saej1979_data_fuel_type_as_string(final VehicleIFace* iface, int dataFrameNumber) {
    return saej1979_data_fuel_type_convert_to_string(saej1979_data_fuel_type(iface, dataFrameNumber));
}

#define saej1979_data_relative_evap_system_vapor_pressure_iterator(data) \
    if ( 1 < data->size ) result = ad_buffer_to_be16(data);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_relative_evap_system_vapor_pressure,
                        "54",
                        saej1979_data_relative_evap_system_vapor_pressure_iterator,
                        SAEJ1979_DATA_RELATIVE_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR
                    )

#define saej1979_data_secondary_oxygen_sensor_trim_iterator(data) \
    if ( 0 < data->size ) result = (100/128.0) * data->buffer[0] - 100;
#define saej1979_data_secondary_oxygen_sensor_trim_2_iterator(data) \
    if ( 1 < data->size ) result = (100/128.0) * data->buffer[1] - 100;

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_1,
                        "55",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_2,
                        "57",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )                    
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_3,
                        "55",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )  
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_4,
                        "57",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    ) 
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_1,
                        "56",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_2,
                        "58",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )                    
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_3,
                        "56",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )  
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_4,
                        "58",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )                                                              
int saej1979_data_short_term_secondary_oxygen_sensor_trim(final VehicleIFace* iface, int dataFrameNumber, int bank_i) {
    assert(1 <= bank_i && bank_i <= 4);
    switch(bank_i) {
        case 1: return saej1979_data_short_term_secondary_oxygen_sensor_trim_1(iface, dataFrameNumber);
        case 2: return saej1979_data_short_term_secondary_oxygen_sensor_trim_2(iface, dataFrameNumber);
        case 3: return saej1979_data_short_term_secondary_oxygen_sensor_trim_3(iface, dataFrameNumber);
        case 4: return saej1979_data_short_term_secondary_oxygen_sensor_trim_4(iface, dataFrameNumber);
    }
    return SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR;
}

int saej1979_data_long_term_secondary_oxygen_sensor_trim(final VehicleIFace* iface, int dataFrameNumber, int bank_i) {
    assert(1 <= bank_i && bank_i <= 4);
    switch(bank_i) {
        case 1: return saej1979_data_long_term_secondary_oxygen_sensor_trim_1(iface, dataFrameNumber);
        case 2: return saej1979_data_long_term_secondary_oxygen_sensor_trim_2(iface, dataFrameNumber);
        case 3: return saej1979_data_long_term_secondary_oxygen_sensor_trim_3(iface, dataFrameNumber);
        case 4: return saej1979_data_long_term_secondary_oxygen_sensor_trim_4(iface, dataFrameNumber);
    }
    return SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR;
}

#define saej1979_data_maf_sensor_present_1_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_maf_sensor_present_1,
                    "66",
                    saej1979_data_maf_sensor_present_1_iterator,
                    false
                )
#define saej1979_data_maf_sensor_present_2_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_maf_sensor_present_2,
                    "66",
                    saej1979_data_maf_sensor_present_2_iterator,
                    false
                )                
bool saej1979_data_maf_sensor_present(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_maf_sensor_present_1(iface, dataFrameNumber);
        case 2: return saej1979_data_maf_sensor_present_2(iface, dataFrameNumber);
    }
    return false;
}
#define saej1979_data_maf_sensor_1_iterator(data) \
    if ( 2 < data->size ) result = (data->buffer[1] * 256 + data->buffer[2])/32;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_maf_sensor_1,
                    "66",
                    saej1979_data_maf_sensor_1_iterator,
                    SAEJ1979_DATA_MAF_SENSOR_ERROR
                ) 
#define saej1979_data_maf_sensor_2_iterator(data) \
    if ( 4 < data->size ) result = (data->buffer[3] * 256 + data->buffer[4])/32;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_maf_sensor_2,
                    "66",
                    saej1979_data_maf_sensor_2_iterator,
                    SAEJ1979_DATA_MAF_SENSOR_ERROR
                )                 
int saej1979_data_maf_sensor(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_maf_sensor_1(iface, dataFrameNumber);
        case 2: return saej1979_data_maf_sensor_2(iface, dataFrameNumber);
    }
    return SAEJ1979_DATA_MAF_SENSOR_ERROR;
}

#define saej1979_data_engine_coolant_temperature_sensor_present_1_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_coolant_temperature_sensor_present_1,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_present_1_iterator,
                    false
                )
#define saej1979_data_engine_coolant_temperature_sensor_present_2_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_coolant_temperature_sensor_present_2,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_present_2_iterator,
                    false
                )                
bool saej1979_data_engine_coolant_temperature_sensor_present(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_coolant_temperature_sensor_present_1(iface, dataFrameNumber);
        case 2: return saej1979_data_engine_coolant_temperature_sensor_present_2(iface, dataFrameNumber);
    }
    return false;
}
#define saej1979_data_engine_coolant_temperature_sensor_1_iterator(data) \
    if ( 1 < data->size ) result = data->buffer[1] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_coolant_temperature_sensor_1,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_1_iterator,
                    SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_ERROR
                ) 
#define saej1979_data_engine_coolant_temperature_sensor_2_iterator(data) \
    if ( 2 < data->size ) result = data->buffer[2] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_coolant_temperature_sensor_2,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_2_iterator,
                    SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_ERROR
                )                 
int saej1979_data_engine_coolant_temperature_sensor(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_coolant_temperature_sensor_1(iface, dataFrameNumber);
        case 2: return saej1979_data_engine_coolant_temperature_sensor_2(iface, dataFrameNumber);
    }
    return SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_ERROR;
}

#define saej1979_data_engine_intake_air_temperature_sensor_present_1_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_intake_air_temperature_sensor_present_1,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_present_1_iterator,
                    false
                )
#define saej1979_data_engine_intake_air_temperature_sensor_present_2_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_intake_air_temperature_sensor_present_2,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_present_2_iterator,
                    false
                )                
bool saej1979_data_engine_intake_air_temperature_sensor_present(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_intake_air_temperature_sensor_present_1(iface, dataFrameNumber);
        case 2: return saej1979_data_engine_intake_air_temperature_sensor_present_2(iface, dataFrameNumber);
    }
    return false;
}
#define saej1979_data_engine_intake_air_temperature_sensor_1_iterator(data) \
    if ( 1 < data->size ) result = data->buffer[1] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_intake_air_temperature_sensor_1,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_1_iterator,
                    SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_SENSOR_ERROR
                ) 
#define saej1979_data_engine_intake_air_temperature_sensor_2_iterator(data) \
    if ( 2 < data->size ) result = data->buffer[2] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_intake_air_temperature_sensor_2,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_2_iterator,
                    SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_SENSOR_ERROR
                )                 
int saej1979_data_engine_intake_air_temperature_sensor(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_intake_air_temperature_sensor_1(iface, dataFrameNumber);
        case 2: return saej1979_data_engine_intake_air_temperature_sensor_2(iface, dataFrameNumber);
    }
    return SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_SENSOR_ERROR;
}

#define saej1979_egt_sensor_present_byte_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0];
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte,saej1979_egt_sensor_bank_1_present_byte,
        "78",
        saej1979_egt_sensor_present_byte_iterator,
        0
    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte,saej1979_egt_sensor_bank_2_present_byte,
        "79",
        saej1979_egt_sensor_present_byte_iterator,
        0
    )
bool saej1979_egt_sensor_present(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    final byte b;
    if ( sensor_i <= 4 ) {
        b = saej1979_egt_sensor_bank_1_present_byte(iface, dataFrameNumber);
    } else {
        b = saej1979_egt_sensor_bank_2_present_byte(iface, dataFrameNumber);
        sensor_i -= 4;
    }
    return bitRetrieve(b, sensor_i - 1);
}
#define saej1979_egt_sensor_temperature_iterator(data) \
    if ( 8 < data->size ) { \
        result = (byte*)malloc(sizeof(byte) * data->size); \
        memcpy(result, data->buffer, sizeof(byte) * data->size); \
    }
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte*,saej1979_egt_sensor_temperature_bank_1,
        "78",
        saej1979_egt_sensor_temperature_iterator,
        null
    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte*,saej1979_egt_sensor_temperature_bank_2,
        "79",
        saej1979_egt_sensor_temperature_iterator,
        null
    )
int saej1979_egt_sensor_temperature(final VehicleIFace* iface, int dataFrameNumber, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    byte *data;
    if ( sensor_i <= 4 ) {
        data = saej1979_egt_sensor_temperature_bank_1(iface, dataFrameNumber);
    } else {
        data = saej1979_egt_sensor_temperature_bank_2(iface, dataFrameNumber);
        sensor_i -= 4;
    }    
    if ( data == null ) {
        return SAEJ1979_EGT_SENSOR_TEMPERATURE_ERROR;
    } else {
        return (data[1 + sensor_i * 2] * 256 + data[1 + sensor_i * 2 + 1])/10 - 40;
    }
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_diesel_particulate_filter_temperature,
                    "7C",
                    saej1979_data_diesel_particulate_filter_temperature_iterator,
                    SAEJ1979_DATA_DIESEL_PARTICULATE_FILTER_TEMPERATURE_ERROR
                )
#define saej1979_data_transmission_actual_gear_present_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_transmission_actual_gear_present,
                    "A4",
                    saej1979_data_transmission_actual_gear_present_iterator,
                    false
                )
#define saej1979_data_transmission_actual_gear_iterator(data) \
    if ( 3 < data->size ) result = (256 * data->buffer[2] + data->buffer[3]) / 1000;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_transmission_actual_gear,
                    "A4",
                    saej1979_data_transmission_actual_gear_iterator,
                    SAEJ1979_DATA_TRANSMISSION_ACTUAL_GEAR_ERROR
                )
                
#define saej1979_data_commanded_diesel_exhaust_fluid_dosing_present_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_commanded_diesel_exhaust_fluid_dosing_present,
                    "A5",
                    saej1979_data_commanded_diesel_exhaust_fluid_dosing_present_iterator,
                    false
                )
#define saej1979_data_commanded_diesel_exhaust_fluid_dosing_iterator(data) \
    if ( 1 < data->size ) result = data->buffer[1] / 2;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_commanded_diesel_exhaust_fluid_dosing,
                    "A5",
                    saej1979_data_commanded_diesel_exhaust_fluid_dosing_iterator,
                    SAEJ1979_DATA_COMMANDED_DIESEL_EXHAUST_FLUID_DOSING_ERROR
                )

// WARNING: never attempt to free this map
static ad_object_hashmap_Ptr_string * _saej1979_data_data_gen_pid_map = null;

char* saej1979_data_data_gen_pid_map_get(void *key) {
    if ( _saej1979_data_data_gen_pid_map == null ) {
        _saej1979_data_data_gen_pid_map = ad_object_hashmap_Ptr_string_new();
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_number_of_dtc), ad_object_string_new_from("01"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_mil_status), ad_object_string_new_from("01"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_status), ad_object_string_new_from("01"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_status_this_cycle), ad_object_string_new_from("41"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_fuel_system_status), ad_object_string_new_from("03"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_load), ad_object_string_new_from("04"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_coolant_temperature), ad_object_string_new_from("05"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_fuel_pressure), ad_object_string_new_from("0A"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_intake_manifold_pressure), ad_object_string_new_from("0B"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_speed), ad_object_string_new_from("0C"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_vehicle_speed), ad_object_string_new_from("0D"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_timing_advance_cycle_1), ad_object_string_new_from("0E"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_intake_air_temperature), ad_object_string_new_from("0F"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_maf_air_flow_rate), ad_object_string_new_from("10"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_throttle_position), ad_object_string_new_from("11"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_secondary_air_status), ad_object_string_new_from("12"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_oxygen_sensors_present_generic), ad_object_string_new_from("13"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_obd_standard), ad_object_string_new_from("1C"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_oxygen_sensors_present_2_generic), ad_object_string_new_from("1D"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_powertakeoff_status), ad_object_string_new_from("1E"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_seconds_since_engine_start), ad_object_string_new_from("1F"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_distance_since_mil_activated), ad_object_string_new_from("21"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_frp_relative), ad_object_string_new_from("22"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_frp_widerange), ad_object_string_new_from("23"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_commanded_egr), ad_object_string_new_from("2C"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_egr_error), ad_object_string_new_from("2D"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_commanded_evap_purge), ad_object_string_new_from("2E"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_fuel_tank_level_input), ad_object_string_new_from("2F"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_warm_ups_since_ecu_reset), ad_object_string_new_from("30"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_distance_since_ecu_reset), ad_object_string_new_from("31"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_evap_system_vapor_pressure), ad_object_string_new_from("32"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_baro_pressure_absolute), ad_object_string_new_from("33"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_ecu_voltage), ad_object_string_new_from("42"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_absolute_engine_load), ad_object_string_new_from("43"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_commanded_air_fuel_equivalence_ratio), ad_object_string_new_from("44"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_relative_throttle_position), ad_object_string_new_from("45"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_ambient_air_temperature), ad_object_string_new_from("46"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_time_run_mil_on), ad_object_string_new_from("4D"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_time_since_trouble_codes_cleared), ad_object_string_new_from("4E"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_max_fuel_air_equiv_ratio), ad_object_string_new_from("4F"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_max_oxygen_sensor_voltage), ad_object_string_new_from("4F"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_max_oxygen_sensor_current), ad_object_string_new_from("4F"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_max_intake_manifold_absolute_pressure), ad_object_string_new_from("4F"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_max_air_flow_rate_from_maf_sensor), ad_object_string_new_from("50"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_fuel_type), ad_object_string_new_from("51"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_absolute_evap_system_vapor_pressure), ad_object_string_new_from("53"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_relative_evap_system_vapor_pressure), ad_object_string_new_from("54"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_short_term_secondary_oxygen_sensor_trim_1), ad_object_string_new_from("55"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_short_term_secondary_oxygen_sensor_trim_2), ad_object_string_new_from("57"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_short_term_secondary_oxygen_sensor_trim_3), ad_object_string_new_from("55"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_short_term_secondary_oxygen_sensor_trim_4), ad_object_string_new_from("57"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_long_term_secondary_oxygen_sensor_trim_1), ad_object_string_new_from("56"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_long_term_secondary_oxygen_sensor_trim_2), ad_object_string_new_from("58"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_long_term_secondary_oxygen_sensor_trim_3), ad_object_string_new_from("56"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_long_term_secondary_oxygen_sensor_trim_4), ad_object_string_new_from("58"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_fuel_rail_absolute_pressure), ad_object_string_new_from("59"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_oil_temperature), ad_object_string_new_from("5C"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_fuel_injection_timing), ad_object_string_new_from("5D"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_fuel_rate), ad_object_string_new_from("5E"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_driver_demand_engine_percent_torque), ad_object_string_new_from("61"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_actual_engine_percent_torque), ad_object_string_new_from("62"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_reference_torque), ad_object_string_new_from("63"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_percent_torque_data_idle), ad_object_string_new_from("64"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_maf_sensor_present_1), ad_object_string_new_from("66"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_maf_sensor_present_2), ad_object_string_new_from("66"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_maf_sensor_1), ad_object_string_new_from("66"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_maf_sensor_2), ad_object_string_new_from("66"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_coolant_temperature_sensor_present_1), ad_object_string_new_from("67"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_coolant_temperature_sensor_present_2), ad_object_string_new_from("67"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_coolant_temperature_sensor_1), ad_object_string_new_from("67"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_coolant_temperature_sensor_2), ad_object_string_new_from("67"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_intake_air_temperature_sensor_present_1), ad_object_string_new_from("68"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_intake_air_temperature_sensor_present_2), ad_object_string_new_from("68"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_intake_air_temperature_sensor_1), ad_object_string_new_from("68"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_intake_air_temperature_sensor_2), ad_object_string_new_from("68"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_egt_sensor_bank_1_present_byte), ad_object_string_new_from("78"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_egt_sensor_bank_2_present_byte), ad_object_string_new_from("79"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_egt_sensor_temperature_bank_1), ad_object_string_new_from("78"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_egt_sensor_temperature_bank_2), ad_object_string_new_from("79"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_diesel_particulate_filter_temperature), ad_object_string_new_from("7C"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_engine_friction_percent_torque), ad_object_string_new_from("8E"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_cylinder_fuel_rate), ad_object_string_new_from("A2"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_transmission_actual_gear_present), ad_object_string_new_from("A4"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_transmission_actual_gear), ad_object_string_new_from("A4"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_commanded_diesel_exhaust_fluid_dosing_present), ad_object_string_new_from("A5"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_commanded_diesel_exhaust_fluid_dosing), ad_object_string_new_from("A5"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_odometer), ad_object_string_new_from("A6"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_abs_switch_present), ad_object_string_new_from("A9"));
        ad_object_hashmap_Ptr_string_set(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(saej1979_data_abs_switch), ad_object_string_new_from("A9"));
    }
    ad_object_string * string = ad_object_hashmap_Ptr_string_get(_saej1979_data_data_gen_pid_map, ad_object_Ptr_new_from(key));
    if ( string == null ) {
        return null;
    } else {
        return strdup(string->data);
    }
}
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_load", double, saej1979_data_engine_load)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("coolant_temp", int, saej1979_data_engine_coolant_temperature)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_trim_short_term_bank_1", double, saej1979_data_short_term_fuel_trim_bank_1)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_trim_long_term_bank_1", double, saej1979_data_long_term_fuel_trim_bank_1)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_trim_short_term_bank_2", double, saej1979_data_short_term_fuel_trim_bank_2)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_trim_long_term_bank_2", double, saej1979_data_long_term_fuel_trim_bank_2)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_spark_or_compression", bool, saej1979_data_engine_spark_or_compression)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_pressure", int, saej1979_data_fuel_pressure)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_speed", double, saej1979_data_engine_speed)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("vehicle_speed", int, saej1979_data_vehicle_speed)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("number_of_dtc", int, saej1979_data_number_of_dtc)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("mil_status", bool, saej1979_data_mil_status)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("intake_manifold_pressure", int, saej1979_data_intake_manifold_pressure)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("timing_advance_cycle_1", double, saej1979_data_timing_advance_cycle_1)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("intake_air_temperature", int, saej1979_data_intake_air_temperature)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("maf_air_flow_rate", double, saej1979_data_maf_air_flow_rate)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("throttle_position", double, saej1979_data_throttle_position)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("powertakeoff_status", bool, saej1979_data_powertakeoff_status)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("seconds_since_engine_start", int, saej1979_data_seconds_since_engine_start)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("distance_since_mil_activated", int, saej1979_data_distance_since_mil_activated)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("frp_relative", double, saej1979_data_frp_relative)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("frp_widerange", int, saej1979_data_frp_widerange)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("commanded_egr", double, saej1979_data_commanded_egr)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("egr_error", double, saej1979_data_egr_error)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("commanded_evap_purge", double, saej1979_data_commanded_evap_purge)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_tank_level_input", double, saej1979_data_fuel_tank_level_input)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("warm_ups_since_ecu_reset", int, saej1979_data_warm_ups_since_ecu_reset)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("distance_since_ecu_reset", int, saej1979_data_distance_since_ecu_reset)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_ethanol_percent", double, saej1979_data_ethanol_fuel_percent)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("evap_system_vapor_pressure", double, saej1979_data_evap_system_vapor_pressure)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("baro_pressure_absolute", int, saej1979_data_baro_pressure_absolute)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("ecu_voltage", double, saej1979_data_ecu_voltage)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("absolute_engine_load", double, saej1979_data_absolute_engine_load)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("commanded_air_fuel_equivalence_ratio", double, saej1979_data_commanded_air_fuel_equivalence_ratio)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("relative_throttle_position", double, saej1979_data_relative_throttle_position)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("ambient_air_temperature", int, saej1979_data_ambient_air_temperature)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("max_fuel_air_equiv_ratio", int, saej1979_data_max_fuel_air_equiv_ratio)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("max_oxygen_sensor_voltage", int, saej1979_data_max_oxygen_sensor_voltage)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("max_oxygen_sensor_current", int, saej1979_data_max_oxygen_sensor_current)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("max_intake_manifold_absolute_pressure", int, saej1979_data_max_intake_manifold_absolute_pressure)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("absolute_evap_system_vapor_pressure", double, saej1979_data_absolute_evap_system_vapor_pressure)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_rail_absolute_pressure", int, saej1979_data_fuel_rail_absolute_pressure)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("relative_accelerator_pedal_position", double, saej1979_data_relative_accelerator_pedal_position)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("hybrid_battery_pack_remaining_life", double, saej1979_data_hybrid_battery_pack_remaining_life)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("fuel_injection_timing", double, saej1979_data_fuel_injection_timing)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_fuel_rate", double, saej1979_data_engine_fuel_rate)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("driver_demand_engine_percent_torque", int, saej1979_data_driver_demand_engine_percent_torque)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("actual_engine_percent_torque", int, saej1979_data_actual_engine_percent_torque)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_reference_torque", int, saej1979_data_engine_reference_torque)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_percent_torque_data_idle", int, saej1979_data_engine_percent_torque_data_idle)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_percent_torque_data_point_1", int, saej1979_data_engine_percent_torque_data_point_1)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_percent_torque_data_point_2", int, saej1979_data_engine_percent_torque_data_point_2)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_percent_torque_data_point_3", int, saej1979_data_engine_percent_torque_data_point_3)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_percent_torque_data_point_4", int, saej1979_data_engine_percent_torque_data_point_4)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_oil_temperature", int, saej1979_data_engine_oil_temperature)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("engine_friction_percent_torque", int, saej1979_data_engine_friction_percent_torque)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("cylinder_fuel_rate", double, saej1979_data_cylinder_fuel_rate)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("odometer", double, saej1979_data_odometer)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("abs_switch_present", bool, saej1979_data_abs_switch_present)
AD_SAEJ1979_LEGACY_FROM_SIGNAL("abs_switch", bool, saej1979_data_abs_switch)