#ifndef __OBJECT_PTR_H
#define __OBJECT_PTR_H

#include "libautodiag/lang/object.h"
#include "libautodiag/lang/lang.h"

typedef struct {
    void * value;
} object_Ptr;

OBJECT_H(Ptr)
object_Ptr * object_Ptr_new_from(void* value);

#endif