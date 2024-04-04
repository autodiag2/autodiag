#include "lib/dirent.h"

#ifdef OS_WINDOWS
int scandir(char *path, DIRENT ***namelist) {

    *namelist = NULL;
    WIN32_FIND_DATAA data;      
    char * pathe;
    asprintf(&pathe, "%s\\*", path);
    HANDLE top = FindFirstFileA(pathe, &data);
    free(pathe);
    if ( top == INVALID_HANDLE_VALUE ) {
        return -1;
    } else {    
        int namelist_n = 0;
        do {
            *namelist = (DIRENT**)realloc(*namelist,sizeof(DIRENT*) * (++namelist_n));
            (*namelist)[namelist_n-1] = (DIRENT*)malloc(sizeof(DIRENT));
            if ( (FILE_ATTRIBUTE_DIRECTORY & data.dwFileAttributes) == 0 ) {
                (*namelist)[namelist_n-1]->d_type = DT_REG;
            } else {
                (*namelist)[namelist_n-1]->d_type = DT_DIR;
            }
            strncpy((*namelist)[namelist_n-1]->d_name, data.cFileName, FILENAME_MAX-1);
        } while ( FindNextFileA(top,&data) != 0 );
        return namelist_n;
    }

}
#endif
