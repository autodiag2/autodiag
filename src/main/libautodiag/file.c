#include "libautodiag/file.h"

bool file_read_line(Buffer * line,bool func(Buffer * line, void*data),void *data) {
    return func(line,data);
}

bool file_read_lines(char *fileName,bool func(Buffer * line, void*data), void *data) {
    FILE_READ_LINES_WITH(fileName,file_read_line,func,data);
}

bool mkdir_p(char *_path) {
    bool rv = true;
    char * path = strdup(_path);
    char * ptr = path;
    char * end_ptr = path + strlen(path);
    char *delim = PATH_FOLDER_DELIM;
    #if defined OS_WINDOWS
        // skip the first part of the path
        ptr = strstr(ptr, delim) + strlen(delim);
    #endif
    while(ptr < end_ptr) {
        char *s = strstr(ptr, delim);
        if ( s == null ) {
            break;
        }
        *s = 0;
        int currentFolderSz = strlen(ptr);
        ptr += currentFolderSz + 1;
        if ( 0 < currentFolderSz ) {
                
            #if defined OS_POSIX
                if ( access(path,R_OK|X_OK) != 0 ) {
                    if ( mkdir(path,S_IRWXU) != 0 ) {
                        if ( chmod(path,S_IRWXU) != 0 ) {
                            rv = false;
                            break;
                        }
                    }
                }
            #elif defined OS_WINDOWS
                if ( access(path, R_OK) != 0 ) {
                    if ( mkdir(path) != 0 ) {
                        rv = false;
                        break;
                    }
                }
            #else
            #   warning Unsupported OS        
            #endif

        }
        memcpy(s,delim,strlen(delim));
    }
    free(path);
    return rv;
}

char *file_get_next_free(const char *filepath) {
    struct stat st;
    size_t len = snprintf(null, 0, "%s", filepath);
    char *path = malloc(len + 1);
    if (!path)
        return null;

    snprintf(path, len + 1, "%s", filepath);

    if (stat(path, &st) != 0)
        return path;

    free(path);

    for (int i = 1; i < 1000; i++) {
        len = snprintf(null, 0, "%s.%d", filepath, i);
        path = malloc(len + 1);
        if (!path)
            return null;

        snprintf(path, len + 1, "%s.%d", filepath, i);

        if (stat(path, &st) != 0)
            return path;

        free(path);
    }

    return null;
}