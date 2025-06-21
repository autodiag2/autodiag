#ifndef __CUSTOM_DIRENT_H
#define __CUSTOM_DIRENT_H

#include "compile_target.h"

#ifdef OS_POSIX
#   include <dirent.h>
#elif defined OS_WINDOWS

#   include <windows.h>
#   include <stdlib.h>
#   include <string.h>
#   include <stdio.h>

#   define DT_UNKNOWN  0
#   define DT_REG      8
#   define DT_DIR      4
#   define DT_CHR      2
#   define DT_FIFO     1
#   define DT_BLK      6

    typedef struct dirent {
        char d_name[MAX_PATH];
        unsigned char d_type;
    } dirent;

    typedef struct {
        dirent **list;
        size_t count;
        size_t capacity;
    } scandir_list;

    static int dirent_type_from_attr(DWORD attr) {
        if (attr & FILE_ATTRIBUTE_DIRECTORY) return DT_DIR;
        return DT_REG;
    }

    int alphasort(const struct dirent **a, const struct dirent **b);
    int scandir(const char *path, struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **));

#else
#   warning Unsupported OS
#endif

typedef struct dirent DIRENT;

#endif