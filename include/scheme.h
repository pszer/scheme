#pragma once

#include <stdio.h>

#include "list.h"
#include "scope.h"

extern scheme_env SYSTEM_GLOBAL_ENVIRONMENT;
extern scheme_env USER_INITIAL_ENVIRONMENT;

void Scheme_DefineStartupEnv( void );
void Scheme_FreeStartupEnv( void );

void Scheme_Display(scheme_object * obj);
void Scheme_Newline( void );

scheme_object * Scheme_Eval(scheme_object * obj, scheme_env * env);
scheme_object * Scheme_Apply(scheme_object * obj, scheme_object ** args, int arg_count);
