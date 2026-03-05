#ifndef __AD_OBJECT_INT_H
#define __AD_OBJECT_INT_H

#include "libautodiag/lang/object.h"

AD_OBJECT_H(Int,
    int value;
)
ad_object_Int * ad_object_Int_new_from(int value);

#endif