#include "error.h"

char * error_str = NULL;
char * Scheme_GetError(void) {
	char * temp = error_str;
	error_str = NULL;
	return temp;
}

void Scheme_SetError(char * str) {
	error_str = str;
}
