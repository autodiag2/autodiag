#include "libautodiag/windows.h"

#if defined OS_WINDOWS
    bool isComPort(HANDLE file) {
        DCB dcb;
        return GetCommState(file,&dcb);
    }
#endif