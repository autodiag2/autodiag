#include "libautodiag/lang/Int.h"

void object_Int_free(object_Int *object) {
    assert(object);
    if (object) {
        free(object);
    }
}
object_Int *object_Int_new() {
    object_Int *obj = (object_Int *)malloc(sizeof(object_Int));
    assert(obj);
    obj->value = 0;
    return obj;
}
object_Int * object_Int_assign(object_Int *object, object_Int *other) {
    assert(object);
    assert(other);
    object->value = other->value;
    return object;
}

object_Int * object_Int_new_from(int value) {
    object_Int * o = object_Int_new();
    o->value = value;
    return o;
}