#pragma once

#include "object.h"

#define SPEC_DEFINE_ARGC 2
#define SPEC_DEFINE_DOT 1
scheme_object * Scheme_Special_Define(scheme_object ** objs, scheme_object* env, size_t count);

#define SPEC_LAMBDA_ARGC 2
#define SPEC_LAMBDA_DOT 1
scheme_object * Scheme_Special_Lambda(scheme_object ** objs, scheme_object* env, size_t count);

#define SPEC_IF_ARGC 3
#define SPEC_IF_DOT 0
scheme_object * Scheme_Special_If(scheme_object ** objs, scheme_object* env, size_t count);

#define SPEC_QUOTE_ARGC 1
#define SPEC_QUOTE_DOT 0
scheme_object * Scheme_Special_Quote(scheme_object ** objs, scheme_object* env, size_t count);

#define SPEC_COND_ARGC 0
#define SPEC_COND_DOT 1
scheme_object * Scheme_Special_Cond(scheme_object ** objs, scheme_object* env, size_t count);
