#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "object.h"

/*
 * lexical analyser
 * takes in a scheme string/file and turns it into tokens
 * e.g.
 * (expt (+ x 2) 5)
 * turns into
 * '(' TOKEN_SYMBOL '(' TOKEN_SYMBOL TOKEN_SYMBOL TOKEN_NUMBER
 *    ')' TOKEN_NUMBER ')' TOKEN_EOF
 */

enum {
	TOKEN_EOF=-1 ,
	TOKEN_SYMBOL=-2 ,
	TOKEN_NUMBER=-3 ,
	TOKEN_BOOLEAN=-4 ,
	TOKEN_STRING=-5
};

enum {
	CHAR_EOF,
	CHAR_SYMBOL,
	CHAR_STRING,
	CHAR_NUMBER,
	CHAR_BOOLEAN,
	CHAR_UNARY
};

struct string_buffer {
	char * buffer;
	int size, pos;
};

// 1 for success, 0 for error
int  InitStringBuffer (struct string_buffer * buffer, int size);
void FreeStringBuffer (struct string_buffer * buffer);
void WriteStringBuffer(struct string_buffer * buffer, char c);

enum {
	LEXER_STRING,
	LEXER_STREAM
};

struct lexer {
	int mode;
	char * buffer, * pos;

	FILE * stream;
	int last_char;

	char bool_val;

	union {
		char * symbol,
		     * string;
	};

	int number_type;
	union {
		long long integer_val;
		struct { long long numerator, denominator; };
		double double_val;
	};

	int curr_line, curr_char;
	int current_token;
};

// 1 for success, 0 for error
int Lexer_LoadFromString(struct lexer * lex, char * string);
int Lexer_LoadFromFile(struct lexer * lex, FILE * file);
int Lexer_LoadFromStream(struct lexer * lex, FILE * stream);

// if there has been an error Lexer_GetError()
// will point to an error message string, otherwise
// its NULL for no error.
extern char * Lexer_ErrorString;
char * Lexer_GetError(void);
void   Lexer_SetError(char * str);

int Lexer_NextToken(struct lexer * lex);
int Lexer_CurrToken(struct lexer * lex);
int Lexer_GetCharType(int c);
int Lexer_IsValidSymbolChar(char c);
int Lexer_CurrChar(struct lexer * lex);
int Lexer_NextChar(struct lexer * lex);
int Lexer_EOF(struct lexer * lex);

void Lexer_HandleWhitespaces(struct lexer * lex);
void Lexer_HandleComments(struct lexer * lex);

int Lexer_AnalyseString(struct lexer * lex);
int Lexer_AnalyseSymbol(struct lexer * lex);
int Lexer_AnalyseNumber(struct lexer * lex);
int Lexer_AnalyseBoolean(struct lexer * lex);

void Lexer_Free(struct lexer * lex);
