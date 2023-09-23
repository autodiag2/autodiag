#ifndef __THREAD_H
#define __THREAD_H

#include "globals.h"
#include "log.h"
#include  <pthread.h>

void thread_allocate_and_start(pthread_t** t, void*ptr);

#endif
