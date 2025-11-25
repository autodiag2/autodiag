#ifndef __OBJECT_INT_H
#define __OBJECT_INT_H

#include "libautodiag/lang/object.h"

OBJECT_H(Int,
    int value;
)
object_Int * object_Int_new_from(int value);

#endif