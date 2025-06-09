#ifndef __ELM329_H
#define __ELM329_H

#include "libautodiag/com/obd/obd.h"
#include "../elm.h"

typedef enum {
    ELM329_PROTO_NONE, ELM329_PROTO_SAE_J1850_1,
    ELM329_PROTO_SAE_J1850_2, ELM329_PROTO_ISO_9141_2,
    ELM329_PROTO_ISO_14230_4_KWP2000_1, ELM329_PROTO_ISO_14230_4_KWP2000_2,
    ELM329_PROTO_ISO_15765_4_CAN_1,ELM329_PROTO_ISO_15765_4_CAN_2,
    ELM329_PROTO_ISO_15765_4_CAN_3,ELM329_PROTO_ISO_15765_4_CAN_4,
    ELM329_PROTO_SAEJ1939,ELM329_PROTO_USER1_CAN,
    ELM329_PROTO_USER2_CAN,ELM329_PROTO_SAEJ1939_2,
    ELM329_PROTO_USER4_CAN,ELM329_PROTO_USER5_CAN
} ELM329_PROTO;

typedef struct {
    ELMDevice;
    /**
     * Protocol used to communicate on that interface, some interfaces could communicate with multiple protocols.<br />
     * This data represent only the protocol currently in use.<br />
     * e.g. ISO 15765-4 CAN (11-bit ID, 500 kBit/s)<br />
     */
    ELM329_PROTO protocol;
} ELM329Device;

/**
 * Serial response type to a command
 */
#define ELM329_RESPONSE_OFFSET                  0xF000
#define ELM329_RESPONSE_SEARCHING               0xF000
#define ELM329_RESPONSE_STOPPED                 0xF001
#define ELM329_RESPONSE_NO_DATA                 0xF002
#define ELM329_RESPONSE_ACT_ALERT               0xF003
#define ELM329_RESPONSE_BUFFER_FULL             0xF004
#define ELM329_RESPONSE_CAN_ERROR               0xF005
#define ELM329_RESPONSE_DATA_ERROR              0xF006
#define ELM329_RESPONSE_DATA_ERROR_AT_LINE      0xF007
#define ELM329_INTERNAL_ERROR                   0xF008
#define ELM329_LOW_POWER                        0xF009
#define ELM329_LOW_VOLTAGE_RESET                0xF00A
#define ELM329_CAN_DATA_ERROR                   0xF00B
#define ELM329_UNABLE_TO_CONNECT                0xF00C

static int ELM329ResponseStrNumber = 13;
static char * ELM329ResponseStr[] = {
    "SEARCHING",        
    "STOPPED",          
    "NO DATA",          
    "ACT ALERT",        
    "BUFFER FULL",      
    "CAN ERROR",        
    "DATA ERROR",       
    "<DATA ERROR",      
    "ERR",              
    "LP ALERT",         
    "LV RESET",         
    "<RX ERROR",        
    "UNABLE TO CONNECT"
};

ELM329Device* elm329_new();
ELM329Device* elm329_new_from_serial(final Serial *serial);

#include "iso15765.h"

#endif
