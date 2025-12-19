#ifndef __CUSTOM_WINDOWS_H
#define __CUSTOM_WINDOWS_H

#include "libautodiag/lang/lang.h"
#include "libautodiag/compile_target.h"

#ifdef OS_WINDOWS
#   ifndef OS_POSIX
#       include <winsock2.h>
#       include <ws2tcpip.h>
#   endif
#   include <windows.h>
    bool isComPort(HANDLE file);
#   ifndef OS_POSIX
        int isSocketHandle(HANDLE h);
#   endif
#endif

#endif