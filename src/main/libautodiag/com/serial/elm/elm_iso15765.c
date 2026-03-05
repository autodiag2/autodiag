#include "libautodiag/com/serial/elm/elm_iso15765.h"
#include "libautodiag/com/serial/elm/elm.h"
#include "libautodiag/model/vehicle.h"

int oneHex(char c) {
    char hex[2] = {0};
    hex[0] = c;
    return (int)strtol(hex,null,16);

}

bool elm_iso15765_parse_response_internal(final void* elm_arg, ad_list_Iso15765Conversation* conversations, char * id_ascii, int id_sz_chars, void * vehicle_arg) {
    final ELMDevice* elm = (ELMDevice*)elm_arg;
    final Vehicle* vehicle = (Vehicle*)vehicle_arg;
    SERIAL_BUFFER_ITERATE(elm,ELM_ISO15765_PARSE_RESPONSE_ITERATOR);
    return true;
}