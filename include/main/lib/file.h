#ifndef __CUSTOM_FILE_H
#define __CUSTOM_FILE_H

#include "compile_target.h"
#define _GNU_SOURCE
#include <stdio.h>
#   include <sys/stat.h>
#if defined OS_WINDOWS
#   include <unistd.h>
#   define FILE_EOL "\r\n"
#   define PATH_FOLDER_DELIM "\\"
#elif defined OS_UNIX || defined OS_POSIX
#   define FILE_EOL "\n"
#   define PATH_FOLDER_DELIM "/"
#   include <sys/types.h>
#   include <fcntl.h>
#else
#   warning Unsupported OS 
#endif

#include "string.h"
#include "buffer.h"

/**
 * Use of fread to get around limitation of the getline function (cannot choose end of line)
 */
#define FILE_READ_LINES_WITH(fileName,handler,handlerFunc, handlerData) \
    FILE * file = fopen(fileName, "r"); \
    if ( file == null ) { \
        perror("fopen"); \
        return false; \
    } else { \
        char eol[] = FILE_EOL;  \
                                \
                                \
        int bulk_sz = 500; \
        BUFFER buffer = buffer_new(); \
                                        \
        do { \
            buffer_ensure_capacity(buffer,bulk_sz); \
            int bytes_readed = fread(buffer->buffer+buffer->size,1,bulk_sz,file); \
            if ( 0 < bytes_readed ) { \
                buffer->size += bytes_readed; \
                buffer_ensure_termination(buffer); \
                \
                for(char * eol_ptr = strstr(buffer->buffer,eol);eol_ptr != null;eol_ptr = strstr(buffer->buffer,eol)) { \
                    *eol_ptr = 0;                     \
                    if ( ! handler(buffer,handlerFunc,handlerData) ) { \
                        return false; \
                    } \
                    int line_length = ((unsigned char*)eol_ptr - buffer->buffer) + strlen(eol); \
                    buffer_left_shift(buffer,line_length); \
                } \
                if ( 0 < buffer->size && buffer->buffer[buffer->size-1] == 0) { \
                    buffer->size --; \
                } \
            } \
        } while(!feof(file)); \
        \
        if ( 0 < buffer->size ) { \
            buffer_ensure_termination(buffer);        \
            if ( ! handler(buffer,handlerFunc,handlerData) ) { \
                return false; \
            } \
        } \
        \
        buffer_free(buffer); \
                            \
        fclose(file); \
        return true; \
    }
    
#endif

bool file_read_lines(char *fileName,bool func(BUFFER line, void*data), void *data);

/**
 * For a given path with optionally a filename at the end,
 * make sure that all directory along the path are at least readable,
 * and executable. New created folder got user RWX
 * path must be absolute under windows.
 */
bool mkdir_p(char *_path);
