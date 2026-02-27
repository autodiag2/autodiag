#include "libautodiag/handle.h"


object_handle_t * object_handle_t_new() {
    object_handle_t * h = (object_handle_t*)malloc(sizeof(object_handle_t));
    #ifdef OS_POSIX
        h->posix_handle = -1;
    #endif
    #ifdef OS_WINDOWS
        h->win_handle = INVALID_HANDLE_VALUE;
    #endif
    return h;
}
void object_handle_t_free(object_handle_t * h) {
    if ( h != null ) {
        free(h);
    }
}
object_handle_t * object_handle_t_assign(object_handle_t * to, object_handle_t * from) {
    #ifdef OS_POSIX
        to->posix_handle = from->posix_handle;
    #endif
    #ifdef OS_WINDOWS
        to->win_handle = from->win_handle;
    #endif
    return to;
}