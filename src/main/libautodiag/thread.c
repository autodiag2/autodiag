#include "libautodiag/thread.h"

void thread_allocate_and_start(pthread_t** t, void*ptr) {
    if ( *t == null ) {
        *t = (pthread_t*)malloc(sizeof(pthread_t));
        if ( pthread_create(*t, null, ptr, null) == 0 ) {
            log_msg(LOG_INFO, "Checker thread created");
        } else {
            log_msg(LOG_ERROR, "Fatal erro during thead creation");
        }
    }
}
