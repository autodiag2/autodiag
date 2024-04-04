#include "compile_target.h"
#ifdef OS_POSIX

#ifndef __CUSTOM_POLL_H
#define __CUSTOM_POLL_H

#include <poll.h>
typedef struct pollfd POLLFD;

#endif
#endif
