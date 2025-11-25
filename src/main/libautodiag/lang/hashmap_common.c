#include "libautodiag/lang/hashmap_common.h"

int object_hashmap_Int_string_key_comparator(object_Int * k1, object_Int *k2) {
    assert(k1 != null);
    assert(k2 != null);
    return k1->value - k2->value;
}
HASHMAP_SRC(Int, string)

int object_hashmap_Int_Int_key_comparator(object_Int * k1, object_Int *k2) {
    return object_hashmap_Int_string_key_comparator(k1, k2);
}
HASHMAP_SRC(Int, Int)
int object_hashmap_string_Ptr_key_comparator(object_string * k1, object_string * k2) {
    return strcmp(k1->data, k2->data);
}
HASHMAP_SRC(string, Ptr)