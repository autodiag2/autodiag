#ifndef __AD_LANG_OBJECT_H
#define __AD_LANG_OBJECT_H

#include <stdlib.h>
#include <assert.h>

#define AD_OBJECT(type) ad_object_##type
#define AD_OBJECT_H(type, struct_def) \
    typedef struct { \
        struct_def; \
    } AD_OBJECT(type); \
    AD_OBJECT_H_NEW(type); \
    AD_OBJECT_H_FREE(type); \
    AD_OBJECT_H_ASSIGN(type); \
    AD_OBJECT_H_COPY(type);

#define AD_OBJECT_SRC(type) \
    AD_OBJECT_SRC_COPY(type)

#define AD_OBJECT_H_NEW(type) ad_object_##type * ad_object_##type##_new()
#define AD_OBJECT_H_FREE(type) void ad_object_##type##_free(ad_object_##type *object)
#define AD_OBJECT_H_ASSIGN(type) ad_object_##type * ad_object_##type##_assign(ad_object_##type *object, ad_object_##type *other)    
#define AD_OBJECT_H_COPY(type) ad_object_##type * ad_object_##type##_copy(ad_object_##type * from)

#define AD_OBJECT_SRC_COPY(type) AD_OBJECT_H_COPY(type) { \
    return ad_object_##type##_assign(ad_object_##type##_new(), from); \
}


#endif