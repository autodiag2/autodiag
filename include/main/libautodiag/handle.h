#ifndef __HANDLE_H
#define __HANDLE_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/all.h"

OBJECT_H(handle_t,
    #ifdef OS_POSIX
        int posix_handle;
    #endif
    #ifdef OS_WINDOWS
        HANDLE win_handle;
    #endif
);

#endif