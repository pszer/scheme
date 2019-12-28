#pragma once

#include <stdlib.h>

#include "error.h"

enum {
	SCHEME_NULL,
	SCHEME_PAIR,
	SCHEME_NUMBER,
	SCHEME_STRING,
	SCHEME_SYMBOL,
};

typedef struct scheme_object {
	unsigned char type;
	
	void * payload;
} scheme_object;

// 1 for success, 0 for error
int  Scheme_AllocateObject(scheme_object ** object, int type);
void Scheme_FreeObject(scheme_object * object);

typedef struct scheme_pair {
	scheme_object * car,
	              * cdr;
} scheme_pair;

void Scheme_FreePair(scheme_pair * pair);

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

void Scheme_FreeNumber(scheme_number * number);

typedef struct scheme_string {
	char * string;
} scheme_string;
typedef struct scheme_string scheme_symbol;

void Scheme_FreeString(scheme_string * string);
void Scheme_FreeSymbol(scheme_symbol * symbol);

scheme_pair   * Scheme_GetPair(scheme_object * obj);
scheme_string * Scheme_GetString(scheme_object * obj);
scheme_number * Scheme_GetNumber(scheme_object * obj);
scheme_symbol * Scheme_GetSymbol(scheme_object * obj);

scheme_object * Scheme_CreateNull( void );
scheme_object * Scheme_CreatePair(scheme_object * car, scheme_object * cdr);
scheme_object * Scheme_CreateInteger(long long integer);
scheme_object * Scheme_CreateRational(long long numerator, long long denominator);
scheme_object * Scheme_CreateDouble(double value);
