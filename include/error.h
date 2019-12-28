#pragma once

#include <stddef.h>

extern char * error_str;
char * Scheme_GetError(void);
void   Scheme_SetError(char * str);
