#pragma once

#include <stdio.h>

#include "list.h"
#include "scope.h"
#include "std.h"
#include "spec-form.h"

extern scheme_env SYSTEM_GLOBAL_ENVIRONMENT;
extern scheme_env USER_INITIAL_ENVIRONMENT;
extern int SCHEME_INTERPRETER_HALT;

void Scheme_DefineStartupEnv( void );
void Scheme_FreeStartupEnv( void );

void Scheme_Display(scheme_object * obj);
void Scheme_Newline( void );

scheme_object * Scheme_Eval(scheme_object * obj, scheme_env * env);
scheme_object * Scheme_Apply(scheme_object * func, scheme_object ** args, int arg_count, scheme_env * env);

char Scheme_BoolTest(scheme_object * obj);
