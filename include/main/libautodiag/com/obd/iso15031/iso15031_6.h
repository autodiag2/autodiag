#ifndef __ISO15031_6_H
#define __ISO15031_6_H

#include "libautodiag/lib.h"

/**
 * ISO 15031-6
 * This part of ISO 15031 provides recommended uniformity for the alphanumeric trouble codes.
 * It further provides guidance for the uniform messages associated with these codes. It specifies
 * several sections addressing format, structure, messages and a few examples, and is applicable
 * to electrical/electronic systems diagnostics of motor vehicles.
 */
typedef enum {
    ISO15031_DTC_TYPE_POWERTRAIN, ISO15031_DTC_TYPE_CHASSIS,
    ISO15031_DTC_TYPE_BODY, ISO15031_DTC_TYPE_NETWORK, ISO15031_DTC_TYPE_UNKNOWN
} ISO15031_DTC_TYPE;

/**
 * Guess a string describing the type
 */
char * iso15031_dtc_type_to_string(final ISO15031_DTC_TYPE type);
/**
 * From the display character 2 output if the code is generic
 * eg P0103 -> 0
 */
bool iso15031_dtc_is_generic_code(char display_character_2);
/**
 * From a dtc type output the first letter
 */
char iso15031_dtc_type_first_letter(final ISO15031_DTC_TYPE type);
ISO15031_DTC_TYPE iso15031_dtc_first_letter_to_type(final char letter);
/**
 * Number is only digits from the dtc
 * eg P0103 -> 0103
 */
char * iso15031_dtc_to_subsystem_string(final char * number);

#endif
