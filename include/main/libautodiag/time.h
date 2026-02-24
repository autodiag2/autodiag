#ifndef __AD_TIME_H
#define __AD_TIME_H

#include <stdint.h>

#include "libautodiag/compile_target.h"

#ifdef OS_WINDOWS
#   include <windows.h>
#elif defined OS_POSIX
#   include <time.h>
#endif


uint64_t time_ms();

#endif