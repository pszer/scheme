#pragma once

#include "object.h"
#include "error.h"

// list operations
// lists are defined using scheme_object pairs as nodes

int Scheme_IsPair(scheme_object * obj);
int Scheme_IsNull(scheme_object * obj);

scheme_object * Scheme_car(scheme_object * obj);
scheme_object * Scheme_cdr(scheme_object * obj);
