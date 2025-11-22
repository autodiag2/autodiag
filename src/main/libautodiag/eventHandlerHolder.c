#include "libautodiag/eventHandlerHolder.h"

EventHandlerHolder * ehh_new() {
    EventHandlerHolder * ehh = (EventHandlerHolder*)malloc(sizeof(EventHandlerHolder));
    ehh->list = null;
    ehh->size = 0;
}
void ehh_free(EventHandlerHolder * ehh) {
    if ( ehh != null ) {
        for(int i = 0; i < ehh->size; i++) {
            free(ehh->list[i]);
        }
        ehh->list = null;
        ehh->size = 0;
        free(ehh);
    }
}
void ehh_register(final EventHandlerHolder* ehh, void *ptr) {
    ehh->list = (void**)realloc(ehh->list, sizeof(void*) * ++ehh->size);
    ehh->list[ehh->size-1] = ptr;
}