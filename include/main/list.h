#ifndef __LIST_H
#define __LIST_H

#define LIST_NEW(list)\
    list->size = 0;\
    list->list = null;

#define LIST_FREE(list)\
    if (list->list != null ) {\
        for(int i = 0; i < list->size; i ++) {\
            free(list->list[i]);\
        }\
        free(list->list);\
        list->size = 0;\
    }

#define LIST_FREE_CONTIGUOUS(list)\
    if (list->list != null ) {\
        free(list->list);\
        list->size = 0;\
    }


#endif
