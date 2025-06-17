#include "libautodiag/installation.h"

static char *_installation_folder_resolve_override = null;

void installation_folder_resolve_override(char *path) {
    if ( _installation_folder_resolve_override != null ) {
        free(_installation_folder_resolve_override);
    }
    if ( path == null ) {
        _installation_folder_resolve_override = null;
    } else {
        _installation_folder_resolve_override = strdup(path);
    }
}

char *installation_folder_resolve(char *relative_path) {

    char *path_in_installation_folder_resolve = null;
    if ( _installation_folder_resolve_override == null ) {
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
            asprintf(&path_in_installation_folder_resolve, "%s\\%s", folder, relative_path);
            if ( access(path_in_installation_folder_resolve,mode) == 0 ) {
                return path_in_installation_folder_resolve;
            } else {
                free(path_in_installation_folder_resolve);
                path_in_installation_folder_resolve = null;
            }

            // Local user installation directory
            asprintf(&path_in_installation_folder_resolve,"%s\\" APP_NAME "\\%s",getenv("LOCALAPPDATA"),relative_path);
            if ( access(path_in_installation_folder_resolve,mode) == 0 ) {
                return path_in_installation_folder_resolve;
            } else {
                free(path_in_installation_folder_resolve);
                path_in_installation_folder_resolve = null;
            }

            // Windows application data directory
            asprintf(&path_in_installation_folder_resolve,"%s\\" APP_NAME "\\%s",getenv("APPDATA"),relative_path);
            if ( access(path_in_installation_folder_resolve,mode) == 0 ) {
                return path_in_installation_folder_resolve;
            } else {
                free(path_in_installation_folder_resolve);
                path_in_installation_folder_resolve = null;
            }

        #elif defined OS_UNIX
            int mode = R_OK;

            // XDG Base Directory Specification
            asprintf(&path_in_installation_folder_resolve, "%s/.local/share/" APP_NAME "/%s", getenv("HOME"), relative_path);
            if ( access(path_in_installation_folder_resolve,mode) == 0 ) {
                return path_in_installation_folder_resolve;
            } else {
                free(path_in_installation_folder_resolve);
                path_in_installation_folder_resolve = null;
            }
            // UNIX common location
            asprintf(&path_in_installation_folder_resolve,"/usr/share/" APP_NAME "/%s", relative_path);
            if ( access(path_in_installation_folder_resolve,mode) == 0 ) {
                return path_in_installation_folder_resolve;
            } else {
                free(path_in_installation_folder_resolve);
                path_in_installation_folder_resolve = null;
            }

            #if defined OS_APPLE
                asprintf(&path_in_installation_folder_resolve, "/Applications/" APP_NAME ".app/Contents/Resources/%s", relative_path);
                if ( access(path_in_installation_folder_resolve,mode) == 0 ) {
                    return path_in_installation_folder_resolve;
                } else {
                    free(path_in_installation_folder_resolve);
                    path_in_installation_folder_resolve = null;
                }
            #endif
        #else
        #   warning Unsupported OS
        #endif
    } else {
        asprintf(&path_in_installation_folder_resolve, "%s" PATH_FOLDER_DELIM "%s", _installation_folder_resolve_override, relative_path);
    }
    return path_in_installation_folder_resolve;
}