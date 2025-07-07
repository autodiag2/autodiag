#ifndef __OBJECT_INT_H
#define __OBJECT_INT_H

#include "libautodiag/lang/object.h"

typedef struct {
    int value;
} object_Int;

OBJECT_H(Int)
object_Int * object_Int_new_from(int value);

#endif