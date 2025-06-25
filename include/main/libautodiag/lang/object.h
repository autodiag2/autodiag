#ifndef __LANG_OBJECT_H
#define __LANG_OBJECT_H

#define H_OBJECT(type) \
    H_OBJECT_NEW(type); \
    H_OBJECT_FREE(type);

#define H_OBJECT_NEW(type) object_##type * object_##type##_new()
#define H_OBJECT_FREE(type) void object_##type##_free(object_##type *object)

#endif