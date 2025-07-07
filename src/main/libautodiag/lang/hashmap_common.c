#include "libautodiag/lang/hashmap_common.h"

int object_hashmap_object_Int_object_string_key_comparator(object_Int * k1, object_Int *k2) {
    assert(k1 != null);
    assert(k2 != null);
    return k1->value - k2->value;
}
HASHMAP_SRC(object_Int, object_string)

int object_hashmap_object_Int_object_Int_key_comparator(object_Int * k1, object_Int *k2) {
    return object_hashmap_object_Int_object_string_key_comparator(k1, k2);
}
HASHMAP_SRC(object_Int, object_Int)