#ifndef __AD_HASHMAP_COMMON_H
#define __AD_HASHMAP_COMMON_H

#include "libautodiag/lang/hashmap.h"
#include "libautodiag/lang/Int.h"
#include "libautodiag/string.h"
#include "libautodiag/lang/Ptr.h"

AD_HASHMAP_H(Int, string)
AD_HASHMAP_H(Int, Int)
AD_HASHMAP_H(string, Ptr)
AD_HASHMAP_H(Ptr, string)
AD_HASHMAP_H(string, string)

#endif