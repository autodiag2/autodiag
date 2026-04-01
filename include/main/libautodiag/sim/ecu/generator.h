#ifndef __AD_SIM_ECU_GENERATOR_H
#define __AD_SIM_ECU_GENERATOR_H

#include "libautodiag/lib.h"
#include "libautodiag/com/obd/obd.h"
#include "libautodiag/com/uds/uds.h"

typedef struct SimECUGenerator {
    /**
     * Context to be set by the caller for the generator to work.
     */
    void *context;
    /**
     * Plain text type of the generator (e.g., "Citroen C5 X7", "cycle", "random", etc.)
     */
    char *type;
    /**
     * Holds the state of the generator (like DTCs, session type, etc.)
     * it is not expected to be modified by external parts using the generator.
     */
    void *state;

    /**
     * Holds any information on ECU that may alter the way the generator respond.
     * Avoid to write a generator for CAN ECU, one generator for non CAN ECU.
     */
    struct {
        /**
         * ECU may respond slightly different.
         * eg. DTC_count for SID 03
         */
        bool is_Iso15765_4;
    } flavour ;
    
    /**
     * Generate response to a given pid (single unit).
     */
    Buffer * (*saej1979_response_pid)(struct SimECUGenerator *generator, final byte pid, int frameNumber);
    /**
     * Wrapp the result in a standard OBD frame.
     */
    Buffer * (*saej1979_response_pids)(struct SimECUGenerator *generator, final Buffer *binRequest);
    /**
     * Generates a response for the given request (OBD/UDS or any data protocol over for example CAN messages)
     * @param this Pointer to the SimECUGenerator instance
     * @param binRequest Pointer to the binary request buffer
     * @return Pointer to the binary response buffer
     */
    Buffer * (*response)(struct SimECUGenerator * this, final Buffer *binRequest);
    /**
     * Generates a response for the given request (OBD/UDS or any data protocol over for example CAN messages).
     * Response buffer is preallocated to prevent crash in python.
     * @param this Pointer to the SimECUGenerator instance
     * @param binRequest Pointer to the binary request buffer
     * @param binResponse Pointer to the preallocated response buffer
     */
    void (*response_for_python)(struct SimECUGenerator * this, final Buffer *binRequest, final Buffer * binResponse);
    /**
     * @return null in case context cannot be represented as string, the context string representation in case of success
     */
    char * (*context_to_string)(struct SimECUGenerator * this);
    /**
     * @return false in case a string representation is not enough to describe the context, true in case of success
     */
    bool (*context_load_from_string)(struct SimECUGenerator * this, char * context);
} SimECUGenerator;

void sim_ecu_generator_fill_nrc(Buffer * binResponse, Buffer * binRequest, byte nrc);
bool sim_ecu_generator_fill_success(Buffer * binResponse, Buffer * binRequest);
SimECUGenerator* sim_ecu_generator_new_cycle();
SimECUGenerator* sim_ecu_generator_new_random();
SimECUGenerator * sim_ecu_generator_new();
SimECUGenerator* sim_ecu_generator_new_citroen_c5_x7();
/**
 * @param address End byte for CAN 11 bits, 29 bits, OBD Message
 */
SimECUGenerator* sim_ecu_generator_new_replay();
#define SIM_ECU_GENERATOR_RESPONSE(f) ((Buffer* (*)(SimECUGenerator *, Buffer *))f)
#define SIM_ECU_GENERATOR_CONTEXT_TO_STRING(f) ((char* (*)(SimECUGenerator *))f)
#define SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(f) ((bool (*)(SimECUGenerator *, char*))f)

#endif