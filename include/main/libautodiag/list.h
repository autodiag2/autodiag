#ifndef __LIST_H
#define __LIST_H

#include <assert.h>
#include <stdlib.h>
#include "lang.h"

#define LIST_FREE_CONTIGUOUS(list)\
    if (list->list != null ) {\
        free(list->list);\
        list->size = 0;\
    }

#define LIST_FOREACH(list_token_name,element_type,element_token_name,loop_context) \
    for(int list_element_index = 0; list_element_index < list_token_name->size; list_element_index++ ) { \
        element_type * element_token_name = list_token_name->list[list_element_index]; \
        loop_context \
    } \

/**
 * Used to define generic list - each new value is a pointer
 */
#define LIST_H_STRUCT(element_type) \
    typedef struct { \
        int size; \
        element_type **list; \
    } element_type##_list;

#define LIST_H(element_type) \
    LIST_H_STRUCT(element_type) \
    LIST_H_NEW(element_type##_list); \
    LIST_H_FREE(element_type##_list); \
    LIST_H_APPEND(element_type##_list,element_type); \
    LIST_H_REMOVE(element_type##_list,element_type); \
    LIST_H_REMOVE_AT(element_type##_list,element_type);

#define LIST_SRC(element_type) \
    LIST_SRC_NEW(element_type##_list) \
    LIST_SRC_FREE(element_type##_list) \
    LIST_SRC_APPEND(element_type##_list,element_type) \
    LIST_SRC_REMOVE(element_type##_list,element_type) \
    LIST_SRC_REMOVE_AT(element_type##_list,element_type)

#define LIST_H_NEW(type) type* type##_new()
#define LIST_SRC_NEW(type) LIST_H_NEW(type) { \
    type* list = (type*) malloc(sizeof(type)); \
    list->size = 0; \
    list->list = null; \
    return list; \
}

#define LIST_H_FREE(type) void type##_free(type* list)
#define LIST_SRC_FREE(type) LIST_H_FREE(type) { \
   if (list->list != null ) {\
        for(int i = 0; i < list->size; i ++) {\
            free(list->list[i]);\
        }\
        free(list->list);\
        list->size = 0;\
    } \
}

#define LIST_H_APPEND(type,type_value) void type##_append(type * list, type_value* value) 
#define LIST_SRC_APPEND(type,type_value) LIST_H_APPEND(type,type_value) { \
    list->list = (type_value**)realloc(list->list, sizeof(type_value*) * (++list->size)); \
    list->list[list->size-1] = value; \
}

#define LIST_H_EMPTY(type) void type##_empty(type * list)
#define LIST_SRC_EMPTY(type, element_free) LIST_H_EMPTY(type) { \
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
 * Where comparator has the shape bool sym(type_value* element, searched)
 * the type of searched depends of what the comparator decide to define
 */
#define LIST_H_FIND(element_type, searched_type) element_type* element_type##_list_find(element_type##_list* list, searched_type searched)
#define LIST_SRC_FIND(element_type,searched_type,comparator) LIST_H_FIND(element_type, searched_type) { \
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
#define LIST_H_REMOVE(type,type_value) bool type##_remove(type* list, type_value* element)
#define LIST_SRC_REMOVE(type,type_value) LIST_H_REMOVE(type,type_value) { \
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
            list->list = (type_value**)realloc(list->list, sizeof(type_value*) * (list->size)); \
        } \
    } \
    return 0 <= index; \
}

#define LIST_H_REMOVE_AT(type, type_value) type_value* type##_remove_at(type* list, int i)
#define LIST_SRC_REMOVE_AT(type, type_value) LIST_H_REMOVE_AT(type,type_value) { \
    assert(list != null); \
    assert(i < list->size); \
    final type_value * res = list->list[i]; \
    for(int j = i; j < (list->size-1); j++) { \
        list->list[j] = list->list[j+1]; \
    } \
    list->size --; \
    return res; \
}

#endif
