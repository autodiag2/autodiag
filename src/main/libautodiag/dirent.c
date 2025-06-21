#include "libautodiag/dirent.h"

#ifdef OS_POSIX

#elif defined OS_WINDOWS
int alphasort(const struct dirent **a, const struct dirent **b) {
    return strcmp((*a)->d_name, (*b)->d_name);
}

int scandir(const char *path, struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **)) {

    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return -1;

    scandir_list result = {0};
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        dirent *entry = malloc(sizeof(dirent));
        if (!entry) continue;

        strncpy(entry->d_name, fd.cFileName, MAX_PATH - 1);
        entry->d_name[MAX_PATH - 1] = '\0';
        entry->d_type = dirent_type_from_attr(fd.dwFileAttributes);

        if (!filter || filter(entry)) {
            if (result.count == result.capacity) {
                result.capacity = result.capacity ? result.capacity * 2 : 16;
                result.list = realloc(result.list, result.capacity * sizeof(dirent *));
            }
            result.list[result.count++] = entry;
        } else {
            free(entry);
        }

    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);

    if (compar) {
        qsort(result.list, result.count, sizeof(dirent *), (int(*)(const void *, const void *))compar);
    }

    *namelist = result.list;
    return result.count;
}
#endif