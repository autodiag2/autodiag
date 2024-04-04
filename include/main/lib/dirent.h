#ifndef __CUSTOM_DIRENT_H
#define __CUSTOM_DIRENT_H

#include "compile_target.h"

#include <dirent.h>

#ifdef OS_WINDOWS
    #include <windows.h>
    #define _GNU_SOURCE
    #include <stdio.h>

    typedef enum {
        DT_DIR, DT_REG, DT_BLK, DT_FIFO, DT_CHR
    } DT_TYPE;

    typedef struct {
        struct dirent;
        DT_TYPE d_type;
    } DIRENT;

    int scandir(char *path, DIRENT ***namelist);
#else
typedef struct dirent DIRENT;
#endif

#endif
