#include "libTest.h"

bool testElm(final OBDIFace* iface) {
    {
        OBDIFace* tmp = fake_can_iface();
        ELMDevice * elm = (ELMDevice *)tmp->device;
        elm->printing_of_spaces = false;
        Buffer * buffer = buffer_new();
        buffer_append_bytes(buffer,"000102030405060708090A0B0C0D0E0F",32);
        Buffer * bin = elm_ascii_to_bin(elm,buffer);
        if ( bin->size != 16 ) return false;
        for(int i = 0; i < bin->size; i++) {
            if ( bin->buffer[i] != i ) return false;
        }
    }
    {
        OBDIFace* tmp = fake_can_iface();
        ELMDevice * elm = (ELMDevice *)tmp->device;        
        elm->printing_of_spaces = true;
        Buffer * buffer = buffer_new();
        buffer_append_bytes(buffer,"00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F",48);
        Buffer * bin = elm_ascii_to_bin(elm,buffer);
        if ( bin->size != 16 ) return false;
        for(int i = 0; i < bin->size; i++) {
            if ( bin->buffer[i] != i ) return false;
        }
    }
    return true;
}
