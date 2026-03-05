#ifndef __AD_HASHMAP_H
#define __AD_HASHMAP_H

#include "libautodiag/lang/lang.h"
#include "libautodiag/lang/object.h"
#include "libautodiag/lang/list.h"

#define AD_HASHMAP(key_type, value_type) AD_OBJECT(hashmap_##key_type##_##value_type)

#define AD_HASHMAP_SRC_NEW(key_type, value_type) AD_OBJECT_H_NEW(hashmap_##key_type##_##value_type) { \
    AD_HASHMAP(key_type, value_type) * object = (AD_HASHMAP(key_type, value_type) *)malloc(sizeof(AD_HASHMAP(key_type, value_type))); \
    object->size = 0; \
    object->keys = null; \
    object->values = null; \
    return object; \
}
#define AD_HASHMAP_SRC_FREE(key_type, value_type) AD_OBJECT_H_FREE(hashmap_##key_type##_##value_type) { \
    if ( object == null ) return; \
    if ( object->keys != null ) { \
        for(unsigned i = 0; i < object->size; i++) { \
            ad_object_##key_type##_free(object->keys[i]); \
        } \
        object->keys = null; \
    } \
    if ( object->values != null ) { \
        for(unsigned i = 0; i < object->size; i++) { \
            ad_object_##value_type##_free(object->values[i]); \
        } \
        object->values = null; \
    } \
    object->size = 0; \
}

#define AD_HASHMAP_H_GET(key_type, value_type) AD_OBJECT(value_type) * ad_object_hashmap_##key_type##_##value_type##_get(AD_HASHMAP(key_type,value_type) * hm, AD_OBJECT(key_type) *key)
#define AD_HASHMAP_SRC_GET(key_type, value_type) AD_HASHMAP_H_GET(key_type, value_type) { \
    assert(hm != null); \
    for(unsigned i = 0; i < hm->size; i++) { \
        if ( ad_object_hashmap_##key_type##_##value_type##_key_comparator(hm->keys[i], key) == 0 ) { \
            return hm->values[i]; \
        } \
    } \
    return null; \
}
#define AD_HASHMAP_H_SET(key_type, value_type) void ad_object_hashmap_##key_type##_##value_type##_set(AD_HASHMAP(key_type,value_type) * hm, AD_OBJECT(key_type) *key, AD_OBJECT(value_type) *value)
#define AD_HASHMAP_SRC_SET(key_type, value_type) AD_HASHMAP_H_SET(key_type, value_type) { \
    assert(hm != null); \
    bool found = false; \
    for(unsigned i = 0; i < hm->size; i++) { \
        if ( ad_object_hashmap_##key_type##_##value_type##_key_comparator(hm->keys[i], key) == 0 ) { \
            hm->values[i] = value; \
            found = true; \
        } \
    } \
    if ( ! found ) { \
        hm->size ++; \
        hm->keys = (AD_OBJECT(key_type)**)realloc(hm->keys, sizeof(AD_OBJECT(key_type)*) * hm->size); \
        hm->values = (AD_OBJECT(value_type)**)realloc(hm->values, sizeof(AD_OBJECT(value_type)*) * hm->size); \
        hm->keys[hm->size-1] = key; \
        hm->values[hm->size-1] = value; \
    } \
}

#define AD_HASHMAP_H_CLEAR(key_type, value_type) void ad_object_hashmap_##key_type##_##value_type##_clear(AD_HASHMAP(key_type,value_type) * hm)
#define AD_HASHMAP_SRC_CLEAR(key_type, value_type) AD_HASHMAP_H_CLEAR(key_type, value_type) { \
    assert(hm != null); \
    for(unsigned i = 0; i < hm->size; i++) { \
        ad_object_##key_type##_free(hm->keys[i]); \
        ad_object_##value_type##_free(hm->values[i]); \
    } \
    hm->size = 0; \
    free(hm->keys); \
    hm->keys = null; \
    free(hm->values); \
    hm->values = null; \
}

#define AD_HASHMAP_H(key_type, value_type) \
    AD_OBJECT_H(hashmap_##key_type##_##value_type, \
        AD_OBJECT(key_type) ** keys; \
        AD_OBJECT(value_type) ** values; \
        unsigned size; \
    ); \
    AD_HASHMAP_H_SET(key_type, value_type); \
    AD_HASHMAP_H_GET(key_type, value_type); \
    AD_HASHMAP_H_CLEAR(key_type, value_type);

#define AD_HASHMAP_SRC(key_type, value_type) \
    AD_HASHMAP_SRC_SET(key_type, value_type) \
    AD_HASHMAP_SRC_GET(key_type, value_type) \
    AD_HASHMAP_SRC_NEW(key_type, value_type) \
    AD_HASHMAP_SRC_FREE(key_type, value_type) \
    AD_HASHMAP_SRC_CLEAR(key_type, value_type) \
    AD_OBJECT_SRC(hashmap_##key_type##_##value_type)

#endif