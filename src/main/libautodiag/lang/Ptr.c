#include "libautodiag/lang/Ptr.h"

AD_OBJECT_SRC(Ptr)

void ad_object_Ptr_free(ad_object_Ptr *object) {
    assert(object);
    if (object) {
        free(object);
    }
}
ad_object_Ptr *ad_object_Ptr_new() {
    ad_object_Ptr *obj = (ad_object_Ptr *)malloc(sizeof(ad_object_Ptr));
    assert(obj);
    obj->value = null;
    return obj;
}
ad_object_Ptr * ad_object_Ptr_assign(ad_object_Ptr *object, ad_object_Ptr *other) {
    assert(object);
    assert(other);
    object->value = other->value;
    return object;
}

ad_object_Ptr * ad_object_Ptr_new_from(void* value) {
    ad_object_Ptr * o = ad_object_Ptr_new();
    o->value = value;
    return o;
}