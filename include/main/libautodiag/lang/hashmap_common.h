#ifndef __AD_HASHMAP_COMMON_H
#define __AD_HASHMAP_COMMON_H

#include "libautodiag/lang/hashmap.h"
#include "libautodiag/lang/Int.h"
#include "libautodiag/string.h"
#include "libautodiag/lang/Ptr.h"

HASHMAP_H(Int, string)
HASHMAP_H(Int, Int)
HASHMAP_H(string, Ptr)
HASHMAP_H(Ptr, string)
HASHMAP_H(string, string)

#endif