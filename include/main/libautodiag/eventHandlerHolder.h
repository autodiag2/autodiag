#ifndef __LIBAUTODIAG_EVENT_HANDLER_HOLDER_H
#define __LIBAUTODIAG_EVENT_HANDLER_HOLDER_H

#include "libautodiag/lang/all.h"
#include <stdlib.h>

typedef struct {
    void **list;
    int size;
} EventHandlerHolder;

#define EHH_PTR(var) ((void*)var)

EventHandlerHolder * ehh_new();
void ehh_free(EventHandlerHolder * ehh);
void ehh_register(final EventHandlerHolder* ehh, void *ptr);

#define ehh_trigger(ehh, HANDLER_TYPE, ...) \
    {   \
        for(int ehh_i = 0; ehh_i < ehh->size; ehh_i++) { \
            (HANDLER_TYPE (ehh)->list[ehh_i])(null, __VA_ARGS__); \
        } \
    }

#endif