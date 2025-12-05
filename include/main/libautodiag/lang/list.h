#ifndef __AD_LIST_H
#define __AD_LIST_H

#include <assert.h>
#include <stdlib.h>
#include "libautodiag/lang/lang.h"

#define AD_LIST_FREE_CONTIGUOUS(list)\
    if (list->list != null ) {\
        free(list->list);\
        list->size = 0;\
    }

#define AD_LIST_FOREACH(list_token_name,element_type,element_token_name,loop_context) \
    for(int list_element_index = 0; list_element_index < list_token_name->size; list_element_index++ ) { \
        element_type * element_token_name = list_token_name->list[list_element_index]; \
        loop_context \
    } \

/**
 * Used to define generic list - each new value is a pointer
 */
#define AD_LIST_H_STRUCT_DEEP(element_type, init_struct) \
    typedef struct { \
        int size; \
        element_type **list; \
        init_struct; \
    } list_##element_type;
#define AD_LIST_H_STRUCT(element_type) AD_LIST_H_STRUCT_DEEP(element_type,)

#define AD_LIST_H(element_type) AD_LIST_H_DEEP(element_type,)
#define AD_LIST_H_DEEP(element_type, init_struct) \
    AD_LIST_H_STRUCT_DEEP(element_type, init_struct) \
    AD_LIST_H_NEW(element_type); \
    AD_LIST_H_FREE(element_type); \
    AD_LIST_H_APPEND(element_type); \
    AD_LIST_H_REMOVE(element_type); \
    AD_LIST_H_REMOVE_AT(element_type); \
    AD_LIST_H_CONTAINS(element_type); \
    AD_LIST_H_INDEX_OF(element_type); \
    AD_LIST_H_CLEAR(element_type); \
    AD_LIST_H_OBJECT_CMP(element_type);

#define AD_LIST_SRC(element_type) AD_LIST_SRC_DEEP(element_type,,)
#define AD_LIST_SRC_DEEP(element_type, new, free) \
    AD_LIST_SRC_NEW(element_type, new) \
    AD_LIST_SRC_FREE(element_type, free) \
    AD_LIST_SRC_APPEND(element_type) \
    AD_LIST_SRC_REMOVE(element_type) \
    AD_LIST_SRC_REMOVE_AT(element_type) \
    AD_LIST_SRC_CONTAINS(element_type) \
    AD_LIST_SRC_INDEX_OF(element_type) \
    AD_LIST_H_SRC_CLEAR(element_type)

#define AD_LIST_H_CLEAR(element_type) void list_##element_type##_clear(list_##element_type * list)
#define AD_LIST_H_SRC_CLEAR(element_type) AD_LIST_H_CLEAR(element_type) { \
    while(0 < list->size) { \
        list_##element_type##_remove_at(list, list->size-1); \
    } \
}

#define AD_LIST_H_OBJECT_CMP(element_type) int element_type##_cmp(element_type * element2, element_type * element)

#define AD_LIST_H_INDEX_OF(element_type) int list_##element_type##_index_of(list_##element_type * list, element_type * element)
#define AD_LIST_SRC_INDEX_OF(element_type) AD_LIST_H_INDEX_OF(element_type) { \
    AD_LIST_FOREACH( \
        list, element_type, element2, \
        { \
            if ( element_type##_cmp(element2, element) == 0 ) { \
                return list_element_index; \
            } \
        } \
    ) \
    return -1; \
}
#define AD_LIST_H_CONTAINS(element_type) bool list_##element_type##_contains(list_##element_type * list, element_type * element)
#define AD_LIST_SRC_CONTAINS(element_type) AD_LIST_H_CONTAINS(element_type) { \
    return list_##element_type##_index_of(list, element) != -1; \
}

#define AD_LIST_H_NEW(element_type) list_##element_type* list_##element_type##_new()
#define AD_LIST_SRC_NEW(element_type, init) AD_LIST_H_NEW(element_type) { \
    list_##element_type* list = (list_##element_type*) malloc(sizeof(list_##element_type)); \
    list->size = 0; \
    list->list = null; \
    init; \
    return list; \
}

#define AD_LIST_H_FREE(element_type) void list_##element_type##_free(list_##element_type* list)
#define AD_LIST_SRC_FREE(element_type, free) AD_LIST_H_FREE(element_type) { \
   if (list->list != null ) {\
        free; \
        for(int i = 0; i < list->size; i ++) {\
            free(list->list[i]);\
        }\
        free(list->list);\
        list->size = 0;\
    } \
}

#define AD_LIST_H_APPEND(element_type) void list_##element_type##_append(list_##element_type * list, element_type* value) 
#define AD_LIST_SRC_APPEND(element_type) AD_LIST_H_APPEND(element_type) { \
    list->list = (element_type**)realloc(list->list, sizeof(element_type*) * (++list->size)); \
    list->list[list->size-1] = value; \
}

#define AD_LIST_H_EMPTY(element_type) void list_##element_type##_empty(list_##element_type * list)
#define AD_LIST_SRC_EMPTY(element_type, element_free) AD_LIST_H_EMPTY(element_type) { \
    assert(list != null); \
    for(int i = 0; i < list->size; i ++) { \
        if ( list->list[i] != null ) { \
            element_free(list->list[i]); \
            list->list[i] = null; \
        } \
    } \
    free(list->list); \
    list->list = null; \
    list->size = 0; \
}

/**
 * Where comparator has the shape bool sym(element_type* element, searched)
 * the type of searched depends of what the comparator decide to define
 */
#define AD_LIST_H_FIND(element_type, searched_type) element_type* list_##element_type##_find(list_##element_type* list, searched_type searched)
#define AD_LIST_SRC_FIND(element_type,searched_type,comparator) AD_LIST_H_FIND(element_type, searched_type) { \
    for(int i = 0; i < list->size; i++) {\
        if ( comparator(list->list[i],searched) ) { \
            return list->list[i]; \
        } \
    } \
    return null; \
}

/**
 * Remove an element given as a pointer from the given list using address based comparision
 */
#define AD_LIST_H_REMOVE(element_type) bool list_##element_type##_remove(list_##element_type* list, element_type* element)
#define AD_LIST_SRC_REMOVE(element_type) AD_LIST_H_REMOVE(element_type) { \
    int index = -1; \
    for(int i = 0; i < list->size; i++) {\
        if ( 0 <= index ) { \
            list->list[i-1] = list->list[i]; \
        } else if ( list->list[i] == element ) { \
            index = i; \
        } \
    } \
    if ( 0 <= index ) { \
        list->size--; \
        if ( list->size == 0 ) { \
            free(list->list); \
            list->list = null; \
        } else { \
            list->list = (element_type**)realloc(list->list, sizeof(element_type*) * (list->size)); \
        } \
    } \
    return 0 <= index; \
}

#define AD_LIST_H_REMOVE_AT(element_type) element_type* list_##element_type##_remove_at(list_##element_type* list, unsigned int i)
#define AD_LIST_SRC_REMOVE_AT(element_type) AD_LIST_H_REMOVE_AT(element_type) { \
    assert(list != null); \
    assert(i < list->size); \
    final element_type * res = list->list[i]; \
    for(unsigned int j = i; j < (list->size-1); j++) { \
        list->list[j] = list->list[j+1]; \
    } \
    list->size --; \
    return res; \
}

#endif
