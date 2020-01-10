#pragma once

#include "object.h"
#include "error.h"

// list operations
// lists are defined using scheme_object pairs as nodes

int Scheme_IsPair(scheme_object * obj);
int Scheme_IsNull(scheme_object * obj);

int Scheme_ListLength(scheme_object * obj);

scheme_object * Scheme_Car(scheme_object * obj);
scheme_object * Scheme_Cdr(scheme_object * obj);

scheme_object * Scheme_Cons(scheme_object * a, scheme_object * b);
scheme_object * Scheme_ListFromArray(scheme_object ** array, int count);
scheme_object * Scheme_AppendList(scheme_object * list, scheme_object * obj);
