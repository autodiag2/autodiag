#include "thread.h"

void thread_allocate_and_start(pthread_t** t, void*ptr) {
    if ( *t == null ) {
        *t = (pthread_t*)malloc(sizeof(pthread_t));
        if ( pthread_create(*t, null, ptr, null) == 0 ) {
            log_msg("Checker thread created", LOG_INFO);
        } else {
            log_msg("Fatal erro during thead creation", LOG_ERROR);
        }
    }
}
