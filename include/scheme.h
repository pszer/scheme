#pragma once

#include <stdio.h>

#include "list.h"
#include "scope.h"
#include "std.h"
#include "spec-form.h"

#define SCHEME_STACK_SIZE 65536

extern scheme_object * SYSTEM_GLOBAL_ENVIRONMENT_OBJ;
extern scheme_object * USER_INITIAL_ENVIRONMENT_OBJ;
extern scheme_env * SYSTEM_GLOBAL_ENVIRONMENT;
extern scheme_env * USER_INITIAL_ENVIRONMENT;
extern int SCHEME_INTERPRETER_HALT;

void Scheme_DefineStartupEnv( void );
void Scheme_FreeStartupEnv( void );

void Scheme_Display(scheme_object * obj);
void Scheme_Newline( void );

typedef struct scheme_call {
	char is_cfunc_call;
	union {
		scheme_lambda * proc;

		struct {
			scheme_cfunc  * cfunc;
			scheme_object ** args;
			int arg_count;
		};
	};

	scheme_object * env;
} scheme_call;

extern scheme_call * call_stack;
extern scheme_call * call_stack_end;
void Scheme_InitCallStack(int stack_size);
void Scheme_FreeCallStack(void);
int  Scheme_PushCallStack(scheme_call call); // returns 1 if tail call push
scheme_object * Scheme_PopCallStack(void);
char Scheme_CanTailCallLambda(scheme_lambda * lambda);

void Scheme_DisplayCallStack(void);

//scheme_call Scheme_GetLambdaCall(scheme_lambda * lambda, scheme_object * obj, scheme_object * env);

scheme_object * Scheme_EvalSExpr(scheme_object * obj, scheme_object * env);
scheme_object * Scheme_Eval(scheme_object * obj, scheme_object * env);
scheme_object * Scheme_Apply(scheme_object * func, scheme_object ** args, int arg_count, scheme_object * env);
scheme_object * Scheme_ApplyCFunc(scheme_cfunc * cfunc, scheme_object ** args, int arg_count, scheme_object * env);
scheme_object * Scheme_ApplyCFuncSpecial(scheme_cfunc * cfunc, scheme_object ** args, int arg_count, scheme_object * env);
scheme_object * Scheme_ApplyLambda(scheme_lambda * lambda, scheme_object ** args, int arg_count, scheme_object * env);

scheme_object * Scheme_CallStack(void);

char Scheme_BoolTest(scheme_object * obj);
