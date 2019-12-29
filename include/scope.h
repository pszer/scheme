#pragma once

#include "object.h"

#define ENVIRONMENT_SIZE 2048
#define SCOPE_SIZE 8

typedef struct define {
	char * string;
	scheme_object * object;
} define;

define * Scheme_CreateDefine(char * string, scheme_object * obj);
define * Scheme_CreateDefineLiteral(const char * string, scheme_object * obj);
void Scheme_FreeDefine(define * def);

typedef struct def_list def_list;
struct def_list {
	define * def;
	def_list * next;	
};

def_list * Scheme_CreateDefList(char * string, scheme_object * obj);
def_list * Scheme_CreateDefListLiteral(const char * string, scheme_object * obj);
def_list * Scheme_CreateDefListFromDefine(define * def);
void Scheme_FreeDefList(def_list * list);
void Scheme_AppendDefList(def_list * list, define * def);
scheme_object * Scheme_FindDefList(def_list * list, const char * string);

typedef struct scope {
	int size;
	def_list ** defines;
} scope;
extern scope environment;

scope * Scheme_CreateScope(int size);
void    Scheme_FreeScope(scope * s);

void Scheme_AddDefine(scope * s, char * string, scheme_object * obj);
void Scheme_AddDefineEnvironment(char * string, scheme_object * obj);
void Scheme_AddDefineEnvironmentLiteral(const char * string, scheme_object * obj);

scheme_object * Scheme_GetDefine(scope * s, const char * string);

void Scheme_InitEnvironment();
void Scheme_FreeEnvironment();

int Scheme_HashFunction(const char * string);
