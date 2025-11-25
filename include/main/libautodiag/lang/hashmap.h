#ifndef __HASHMAP_H
#define __HASHMAP_H

#include "libautodiag/lang/lang.h"
#include "libautodiag/lang/object.h"
#include "libautodiag/lang/list.h"

#define HASHMAP(key_type, value_type) OBJECT(hashmap_##key_type##_##value_type)

#define HASHMAP_SRC_NEW(key_type, value_type) OBJECT_H_NEW(hashmap_##key_type##_##value_type) { \
    HASHMAP(key_type, value_type) * object = (HASHMAP(key_type, value_type) *)malloc(sizeof(HASHMAP(key_type, value_type))); \
    object->size = 0; \
    object->keys = null; \
    object->values = null; \
    return object; \
}
#define HASHMAP_SRC_FREE(key_type, value_type) OBJECT_H_FREE(hashmap_##key_type##_##value_type) { \
    if ( object == null ) return; \
    if ( object->keys != null ) { \
        for(unsigned int i = 0; i < object->size; i++) { \
            object_##key_type##_free(object->keys[i]); \
        } \
        object->keys = null; \
    } \
    if ( object->values != null ) { \
        for(unsigned int i = 0; i < object->size; i++) { \
            object_##value_type##_free(object->values[i]); \
        } \
        object->values = null; \
    } \
    object->size = 0; \
}

#define HASHMAP_H_GET(key_type, value_type) OBJECT(value_type) * object_hashmap_##key_type##_##value_type##_get(HASHMAP(key_type,value_type) * hm, OBJECT(key_type) *key)
#define HASHMAP_SRC_GET(key_type, value_type) HASHMAP_H_GET(key_type, value_type) { \
    assert(hm != null); \
    for(unsigned int i = 0; i < hm->size; i++) { \
        if ( object_hashmap_##key_type##_##value_type##_key_comparator(hm->keys[i], key) == 0 ) { \
            return hm->values[i]; \
        } \
    } \
    return null; \
}
#define HASHMAP_H_SET(key_type, value_type) void object_hashmap_##key_type##_##value_type##_set(object_hashmap_##key_type##_##value_type * hm, OBJECT(key_type) *key, OBJECT(value_type) *value)
#define HASHMAP_SRC_SET(key_type, value_type) HASHMAP_H_SET(key_type, value_type) { \
    assert(hm != null); \
    bool found = false; \
    for(unsigned int i = 0; i < hm->size; i++) { \
        if ( object_hashmap_##key_type##_##value_type##_key_comparator(hm->keys[i], key) == 0 ) { \
            hm->values[i] = value; \
            found = true; \
        } \
    } \
    if ( ! found ) { \
        hm->size ++; \
        hm->keys = (OBJECT(key_type)**)realloc(hm->keys, sizeof(OBJECT(key_type)*) * hm->size); \
        hm->values = (OBJECT(value_type)**)realloc(hm->values, sizeof(OBJECT(value_type)*) * hm->size); \
        hm->keys[hm->size-1] = key; \
        hm->values[hm->size-1] = value; \
    } \
}

#define HASHMAP_H(key_type, value_type) \
    OBJECT_H(hashmap_##key_type##_##value_type, \
        OBJECT(key_type) ** keys; \
        OBJECT(value_type) ** values; \
        unsigned int size; \
    ); \
    HASHMAP_H_SET(key_type, value_type); \
    HASHMAP_H_GET(key_type, value_type); \

#define HASHMAP_SRC(key_type, value_type) \
    HASHMAP_SRC_SET(key_type, value_type) \
    HASHMAP_SRC_GET(key_type, value_type) \
    HASHMAP_SRC_NEW(key_type, value_type) \
    HASHMAP_SRC_FREE(key_type, value_type)

#endif