#include "libautodiag/jni/target_device.h"

#ifdef OS_ANDROID

static char * tmpDir = null;

char * jni_data_dir_get() {
    return tmpDir;
}

void jni_data_dir_set(char *path) {
    assert(path != null);
    tmpDir = strdup(path);
}

#endif