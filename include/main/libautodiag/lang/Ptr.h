#ifndef __AD_OBJECT_PTR_H
#define __AD_OBJECT_PTR_H

#include "libautodiag/lang/object.h"
#include "libautodiag/lang/lang.h"

AD_OBJECT_H(Ptr,
    void * value;
)
ad_object_Ptr * ad_object_Ptr_new_from(void* value);

#endif