#include "libTest.h"
#include "lib/stringlist.h"

bool testStringList() {
    {
        char *res = strdup("res"), *res2 = strdup("res2");
        StringList * list = StringList_new();
        StringList_append(list,res);
        StringList_append(list,res2);
        for(int i = 0; i < list->size; i++) {
            testOutput("element[%d]=%s\n", i, list->list[i]);
        }
        assert(list->size == 2);
        StringList_free(list);
    }
    return true;
}
