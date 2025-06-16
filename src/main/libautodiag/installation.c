#include "libautodiag/installation.h"

static char *_installation_folder_override = null;

void installation_folder_override(char *path) {
    if ( _installation_folder_override != null ) {
        free(_installation_folder_override);
    }
    if ( path == null ) {
        _installation_folder_override = null;
    } else {
        _installation_folder_override = strdup(path);
    }
}

char *installation_folder(char *relative_path) {

    char *path_in_installation_folder = null;
    if ( _installation_folder_override == null ) {
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
            asprintf(&path_in_installation_folder, "%s\\%s", folder, relative_path);
            if ( access(path_in_installation_folder,mode) == 0 ) {
                return path_in_installation_folder;
            } else {
                free(path_in_installation_folder);
                path_in_installation_folder = null;
            }

            // Local user installation directory
            asprintf(&path_in_installation_folder,"%s\\" APP_NAME "\\%s",getenv("LOCALAPPDATA"),relative_path);
            if ( access(path_in_installation_folder,mode) == 0 ) {
                return path_in_installation_folder;
            } else {
                free(path_in_installation_folder);
                path_in_installation_folder = null;
            }

            // Windows application data directory
            asprintf(&path_in_installation_folder,"%s\\" APP_NAME "\\%s",getenv("APPDATA"),relative_path);
            if ( access(path_in_installation_folder,mode) == 0 ) {
                return path_in_installation_folder;
            } else {
                free(path_in_installation_folder);
                path_in_installation_folder = null;
            }

        #elif defined OS_UNIX
            int mode = R_OK;

            // XDG Base Directory Specification
            asprintf(&path_in_installation_folder, "%s/.local/share/" APP_NAME "/%s", getenv("HOME"), relative_path);
            if ( access(path_in_installation_folder,mode) == 0 ) {
                return path_in_installation_folder;
            } else {
                free(path_in_installation_folder);
                path_in_installation_folder = null;
            }
            // UNIX common location
            asprintf(&path_in_installation_folder,"/usr/share/" APP_NAME "/%s", relative_path);
            if ( access(path_in_installation_folder,mode) == 0 ) {
                return path_in_installation_folder;
            } else {
                free(path_in_installation_folder);
                path_in_installation_folder = null;
            }

            #if defined OS_APPLE
                asprintf(&path_in_installation_folder, "/Applications/" APP_NAME ".app/Contents/Resources/%s", relative_path);
                if ( access(path_in_installation_folder,mode) == 0 ) {
                    return path_in_installation_folder;
                } else {
                    free(path_in_installation_folder);
                    path_in_installation_folder = null;
                }
            #endif
        #else
        #   warning Unsupported OS
        #endif
    } else {
        asprintf(&path_in_installation_folder, "%s\\%s", _installation_folder_override, relative_path);
    }
    return path_in_installation_folder;
}