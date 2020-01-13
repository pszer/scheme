#pragma once

#include <stdlib.h>
#include <string.h>

#include "symbol.h"
#include "error.h"

typedef struct scheme_env_obj scheme_env_obj;
typedef struct scheme_env scheme_env;

enum {
	SCHEME_NULL,
	SCHEME_PAIR,
	SCHEME_NUMBER,
	SCHEME_BOOLEAN,
	SCHEME_STRING,
	SCHEME_SYMBOL,
	SCHEME_LAMBDA,
	SCHEME_ENV,
	SCHEME_CFUNC
};

typedef struct scheme_object {
	unsigned char type;	
	void * payload;
	int ref_count;
} scheme_object;

// 1 for success, 0 for error
int  Scheme_AllocateObject(scheme_object ** object, int type);
void Scheme_FreeObject(scheme_object * object);

// required for changing ref_count
void Scheme_ReferenceObject(scheme_object ** pointer, scheme_object * object);
void Scheme_DereferenceObject(scheme_object ** pointer);

#include "scope.h"

#define SCHEME_FREED_MEMORY_START_SIZE 8
// memory of what objects have been freed
// when freeing a list to avoid a cycle
struct scheme_freed_memory {
	int size, pos;
	scheme_object ** objects;
};
struct scheme_freed_memory * Scheme_InitFreedMemory();
int  Scheme_CheckIfFreed(struct scheme_freed_memory * mem, scheme_object * address);
void Scheme_AddFreed(struct scheme_freed_memory * mem, scheme_object * address);
void Scheme_FreeFreedMemory(struct scheme_freed_memory * mem);

typedef struct scheme_pair {
	scheme_object * car,
	              * cdr;
} scheme_pair;

enum {
	NUMBER_INTEGER,
	NUMBER_RATIONAL,
	NUMBER_DOUBLE
};

typedef struct scheme_boolean {
	char val;
} scheme_boolean;

typedef struct scheme_number {
	unsigned char type;

	union {
		long long integer_val;
		double double_val;
		struct { long long numerator, denominator; };
	};
} scheme_number;

typedef struct scheme_string {
	union {
		char * string, * symbol;
	};
} scheme_string;

typedef struct scheme_symbol {
	symbol * sym;
} scheme_symbol;

typedef struct scheme_lambda {
	int arg_count;
	char dot_args;
	symbol ** arg_ids;

	int body_count;
	scheme_object ** body;

	scheme_object * closure;
} scheme_lambda;

// scheme_object * func(scheme_object ** objects, size_t object_count);
typedef struct scheme_cfunc {
	scheme_object* (*func)(scheme_object **, scheme_object*, size_t);
	
	int arg_count;
	char dot_args;
	char special_form;
} scheme_cfunc;

void Scheme_FreePair(scheme_pair * pair);
void Scheme_FreeNumber(scheme_number * number);
void Scheme_FreeBoolean(scheme_boolean * boolean);
void Scheme_FreeString(scheme_string * string);
void Scheme_FreeSymbol(scheme_symbol * symbol);
void Scheme_FreeLambda(scheme_lambda * lambda);
void Scheme_FreeEnvObj(scheme_env * env);
void Scheme_FreeCFunc(scheme_cfunc * cfunc);

scheme_pair    * Scheme_GetPair  (scheme_object * obj);
scheme_number  * Scheme_GetNumber(scheme_object * obj);
scheme_boolean * Scheme_GetBoolean(scheme_object * obj);
scheme_string  * Scheme_GetString(scheme_object * obj);
scheme_symbol  * Scheme_GetSymbol(scheme_object * obj);
scheme_lambda  * Scheme_GetLambda(scheme_object * obj);
scheme_env     * Scheme_GetEnvObj(scheme_object * obj);
scheme_cfunc   * Scheme_GetCFunc (scheme_object * obj);

/* Object constructors
 * CreateSymbol and CreateString assume
 * string given as argument is allocated on the heap
 */
scheme_object * Scheme_CreateNull( void );
scheme_object * Scheme_CreateSymbol(char * symbol);
scheme_object * Scheme_CreateSymbolFromSymbol(symbol * symbol);
scheme_object * Scheme_CreatePair(scheme_object * car, scheme_object * cdr);
scheme_object * Scheme_CreateBoolean(char val);
scheme_object * Scheme_CreateInteger(long long integer);
scheme_object * Scheme_CreateRational(long long numerator, long long denominator);
scheme_object * Scheme_CreateDouble(double value);
scheme_object * Scheme_CreateString(char * string);
scheme_object * Scheme_CreateEnvObj(scheme_object * parent, int init_size);
scheme_object * Scheme_CreateLambda(int argc, char dot_args, symbol ** args, int body_count,
	scheme_object ** body, scheme_object * closure);
scheme_object * Scheme_CreateCFunc(int argc, char dot_args, char special_form,
	scheme_object* (*func)(scheme_object**,scheme_object*,size_t));

scheme_object * Scheme_CreateSymbolLiteral(const char * symbol);
scheme_object * Scheme_CreateStringLiteral(const char * string);
