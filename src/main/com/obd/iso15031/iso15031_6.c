#include "com/obd/iso15031/iso15031_6.h"

char * iso15031_dtc_type_to_string(final ISO15031_DTC_TYPE type) {
    switch(type) {
        case ISO15031_DTC_TYPE_POWERTRAIN:  return strdup("power train");
        case ISO15031_DTC_TYPE_CHASSIS:     return strdup("chassis");
        case ISO15031_DTC_TYPE_BODY:        return strdup("body");
        case ISO15031_DTC_TYPE_NETWORK:     return strdup("network");
    }
    return strdup("unknown");
}

bool iso15031_dtc_is_generic_code(char display_character_2) {
    switch(display_character_2) {
        case '0':
        case '2':
        case '3': 
            return true;
        case '1':
            return false;
    }
    assert(false);
    return false;
}

ISO15031_DTC_TYPE iso15031_dtc_first_letter_to_type(final char letter) {
    switch(letter) {
        case 'p':
        case 'P':   return ISO15031_DTC_TYPE_POWERTRAIN;
        case 'c':
        case 'C':   return ISO15031_DTC_TYPE_CHASSIS;
        case 'b':
        case 'B':   return ISO15031_DTC_TYPE_BODY;
        case 'u':
        case 'U':   return ISO15031_DTC_TYPE_NETWORK;
    }
    assert(false);
}

char iso15031_dtc_type_first_letter(final ISO15031_DTC_TYPE type) {
    switch(type) {
        case ISO15031_DTC_TYPE_POWERTRAIN:  return 'P';
        case ISO15031_DTC_TYPE_CHASSIS:     return 'C';
        case ISO15031_DTC_TYPE_BODY:        return 'B';
        case ISO15031_DTC_TYPE_NETWORK:     return 'U';
    }
    return '?';
}
char * iso15031_dtc_to_subsystem_string(final char * number) {
    if ( iso15031_dtc_is_generic_code(number[0]) ) {
        switch(number[1]) {
            case '1': return strdup("fuel & air metering");
            case '2': return strdup("fuel & air metering (injector circuit)");
            case '3': return strdup("ignition system or misfire");
            case '4': return strdup("auxiliary emission controls");
            case '5': return strdup("vehicle speed control & idle control system");
            case '6': return strdup("computer cutput circuit");
            case '7':
            case '8':
            case '9':
                return strdup("transmission (gearbox)");
            case 'A':
            case 'B':
            case 'C':
                return strdup("hybrid propulsion");
        }
        return strdup("unknown");
    } else {
        return strdup("manufacturer specific");
    }
}
