#ifndef __THREAD_H
#define __THREAD_H

#include  <pthread.h>
#include "log.h"

void thread_allocate_and_start(pthread_t** t, void*ptr);

#define THREAD_WRITE_DAEMON(sym,handle,cleanupHandle,thread) static void* sym(void *arg) { \
    pthread_cleanup_push(cleanupHandle, null); \
    handle(); \
    free(thread); \
    thread = null; \
    pthread_cleanup_pop(1); \
    return null; \
}

#define THREAD_CANCEL(thread) \
    if ( thread != null ) { \
        //pthread_cancel(*thread); \
        free(thread); \
        thread = null; \
    }

#endif
