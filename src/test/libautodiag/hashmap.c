#include "libTest.h"
#include "libautodiag/lang/hashmap.h"

int object_hashmap_char_char_key_comparator(char *k1, char *k2) {
    return strcmp(k1, k2);
}

HASHMAP_H(char, char)
HASHMAP_SRC(char, char)

bool testHashMap() {
    object_hashmap_char_char * hm = object_hashmap_char_char_new();
    object_hashmap_char_char_set(hm, strdup("k"), strdup("v"));
    assert(object_hashmap_char_char_get(hm, "k") != null);
    assert(object_hashmap_char_char_get(hm, "y") == null);
    assert(strcmp(object_hashmap_char_char_get(hm, "k"), "v") == 0);
}