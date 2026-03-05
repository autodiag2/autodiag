#ifndef __LANG_OBJECT_H
#define __LANG_OBJECT_H

#include <stdlib.h>
#include <assert.h>

#define OBJECT(type) ad_object_##type
#define OBJECT_H(type, struct_def) \
    typedef struct { \
        struct_def; \
    } OBJECT(type); \
    OBJECT_H_NEW(type); \
    OBJECT_H_FREE(type); \
    OBJECT_H_ASSIGN(type); \
    OBJECT_H_COPY(type);

#define AD_OBJECT_SRC(type) \
    AD_OBJECT_SRC_COPY(type)

#define OBJECT_H_NEW(type) ad_object_##type * ad_object_##type##_new()
#define OBJECT_H_FREE(type) void ad_object_##type##_free(ad_object_##type *object)
#define OBJECT_H_ASSIGN(type) ad_object_##type * ad_object_##type##_assign(ad_object_##type *object, ad_object_##type *other)    
#define OBJECT_H_COPY(type) ad_object_##type * ad_object_##type##_copy(ad_object_##type * from)

#define AD_OBJECT_SRC_COPY(type) OBJECT_H_COPY(type) { \
    return ad_object_##type##_assign(ad_object_##type##_new(), from); \
}


#endif