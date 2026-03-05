#ifndef __OBJECT_INT_H
#define __OBJECT_INT_H

#include "libautodiag/lang/object.h"

OBJECT_H(Int,
    int value;
)
ad_object_Int * ad_object_Int_new_from(int value);

#endif