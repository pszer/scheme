#include "list.h"

int Scheme_IsPair(scheme_object * obj) {
	return obj && obj->type == SCHEME_PAIR;
}

int Scheme_IsNull(scheme_object * obj) {
	return obj && obj->type == SCHEME_NULL;
}

int Scheme_ListLength(scheme_object * obj) {
	scheme_object * node = obj;

	if (!obj) {
		Scheme_SetError("ListLength() on NULL");
		return 0;
	}

	int count = 1;
	while (1) {
		if (node->type != SCHEME_PAIR) {
			Scheme_SetError("ListLength() on non-list");
			return 0;
		}

		scheme_pair * p = Scheme_GetPair(node);
		if (p->cdr == NULL || p->cdr->type == SCHEME_NULL)
			return count;
		node = p->cdr;
		++count;
	}
}

#define TEST_NULL(obj, fail) if(obj==NULL){Scheme_SetError("operation on null object");return fail;}
scheme_object * Scheme_Car(scheme_object * obj) {
	TEST_NULL(obj, NULL);

	scheme_pair * pair = Scheme_GetPair(obj);
	if (!pair) return NULL;

	return pair->car;
}

scheme_object * Scheme_Cdr(scheme_object * obj) {
	TEST_NULL(obj, NULL);

	scheme_pair * pair = Scheme_GetPair(obj);
	if (!pair) return NULL;

	return pair->cdr;
}

scheme_object * Scheme_Cons(scheme_object * a, scheme_object * b) {
	if (a && a->type == SCHEME_NULL) a = NULL;
	if (b && b->type == SCHEME_NULL) b = NULL;
	return Scheme_CreatePair(a , b);
}

scheme_object * Scheme_ListFromArray(scheme_object ** array, int count) {
	if (count == 0) return Scheme_CreateNull();
	if (count < 0) {
		Scheme_SetError("Scheme_ListFromArray() : bad count");
		return NULL;
	}

	scheme_object * base = Scheme_CreatePair(array[0], NULL),
	              * prev = base;
	int i;
	for (i = 1; i < count; ++i) {
		scheme_object * new_pair = Scheme_CreatePair(array[i], NULL);
		Scheme_GetPair(prev)->cdr = new_pair;
		prev = new_pair;
	}

	return base;
}

scheme_object * Scheme_AppendList(scheme_object * list, scheme_object * obj) {
	if (!list || list->type != SCHEME_PAIR) {
		Scheme_SetError("attempt to append to non-pair object");
		return NULL;
	}

	scheme_object * iter = list;
	while (1) {
		scheme_pair * pair = Scheme_GetPair(iter);
		if (!pair->cdr) {
			pair->cdr = Scheme_CreatePair(obj, NULL);
			return list;
		} else {
			iter = pair->cdr;
		}
	}

	return NULL;
}
