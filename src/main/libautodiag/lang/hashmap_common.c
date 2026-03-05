#include "libautodiag/lang/hashmap_common.h"

int ad_object_hashmap_Int_string_key_comparator(ad_object_Int * k1, ad_object_Int *k2) {
    assert(k1 != null);
    assert(k2 != null);
    return k1->value - k2->value;
}
ad_object_hashmap_Int_string * ad_object_hashmap_Int_string_assign(ad_object_hashmap_Int_string*to, ad_object_hashmap_Int_string*from) {
    assert(to != null);
    assert(from != null);
    ad_object_hashmap_Int_string_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        ad_object_hashmap_Int_string_set(to, ad_object_Int_new_from(from->keys[i]->value), ad_object_string_new_from(from->values[i]->data));
    }
    return to;
}
HASHMAP_SRC(Int, string)

ad_object_hashmap_Int_Int * ad_object_hashmap_Int_Int_assign(ad_object_hashmap_Int_Int*to, ad_object_hashmap_Int_Int*from) {
    assert(to != null);
    assert(from != null);
    ad_object_hashmap_Int_Int_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        ad_object_hashmap_Int_Int_set(to, ad_object_Int_new_from(from->keys[i]->value), ad_object_Int_new_from(from->values[i]->value));
    }
    return to;
}
int ad_object_hashmap_Int_Int_key_comparator(ad_object_Int * k1, ad_object_Int *k2) {
    return ad_object_hashmap_Int_string_key_comparator(k1, k2);
}
HASHMAP_SRC(Int, Int)

int ad_object_hashmap_string_Ptr_key_comparator(ad_object_string * k1, ad_object_string * k2) {
    return strcmp(k1->data, k2->data);
}
ad_object_hashmap_string_Ptr * ad_object_hashmap_string_Ptr_assign(ad_object_hashmap_string_Ptr*to, ad_object_hashmap_string_Ptr*from) {
    assert(to != null);
    assert(from != null);
    ad_object_hashmap_string_Ptr_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        ad_object_hashmap_string_Ptr_set(to, ad_object_string_new_from(from->keys[i]->data), ad_object_Ptr_new_from(from->values[i]->value));
    }
    return to;
}
HASHMAP_SRC(string, Ptr)

ad_object_hashmap_Ptr_string * ad_object_hashmap_Ptr_string_assign(ad_object_hashmap_Ptr_string*to, ad_object_hashmap_Ptr_string*from) {
    assert(to != null);
    assert(from != null);
    ad_object_hashmap_Ptr_string_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        ad_object_hashmap_Ptr_string_set(to, ad_object_Ptr_new_from(from->keys[i]->value), ad_object_string_new_from(from->values[i]->data));
    }
    return to;
}
int ad_object_hashmap_Ptr_string_key_comparator(ad_object_Ptr * k1, ad_object_Ptr *k2) {
    return k1->value - k2->value;
}
HASHMAP_SRC(Ptr, string)

int ad_object_hashmap_string_string_key_comparator(ad_object_string *k1, ad_object_string *k2) {
    assert(k1 != null);
    assert(k2 != null);
    return strcmp(k1->data, k2->data);
}
ad_object_hashmap_string_string * ad_object_hashmap_string_string_assign(ad_object_hashmap_string_string*to, ad_object_hashmap_string_string*from) {
    assert(to != null);
    assert(from != null);
    ad_object_hashmap_string_string_clear(to);
    for(unsigned i = 0; i < from->size; i++) {
        ad_object_hashmap_string_string_set(to, ad_object_string_new_from(from->keys[i]->data), ad_object_string_new_from(from->values[i]->data));
    }
    return to;
}
HASHMAP_SRC(string, string)
