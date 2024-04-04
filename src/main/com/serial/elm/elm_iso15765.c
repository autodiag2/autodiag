#include "com/serial/elm/elm_iso15765.h"

int oneHex(char c) {
    char hex[2] = {0};
    hex[0] = c;
    return (int)strtol(hex,null,16);

}
