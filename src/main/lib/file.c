#include "lib/file.h"

bool file_read_line(BUFFER line,bool func(BUFFER line, void*data),void *data) {
    return func(line,data);
}

bool file_read_lines(char *fileName,bool func(BUFFER line, void*data), void *data) {
    FILE_READ_LINES_WITH(fileName,file_read_line,func,data);
}

bool mkdir_p(char *_path) {
    bool rv = true;
    char * path = strdup(_path);
    char * ptr = path;
    char * end_ptr = path + strlen(path);
    char *delim = PATH_FOLDER_DELIM;
    while(ptr < end_ptr) {
        char *s = strstr(ptr, delim);
        if ( s == null ) {
            break;
        }
        *s = 0;
        int currentFolderSz = strlen(ptr);
        ptr += currentFolderSz + 1;
        if ( 0 < currentFolderSz ) {
                
            #ifdef OS_POSIX
                if ( access(path,R_OK|X_OK) != 0 ) {
                    if ( mkdir(path,S_IRWXU) != 0 ) {
                        if ( chmod(path,S_IRWXU) != 0 ) {
                            rv = false;
                            break;
                        }
                    }
                }
            #elif defined OS_WINDOWS
                if ( access(path,R_OK|X_OK) != 0 ) {
                    mkdir(path);
                    if ( chmod(path,0x700) != 0 ) {
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
