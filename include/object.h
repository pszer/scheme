#pragma once

#include <stdlib.h>
#include <string.h>

#include "error.h"

enum {
	SCHEME_NULL,
	SCHEME_PAIR,
	SCHEME_NUMBER,
	SCHEME_STRING,
	SCHEME_SYMBOL,
	SCHEME_LAMBDA,
};

typedef struct scheme_object {
	unsigned char type;
	
	void * payload;
} scheme_object;

// 1 for success, 0 for error
int  Scheme_AllocateObject(scheme_object ** object, int type);
void Scheme_FreeObject(scheme_object * object);

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
typedef struct scheme_string scheme_symbol;

typedef struct scheme_lambda {
	int arg_count;
	scheme_symbol * arg_ids;
	scheme_object * body;
} scheme_lambda;

void Scheme_FreePair(scheme_pair * pair);
void Scheme_FreeNumber(scheme_number * number);
void Scheme_FreeString(scheme_string * string);
void Scheme_FreeSymbol(scheme_symbol * symbol);
void Scheme_FreeLambda(scheme_lambda * lambda);

scheme_pair   * Scheme_GetPair(scheme_object * obj);
scheme_string * Scheme_GetString(scheme_object * obj);
scheme_number * Scheme_GetNumber(scheme_object * obj);
scheme_symbol * Scheme_GetSymbol(scheme_object * obj);
scheme_lambda * Scheme_GetLambda(scheme_object * obj);

/* Object constructors
 * CreateSymbol and CreateString assume
 * string given as argument is allocated on the heap
 */
scheme_object * Scheme_CreateNull( void );
scheme_object * Scheme_CreateSymbol(char * symbol);
scheme_object * Scheme_CreateString(char * string);
scheme_object * Scheme_CreatePair(scheme_object * car, scheme_object * cdr);
scheme_object * Scheme_CreateInteger(long long integer);
scheme_object * Scheme_CreateRational(long long numerator, long long denominator);
scheme_object * Scheme_CreateDouble(double value);
scheme_object * Scheme_CreateLambda(int acount, scheme_symbol * args, scheme_object * body);

// Copies string onto heap
char * Scheme_CopyStringHeap(const char * string);

scheme_object * Scheme_CreateSymbolLiteral(const char * symbol);
scheme_object * Scheme_CreateStringLiteral(const char * string);
