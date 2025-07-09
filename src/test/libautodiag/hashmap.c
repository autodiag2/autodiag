#include "libTest.h"
#include "libautodiag/lang/hashmap.h"

int object_hashmap_object_string_object_string_key_comparator(object_string *k1, object_string *k2) {
    assert(k1 != null);
    assert(k2 != null);
    return strcmp(k1->data, k2->data);
}

HASHMAP_H(object_string, object_string)
HASHMAP_SRC(object_string, object_string)

bool testHashMap() {
    object_hashmap_object_string_object_string * hm = object_hashmap_object_string_object_string_new();
    object_hashmap_object_string_object_string_set(hm, object_string_new_from("k"), object_string_new_from("v"));
    assert(object_hashmap_object_string_object_string_get(hm, object_string_new_from("k")) != null);
    assert(object_hashmap_object_string_object_string_get(hm, object_string_new_from("y")) == null);
    assert(strcmp(object_hashmap_object_string_object_string_get(hm, object_string_new_from("k"))->data, "v") == 0);
}