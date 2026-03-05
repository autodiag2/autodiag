#include "libTest.h"
#include "libautodiag/lang/hashmap.h"

bool testHashMap() {
    object_hashmap_string_string * hm = object_hashmap_string_string_new();
    object_hashmap_string_string_set(hm, object_string_new_from("k"), object_string_new_from("v"));
    assert(object_hashmap_string_string_get(hm, object_string_new_from("k")) != null);
    assert(object_hashmap_string_string_get(hm, object_string_new_from("y")) == null);
    assert(strcmp(object_hashmap_string_string_get(hm, object_string_new_from("k"))->data, "v") == 0);
    return true;
}