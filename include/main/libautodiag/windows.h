#ifndef __CUSTOM_WINDOWS_H
#define __CUSTOM_WINDOWS_H

#include "libautodiag/compile_target.h"

#ifdef OS_WINDOWS
#   include <windows.h>
    bool isComPort(HANDLE file);
#endif

#endif