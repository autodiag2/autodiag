#include "libautodiag/com/uds/flash_utils.h"

ad_object_ecu_memory * ad_object_ecu_memory_new() {
    ad_object_ecu_memory * m = (ad_object_ecu_memory*)malloc(sizeof(ad_object_ecu_memory));
    m->address = ad_buffer_new();
    m->content = ad_buffer_new();
    m->ecu = null;
    m->length = ad_buffer_new();
    m->fmt_address = 0x00;
    m->fmt_length = 0x00;
    return m;
}
ad_object_ecu_memory * ad_object_ecu_memory_assign(ad_object_ecu_memory * to, ad_object_ecu_memory * from) {
    to->address = ad_buffer_copy(from->address);
    to->content = ad_buffer_copy(from->content);
    to->ecu = from->ecu;
    to->length = ad_buffer_copy(from->length);
    to->fmt_address = from->fmt_address;
    to->fmt_length = from->fmt_length;
    return to;
}
void ad_object_ecu_memory_free(ad_object_ecu_memory * m) {
    if ( m != null ) {
        ad_buffer_free(m->address);
        ad_buffer_free(m->content);
        ad_buffer_free(m->length);
        m->ecu = null;
        m->fmt_address = 0x00;
        m->fmt_length = 0x00;
        free(m);
    }
}
bool ad_uds_write_memory_by_address(VehicleIFace * iface, Buffer * address, Buffer * length, Buffer * memory) {
    if ( 0x0F < address->size ) {
        log_err("too long");
        return false;
    }
    if ( 0x0F < length->size ) {
        log_err("too long");
        return false;
    }
    if ( ! ad_uds_request_session_cond(iface, AD_UDS_SESSION_PROGRAMMING) ) {
        return false;
    }
    if ( ! ad_uds_security_access(iface, 0x01) ) {
        return false;
    }
    iface->lock(iface);
    Buffer * binRequest = ad_buffer_from_ints(
        AD_UDS_SERVICE_WRITE_MEMORY_BY_ADDRESS, 
        ((length->size << 4) & 0xF0) | (address->size & 0x0F)
    );
    ad_buffer_append(binRequest, address);
    ad_buffer_append(binRequest, length);
    ad_buffer_append(binRequest, memory);
    if ( iface->send(iface, binRequest) <= 0 ) {
        iface->unlock(iface);
        return false;
    }
    iface->clear_data(iface);
    if ( iface->recv(iface) <= 0 ) {
        iface->unlock(iface);
        return false;
    }
    bool result = false;
    for(int i = 0; i < iface->vehicle->data_buffer->size; i++) {
        Buffer * binResponse = iface->vehicle->data_buffer->list[i];
        if ( 0 < binResponse->size ) {
            byte b0 = binResponse->buffer[0];
            if ( b0 == (AD_UDS_SERVICE_WRITE_MEMORY_BY_ADDRESS | AD_UDS_POSITIVE_RESPONSE) ) {
                result = true;
                break;
            } else {
                log_warn("negative response");
            }
        } else {
            log_warn("null buffer");
        }
    }
    iface->unlock(iface);
    return result;
}
Buffer * ad_uds_read_memory_by_address(VehicleIFace * iface, Buffer * address, Buffer * length) {
    Buffer * memory = null;

    if ( 0x0F < address->size ) {
        log_err("too long");
        return null;
    }
    if ( 0x0F < length->size ) {
        log_err("too long");
        return null;
    }
    if ( ! ad_uds_request_session_cond(iface, AD_UDS_SESSION_PROGRAMMING) ) {
        return false;
    }
    if ( ! ad_uds_security_access(iface, 0x01) ) {
        return false;
    }
    iface->lock(iface);
    Buffer * binRequest = ad_buffer_from_ints(
        AD_UDS_SERVICE_READ_MEMORY_BY_ADDRESS, 
        ((length->size << 4) & 0xF0) | (address->size & 0x0F)
    );
    ad_buffer_append(binRequest, address);
    ad_buffer_append(binRequest, length);
    if ( iface->send(iface, binRequest) <= 0 ) {
        iface->unlock(iface);
        return null;
    }
    iface->clear_data(iface);
    if ( iface->recv(iface) <= 0 ) {
        iface->unlock(iface);
        return null;
    }
    if ( 1 < iface->vehicle->data_buffer->size ) {
        log_warn("multiple ecu reading is not supported, take the first response");
    }
    for(int i = 0; i < iface->vehicle->data_buffer->size; i++) {
        Buffer * binResponse = iface->vehicle->data_buffer->list[i];
        if ( 0 < binResponse->size ) {
            byte b0 = binResponse->buffer[0];
            if ( b0 == (AD_UDS_SERVICE_READ_MEMORY_BY_ADDRESS | AD_UDS_POSITIVE_RESPONSE) ) {
                memory = ad_buffer_slice(binResponse, 1, binResponse->size - 1);
                break;
            } else {
                log_warn("negative response: %s", ad_buffer_to_hex_string(binResponse));
            }
        } else {
            log_warn("null buffer");
        }
    }
    iface->unlock(iface);
    return memory;
}