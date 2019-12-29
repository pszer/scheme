#include "scope.h"

scope environment;

define * Scheme_CreateDefine(char * string, scheme_object * obj) {
	define * def = malloc(sizeof(define));
	def->string = string;
	def->object = obj;
	return def;
}

define * Scheme_CreateDefineLiteral(const char * string, scheme_object * obj) {
	return Scheme_CreateDefine(Scheme_CopyStringHeap(string), obj);
}

void Scheme_FreeDefine(define * def) {
	if (!def) return;
	if (def->string) free(def->string);
	Scheme_FreeObject(def->object);
}

def_list * Scheme_CreateDefList(char * string, scheme_object * obj) {
	def_list * node = malloc(sizeof(def_list));
	node->def = Scheme_CreateDefine(string, obj);
	node->next = NULL;
	return node;
}

def_list * Scheme_CreateDefListLiteral(const char * string, scheme_object * obj) {
	return Scheme_CreateDefList(Scheme_CopyStringHeap(string), obj);
}

def_list * Scheme_CreateDefListFromDefine(define * def) {
	def_list * node = malloc(sizeof(def_list));
	node->def = def;
	node->next = NULL;
	return node;
}

void Scheme_FreeDefList(def_list * list) {
	if (!list) return;
	if (list->def) {
		Scheme_FreeDefine(list->def);
		free(list->def);
	}
	if (list->next) Scheme_FreeDefList(list->next);
	free(list);
}

void Scheme_AppendDefList(def_list * list, define * def) {
	if (strcmp(list->def->string, def->string) == 0) {
		Scheme_FreeDefine(list->def);
		list->def = def;
	} else if (list->next) {
		Scheme_AppendDefList(list->next, def);
	} else {
		list->next = Scheme_CreateDefListFromDefine(def);
	}
}

scheme_object * Scheme_FindDefList(def_list * list, const char * string) {
	if (list == NULL)
		return NULL;
	if (strcmp(string, list->def->string) == 0)
		return list->def->object;
	else 
		Scheme_FindDefList(list->next, string);
}

scope * Scheme_CreateScope(int size) {
	scope * s = malloc(sizeof(scope));
	s->size = size;
	s->defines = malloc(size * sizeof(def_list *));
}

void Scheme_FreeScope(scope * s) {
	if (!s) return;

	int i;
	for (i = 0; i < s->size; ++i) {
		if (s->defines[i])
			Scheme_FreeDefList(s->defines[i]);
	}

	free(s->defines);
	free(s);
}

void Scheme_AddDefine(scope * s, char * string, scheme_object * obj) {
	int index = Scheme_HashFunction(string) % s->size;
	if (s->defines[index])
		Scheme_AppendDefList(s->defines[index], Scheme_CreateDefine(string, obj));
	else
		s->defines[index] = Scheme_CreateDefList(string, obj);
}

void Scheme_AddDefineEnvironment(char * string, scheme_object * obj) {
	Scheme_AddDefine(&environment, string, obj);
}

void Scheme_AddDefineEnvironmentLiteral(const char * string, scheme_object * obj) {
	Scheme_AddDefineEnvironment(Scheme_CopyStringHeap(string), obj);
}

void Scheme_InitEnvironment() {
	environment.size = ENVIRONMENT_SIZE;
	environment.defines = malloc(environment.size * sizeof(def_list *));

	int i;
	for (i = 0; i < environment.size; ++i) {
		environment.defines[i] = NULL;
	}
}

void Scheme_FreeEnvironment() {
	if (environment.defines) {
		int i;
		for (i = 0; i < environment.size; ++i) {
			if (environment.defines[i]) {
				Scheme_FreeDefList(environment.defines[i]);
			}
		}

		free(environment.defines);
	}
}

scheme_object * Scheme_GetDefine(scope * s, const char * string) {
	int index = Scheme_HashFunction(string);
	return Scheme_FindDefList(s->defines[index], string);
}

int Scheme_HashFunction(const char * string) {
	int result = 57;
	const char * c;
	for (c = string; *c; ++c) {
		result <<= 1 + (*c%2);
		result += *c;
	}

	return result;
}
