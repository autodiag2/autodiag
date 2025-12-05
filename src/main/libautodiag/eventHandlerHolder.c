#include "libautodiag/eventHandlerHolder.h"

EventHandlerHolder * ehh_new() {
    EventHandlerHolder * ehh = (EventHandlerHolder*)malloc(sizeof(EventHandlerHolder));
    ehh->list = null;
    ehh->size = 0;
    return ehh;
}
void ehh_free(EventHandlerHolder * ehh) {
    if ( ehh != null ) {
        if ( ehh->list != null ) {
            free(ehh->list);
            ehh->list = null;
        }
        ehh->size = 0;
        free(ehh);
    }
}
void ehh_register(final EventHandlerHolder* ehh, void *ptr) {
    ehh->list = (void**)realloc(ehh->list, sizeof(void*) * ++ehh->size);
    ehh->list[ehh->size-1] = ptr;
}
bool ehh_unregister(final EventHandlerHolder* ehh, void *ptr) {
    bool shift = false;
    for(int i = 0; i < ehh->size; i++) {
        if ( ptr == ehh->list[i] ) {
            shift = true;
        }
        if ( shift && i < (ehh->size-1) ) {
            ehh->list[i] = ehh->list[i+1];
        }
    }
    if ( shift ) {
        ehh->list = (void**)realloc(ehh->list, sizeof(void*) * --ehh->size);
    }
    return shift;
}