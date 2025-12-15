#include "libautodiag/jni/tmp_file.h"

#ifdef OS_ANDROID

static char * tmpDir = null;

char * jni_tmp_dir_get() {
    return tmpDir;
}

void jni_tmp_dir_set(char *path) {
    assert(path != null);
    tmpDir = strdup(path);
}

#endif