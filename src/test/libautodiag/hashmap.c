#include "libTest.h"
#include "libautodiag/lang/hashmap.h"

int hashmap_char_char_key_comparator(char *k1, char *k2) {
    return strcmp(k1, k2);
}

HASHMAP_H(char, char)
HASHMAP_SRC(char, char)

bool testHashMap() {
    hashmap_char_char * hm = hashmap_char_char_new();
    hashmap_char_char_set(hm, strdup("k"), strdup("v"));
    assert(hashmap_char_char_get(hm, "k") != null);
    assert(hashmap_char_char_get(hm, "y") == null);
    assert(strcmp(hashmap_char_char_get(hm, "k"), "v") == 0);
}