#include "libautodiag/lang/hashmap_common.h"

int object_hashmap_Int_string_key_comparator(object_Int * k1, object_Int *k2) {
    assert(k1 != null);
    assert(k2 != null);
    return k1->value - k2->value;
}
object_hashmap_Int_string * object_hashmap_Int_string_assign(object_hashmap_Int_string*to, object_hashmap_Int_string*from) {
    assert(to != null);
    assert(from != null);
    object_hashmap_Int_string_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        object_hashmap_Int_string_set(to, object_Int_new_from(from->keys[i]->value), object_string_new_from(from->values[i]->data));
    }
    return to;
}
HASHMAP_SRC(Int, string)

object_hashmap_Int_Int * object_hashmap_Int_Int_assign(object_hashmap_Int_Int*to, object_hashmap_Int_Int*from) {
    assert(to != null);
    assert(from != null);
    object_hashmap_Int_Int_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        object_hashmap_Int_Int_set(to, object_Int_new_from(from->keys[i]->value), object_Int_new_from(from->values[i]->value));
    }
    return to;
}
int object_hashmap_Int_Int_key_comparator(object_Int * k1, object_Int *k2) {
    return object_hashmap_Int_string_key_comparator(k1, k2);
}
HASHMAP_SRC(Int, Int)
int object_hashmap_string_Ptr_key_comparator(object_string * k1, object_string * k2) {
    return strcmp(k1->data, k2->data);
}
object_hashmap_string_Ptr * object_hashmap_string_Ptr_assign(object_hashmap_string_Ptr*to, object_hashmap_string_Ptr*from) {
    assert(to != null);
    assert(from != null);
    object_hashmap_string_Ptr_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        object_hashmap_string_Ptr_set(to, object_string_new_from(from->keys[i]->data), object_Ptr_new_from(from->values[i]->value));
    }
    return to;
}
HASHMAP_SRC(string, Ptr)

object_hashmap_Ptr_string * object_hashmap_Ptr_string_assign(object_hashmap_Ptr_string*to, object_hashmap_Ptr_string*from) {
    assert(to != null);
    assert(from != null);
    object_hashmap_Ptr_string_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        object_hashmap_Ptr_string_set(to, object_Ptr_new_from(from->keys[i]->value), object_string_new_from(from->values[i]->data));
    }
    return to;
}
int object_hashmap_Ptr_string_key_comparator(object_Ptr * k1, object_Ptr *k2) {
    return k1->value - k2->value;
}
HASHMAP_SRC(Ptr, string)