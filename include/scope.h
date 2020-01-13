#pragma once

#include <string.h>

#include "symbol.h"

#define SCOPE_SIZE 8

typedef struct scheme_object scheme_object;
typedef struct scheme_define {
	symbol * sym;
	scheme_object * object;
} scheme_define;

typedef struct scheme_env scheme_env;
struct scheme_env {
	int size, count;
	scheme_define * defs;

	// pointer to parent env
	scheme_object * parent;
};

#include "object.h"

scheme_define Scheme_CreateDefine(symbol * sym, scheme_object * obj);
scheme_define Scheme_CreateDefineString(char * string, scheme_object * obj);
void Scheme_FreeDefine(scheme_define * scheme_def);
void Scheme_OverwriteDefine(scheme_define * def, scheme_object * obj);

// lexigraphically compares two strings 'a' and 'b'
// returns
// -1 if a < b
//  0 if a = b
// +1 if a > b
//char LexigraphicCompare(const char * a, const char * b);

// definitions in a scheme_environment are sorted lexigraphically

scheme_env Scheme_CreateEnv(scheme_object * parent, int init_size);
void Scheme_FreeEnv(scheme_env * env);
void Scheme_ResizeEnv(scheme_env * env, int new_size);

void Scheme_DefineEnv(scheme_env * env, scheme_define def);
scheme_define * Scheme_GetEnv(scheme_env * env, symbol * sym);

void Scheme_DisplayEnv(scheme_env * env);
