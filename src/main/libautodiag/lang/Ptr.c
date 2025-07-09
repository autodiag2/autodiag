#include "libautodiag/lang/Ptr.h"

void object_Ptr_free(object_Ptr *object) {
    assert(object);
    if (object) {
        free(object);
    }
}
object_Ptr *object_Ptr_new() {
    object_Ptr *obj = (object_Ptr *)malloc(sizeof(object_Ptr));
    assert(obj);
    obj->value = null;
    return obj;
}
object_Ptr * object_Ptr_assign(object_Ptr *object, object_Ptr *other) {
    assert(object);
    assert(other);
    object->value = other->value;
    return object;
}

object_Ptr * object_Ptr_new_from(void* value) {
    object_Ptr * o = object_Ptr_new();
    o->value = value;
    return o;
}