#ifndef __AD_INITOOLS_H
#define __AD_INITOOLS_H

#include <stdio.h>
#include "libautodiag/string.h"
#include "buffer.h"
#include "file.h"

bool parse_ini_file(char *iniFile, bool func(char*,char*,char*), void *funcData);

#endif
