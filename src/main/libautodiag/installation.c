#include "libautodiag/installation.h"

char *installation_folder(char *relative_path) {

    char *data_path = null;
    #if defined OS_WINDOWS
        int mode = R_OK;

        // System wide installation
        char path[MAX_PATH];
        char folder[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        strcpy(folder, path);
        char *last_backslash = strrchr(folder, '\\');
        if (last_backslash != NULL) {
            *last_backslash = '\0'; 
        }
        asprintf(&data_path, "%s\\%s", folder, relative_path);
        if ( access(data_path,mode) == 0 ) {
            return data_path;
        } else {
            free(data_path);
            data_path = null;
        }

        // Local user installation directory
        asprintf(&data_path,"%s\\" APP_NAME "\\%s",getenv("LOCALAPPDATA"),relative_path);
        if ( access(data_path,mode) == 0 ) {
            return data_path;
        } else {
            free(data_path);
            data_path = null;
        }

        // Windows application data directory
        asprintf(&data_path,"%s\\" APP_NAME "\\%s",getenv("APPDATA"),relative_path);
        if ( access(data_path,mode) == 0 ) {
            return data_path;
        } else {
            free(data_path);
            data_path = null;
        }

    #elif defined OS_UNIX
        int mode = R_OK;

        // XDG Base Directory Specification
        asprintf(&data_path, "%s/.local/share/" APP_NAME "/%s", getenv("HOME"), relative_path);
        if ( access(data_path,mode) == 0 ) {
            return data_path;
        } else {
            free(data_path);
            data_path = null;
        }
        // UNIX common location
        asprintf(&data_path,"/usr/share/" APP_NAME "/%s", relative_path);
        if ( access(data_path,mode) == 0 ) {
            return data_path;
        } else {
            free(data_path);
            data_path = null;
        }

        #if defined OS_APPLE
            asprintf(&data_path, "/Applications/" APP_NAME ".app/Contents/Resources/%s", relative_path);
            if ( access(data_path,mode) == 0 ) {
                return data_path;
            } else {
                free(data_path);
                data_path = null;
            }
        #endif
    #else
    #   warning Unsupported OS
    #endif
    return data_path;
}