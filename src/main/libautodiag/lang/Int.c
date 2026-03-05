#include "libautodiag/lang/Int.h"

AD_OBJECT_SRC(Int)

void ad_object_Int_free(ad_object_Int *object) {
    assert(object);
    if (object) {
        free(object);
    }
}
ad_object_Int *ad_object_Int_new() {
    ad_object_Int *obj = (ad_object_Int *)malloc(sizeof(ad_object_Int));
    assert(obj);
    obj->value = 0;
    return obj;
}
ad_object_Int * ad_object_Int_assign(ad_object_Int *object, ad_object_Int *other) {
    assert(object);
    assert(other);
    object->value = other->value;
    return object;
}

ad_object_Int * ad_object_Int_new_from(int value) {
    ad_object_Int * o = ad_object_Int_new();
    o->value = value;
    return o;
}