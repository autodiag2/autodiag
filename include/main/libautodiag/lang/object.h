#ifndef __LANG_OBJECT_H
#define __LANG_OBJECT_H

#include <stdlib.h>
#include <assert.h>

#define OBJECT(type) object_##type
#define OBJECT_H(type, struct_def) \
    typedef struct { \
        struct_def; \
    } OBJECT(type); \
    OBJECT_H_NEW(type); \
    OBJECT_H_FREE(type); \
    OBJECT_H_ASSIGN(type);

#define OBJECT_H_NEW(type) object_##type * object_##type##_new()
#define OBJECT_H_FREE(type) void object_##type##_free(object_##type *object)
#define OBJECT_H_ASSIGN(type) object_##type * object_##type##_assign(object_##type *object, object_##type *other)    


#endif