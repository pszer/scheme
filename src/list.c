#include "list.h"

int Scheme_IsPair(scheme_object * obj) {
	return obj && obj->type == SCHEME_PAIR;
}

int Scheme_IsNull(scheme_object * obj) {
	return obj && obj->type == SCHEME_NULL;
}

#define TEST_NULL(obj, fail) if(obj==NULL){Scheme_SetError("operation on null object");return fail;}
scheme_object * Scheme_car(scheme_object * obj) {
	TEST_NULL(obj, NULL);

	scheme_pair * pair = Scheme_GetPair(obj);
	if (!pair) return NULL;

	if (pair->car == NULL)
		return NULL;
	else    return pair->car;
}

scheme_object * Scheme_cdr(scheme_object * obj) {
	TEST_NULL(obj, NULL);

	scheme_pair * pair = Scheme_GetPair(obj);
	if (!pair) return NULL;

	if (pair->cdr == NULL)
		return NULL;
	else    return pair->cdr;
} 
