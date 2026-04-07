#include "libTest.h"
#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/com/uds/uds.h"

bool testUDS() {
    SimECU * ecu = sim_ecu_new(0xE8);
    ecu->generator = sim_ecu_generator_new_citroen_c5_x7();
    final VehicleIFace* iface = tf_ad_serial_open(tf_sim_elm327_start_with_ecu(ecu));
    {
        assert(ad_uds_request_session_cond(iface, AD_UDS_SESSION_DEFAULT));
        assert(ad_uds_request_session_cond(iface, AD_UDS_SESSION_EXTENDED_DIAGNOSTIC));
        assert(ad_uds_request_session_cond(iface, AD_UDS_SESSION_PROGRAMMING));
        assert(ad_uds_request_session_cond(iface, AD_UDS_SESSION_SYSTEM_SAFETY_DIAGNOSTIC));
    }
    {
        Buffer * address = ad_buffer_from_ascii_hex("0001");
        Buffer * length = ad_buffer_from_ascii_hex("000F");
        Buffer * dataRecord = ad_uds_read_memory_by_address(iface, address, length);
        assert(dataRecord != null);
        Buffer * dataRecord2 = ad_uds_read_memory_by_address(iface, address, length);
        assert(dataRecord2 != null);
        assert(ad_buffer_cmp(dataRecord, dataRecord2) == 0);
        Buffer * writeRecord = ad_buffer_new_random(dataRecord->size);
        assert(ad_uds_write_memory_by_address(iface, address, length, writeRecord));
        Buffer * dataRecord3 = ad_uds_read_memory_by_address(iface, address, length);
        assert(ad_buffer_cmp(dataRecord3, writeRecord) == 0);
    }
    {
        Buffer * address = ad_buffer_from_ascii_hex("0001");
        Buffer * length = ad_buffer_from_ascii_hex("00F0");
        Buffer * dataRecord = ad_uds_read_memory_by_address(iface, address, length);
        assert(dataRecord != null);
        Buffer * dataRecord2 = ad_uds_read_memory_by_address(iface, address, length);
        assert(dataRecord2 != null);
        assert(ad_buffer_cmp(dataRecord, dataRecord2) == 0);
        Buffer * writeRecord = ad_buffer_new_random(dataRecord->size);
        assert(ad_uds_write_memory_by_address(iface, address, length, writeRecord));
        Buffer * dataRecord3 = ad_uds_read_memory_by_address(iface, address, length);
        assert(ad_buffer_cmp(dataRecord3, writeRecord) == 0);
    }
    {
        Buffer * address = ad_buffer_from_ascii_hex("0000");
        Buffer * length = ad_buffer_from_ascii_hex("1000");
        Buffer * writeRecord = ad_buffer_new_random(ad_buffer_to_be(length));
        assert(!ad_uds_write_memory_by_address(iface, address, length, writeRecord));
    }
    return true;
}