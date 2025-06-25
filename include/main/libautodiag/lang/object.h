#ifndef __LANG_OBJECT_H
#define __LANG_OBJECT_H

#define OBJECT_H(type) \
    OBJECT_H_NEW(type); \
    OBJECT_H_FREE(type);

#define OBJECT_H_NEW(type) object_##type * object_##type##_new()
#define OBJECT_H_FREE(type) void object_##type##_free(object_##type *object)

#endif