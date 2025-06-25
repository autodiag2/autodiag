#ifndef __HASHMAP_H
#define __HASHMAP_H

#include "libautodiag/lang/lang.h"
#include "libautodiag/lang/list.h"

#define HASHMAP_H_STRUCT(key_type, value_type) \
    typedef struct { \
        key_type ** keys; \
        value_type ** values; \
        unsigned int size; \
    } hashmap_##key_type##_##value_type;

#define HASHMAP_H_NEW(key_type, value_type) hashmap_##key_type##_##value_type * hashmap_##key_type##_##value_type##_new()
#define HASHMAP_SRC_NEW(key_type, value_type) HASHMAP_H_NEW(key_type, value_type) { \
    hashmap_##key_type##_##value_type * hm = (hashmap_##key_type##_##value_type *)malloc(sizeof(hashmap_##key_type##_##value_type)); \
    hm->size = 0; \
    hm->keys = null; \
    hm->values = null; \
    return hm; \
}
#define HASHMAP_H_FREE(key_type, value_type) void hashmap_##key_type##_##value_type##_free(hashmap_##key_type##_##value_type *hm)
#define HASHMAP_SRC_FREE(key_type, value_type) HASHMAP_H_FREE(key_type, value_type) { \
    if ( hm == null ) return; \
    if ( hm->keys != null ) { \
        for(unsigned int i = 0; i < hm->size; i++) { \
            /* should call the key free operator */ \
            free(hm->keys[i]); \
        } \
        hm->keys = null; \
    } \
    if ( hm->values != null ) { \
        for(unsigned int i = 0; i < hm->size; i++) { \
            /* should call the value free operator */ \
            free(hm->values[i]); \
        } \
        hm->values = null; \
    } \
    hm->size = 0; \
}

#define HASHMAP_H_GET(key_type, value_type) value_type * hashmap_##key_type##_##value_type##_get(hashmap_##key_type##_##value_type * hm, key_type *key)
#define HASHMAP_SRC_GET(key_type, value_type) HASHMAP_H_GET(key_type, value_type) { \
    assert(hm != null); \
    for(unsigned int i = 0; i < hm->size; i++) { \
        if ( hashmap_##key_type##_##value_type##_key_comparator(hm->keys[i], key) == 0 ) { \
            return hm->values[i]; \
        } \
    } \
    return null; \
}
#define HASHMAP_H_SET(key_type, value_type) void hashmap_##key_type##_##value_type##_set(hashmap_##key_type##_##value_type * hm, key_type *key, value_type *value)
#define HASHMAP_SRC_SET(key_type, value_type) HASHMAP_H_SET(key_type, value_type) { \
    assert(hm != null); \
    bool found = false; \
    for(unsigned int i = 0; i < hm->size; i++) { \
        if ( hashmap_##key_type##_##value_type##_key_comparator(hm->keys[i], key) == 0 ) { \
            hm->values[i] = value; \
            found = true; \
        } \
    } \
    if ( ! found ) { \
        hm->size ++; \
        hm->keys = (value_type**)realloc(hm->keys, sizeof(value_type*) * hm->size); \
        hm->values = (value_type**)realloc(hm->values, sizeof(value_type*) * hm->size); \
        hm->keys[hm->size-1] = key; \
        hm->values[hm->size-1] = value; \
    } \
}

#define HASHMAP_H(key_type, value_type) \
    HASHMAP_H_STRUCT(key_type, value_type) \
    HASHMAP_H_SET(key_type, value_type); \
    HASHMAP_H_GET(key_type, value_type); \
    HASHMAP_H_NEW(key_type, value_type); \
    HASHMAP_H_FREE(key_type, value_type);

#define HASHMAP_SRC(key_type, value_type) \
    HASHMAP_SRC_SET(key_type, value_type) \
    HASHMAP_SRC_GET(key_type, value_type) \
    HASHMAP_SRC_NEW(key_type, value_type) \
    HASHMAP_SRC_FREE(key_type, value_type)

#endif