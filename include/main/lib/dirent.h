#ifndef __CUSTOM_DIRENT_H
#define __CUSTOM_DIRENT_H

#include "compile_target.h"

#ifdef OS_POSIX
#   include <dirent.h>
typedef struct dirent DIRENT;
#else
#   warning Unsupported OS
#endif
