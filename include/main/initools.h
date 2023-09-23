#ifndef __INITOOLS_H
#define __INITOOLS_H

#include "globals.h"
#include <stdio.h>
#include <string.h>

bool parse_ini_file(char *iniFile, bool func(char*,char*,char*), void *funcData);

#endif
