#include "libautodiag/windows.h"

#if defined OS_WINDOWS
    bool isComPort(HANDLE file) {
        DCB dcb;
        return GetCommState(file,&dcb);
    }
    int isSocketHandle(HANDLE h) {
        WSAPROTOCOL_INFO info;
        int len = sizeof(info);
        int r = getsockopt((SOCKET)h, SOL_SOCKET, SO_PROTOCOL_INFO,
                        (char *)&info, &len);
        return r == 0;
    }
#endif