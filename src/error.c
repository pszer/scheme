#include "error.h"
#include "scheme.h"

char * error_str = NULL ;
char * Scheme_GetError(void) {
	//Scheme_DisplayCallStack();

	char * temp = error_str;
	error_str = NULL;
	return temp;
}

void Scheme_SetError(char * str) {
	error_str = str;
}
