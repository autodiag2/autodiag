#ifndef __HASHMAP_COMMON_H
#define __HASHMAP_COMMON_H

#include "libautodiag/lang/hashmap.h"
#include "libautodiag/lang/Int.h"
#include "libautodiag/string.h"
#include "libautodiag/lang/Ptr.h"

HASHMAP_H(Int, string)
HASHMAP_H(Int, Int)
HASHMAP_H(string, Ptr)

#endif