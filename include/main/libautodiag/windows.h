#ifndef __CUSTOM_WINDOWS_H
#define __CUSTOM_WINDOWS_H

#include "libautodiag/lang/lang.h"
#include "libautodiag/compile_target.h"

#ifdef OS_WINDOWS
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   include <windows.h>
    bool isComPort(HANDLE file);
    int isSocketHandle(HANDLE h);
#endif

#endif