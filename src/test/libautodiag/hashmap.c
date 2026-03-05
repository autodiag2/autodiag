#include "libTest.h"
#include "libautodiag/lang/hashmap.h"

bool testHashMap() {
    ad_object_hashmap_string_string * hm = ad_object_hashmap_string_string_new();
    ad_object_hashmap_string_string_set(hm, ad_object_string_new_from("k"), ad_object_string_new_from("v"));
    assert(ad_object_hashmap_string_string_get(hm, ad_object_string_new_from("k")) != null);
    assert(ad_object_hashmap_string_string_get(hm, ad_object_string_new_from("y")) == null);
    assert(strcmp(ad_object_hashmap_string_string_get(hm, ad_object_string_new_from("k"))->data, "v") == 0);
    return true;
}