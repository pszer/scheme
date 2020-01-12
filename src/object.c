#include "object.h"
#include "scheme.h"

int Scheme_AllocateObject(scheme_object ** object, int type) {
	*object = malloc(sizeof(scheme_object));
	if (!*object) {
		Scheme_SetError("runtime malloc(scheme_object) error");
		return 0;
	}

	switch (type) {
	case SCHEME_NULL:
		(*object)->payload = NULL;
		return 1;
	case SCHEME_PAIR:
		(*object)->payload = malloc(sizeof(scheme_pair));
		break;
	case SCHEME_NUMBER:
		(*object)->payload = malloc(sizeof(scheme_number));
		break;
	case SCHEME_BOOLEAN:
		(*object)->payload = malloc(sizeof(scheme_boolean));
		break;
	case SCHEME_SYMBOL:
		(*object)->payload = malloc(sizeof(scheme_symbol));
		break;
	case SCHEME_STRING:
		(*object)->payload = malloc(sizeof(scheme_string));
		break;
	case SCHEME_LAMBDA:
		(*object)->payload = malloc(sizeof(scheme_lambda));
		break;
	case SCHEME_ENV:
		(*object)->payload = malloc(sizeof(scheme_env_obj));
		break;
	case SCHEME_CFUNC:
		(*object)->payload = malloc(sizeof(scheme_cfunc));
		break;
	default:
		free(*object);
		Scheme_SetError("invalid type given to Scheme_AllocateObject");
		return 0;
	}

	if ((*object)->payload == NULL) {
		free(*object);
		Scheme_SetError("runtime malloc(payload) error");
		return 0;
	}

	(*object)->type = type;
	(*object)->ref_count = 1;
	return 1;
}

void Scheme_ReferenceObject(scheme_object ** pointer, scheme_object * object) {
	*pointer = object;
	if (object) ++object->ref_count;
}

void Scheme_DereferenceObject(scheme_object ** pointer) {
	if (!pointer) return;

	if (*pointer) {
		//Scheme_Display(*pointer);
		//printf(" ref count : %i\n", (*pointer)->ref_count);
		(*pointer)->ref_count -= 1;
		if ((*pointer)->ref_count <= 0) {
			Scheme_FreeObject(*pointer);
		}
		*pointer = NULL;
	}
}

struct scheme_freed_memory * Scheme_InitFreedMemory() {
	struct scheme_freed_memory * mem = malloc(sizeof(struct scheme_freed_memory));
	mem->size = SCHEME_FREED_MEMORY_START_SIZE;
	mem->pos  = 0;

	mem->objects = malloc(sizeof(scheme_object *) * mem->size);
	return mem;
}

int Scheme_CheckIfFreed(struct scheme_freed_memory * mem, scheme_object * address) {
	int i;
	for (i = 0; i < mem->pos; ++i) {
		if (mem->objects[i] == address) return 1;
	}

	return 0;
}

void Scheme_AddFreed(struct scheme_freed_memory * mem, scheme_object * address) {
	if (mem->pos >= mem->size) {
		mem->size *= 2;
		mem->objects = realloc(mem->objects, mem->size * sizeof(scheme_object *));
	}

	mem->objects[mem->pos] = address;
	++mem->pos;
}

void Scheme_FreeFreedMemory(struct scheme_freed_memory * mem) {
	free(mem->objects);
	free(mem);
}

struct scheme_freed_memory * freed_mem = NULL;
void Scheme_FreeObject(scheme_object * object) {
	if (object == NULL) return;
	//Scheme_Display(object);
	//printf(" got freed!\n");

	#define freereturn(p) {p(object->payload);free(object);return;}
	switch (object->type) {
	case SCHEME_NULL:
		free(object);
		return;
	case SCHEME_PAIR: 
		freed_mem = Scheme_InitFreedMemory();
		Scheme_AddFreed(freed_mem, object);
		Scheme_FreePair(object->payload);
		free(object);
		Scheme_FreeFreedMemory(freed_mem);
		return;
	case SCHEME_NUMBER : freereturn(Scheme_FreeNumber);
	case SCHEME_BOOLEAN: freereturn(Scheme_FreeBoolean);
	case SCHEME_SYMBOL : freereturn(Scheme_FreeSymbol);
	case SCHEME_STRING : freereturn(Scheme_FreeString);
	case SCHEME_LAMBDA : freereturn(Scheme_FreeLambda);
	case SCHEME_CFUNC  : freereturn(Scheme_FreeCFunc);
	default: return;
	}
	#undef freereturn
}

void Scheme_FreeObjectRecur(scheme_object * object) {
	if (object == NULL) return;

	if (object->type != SCHEME_PAIR) {
		Scheme_DereferenceObject(&object);
	} else {
		Scheme_FreePair(object->payload);
		free(object);
	}
}

void Scheme_FreePair(scheme_pair * pair) {
	if (pair == NULL) return;

	#define CHECK_FREE(address) if (!Scheme_CheckIfFreed(freed_mem, address)) {\
                                        Scheme_AddFreed(freed_mem, address); \
	                                Scheme_FreeObjectRecur(address);}
	CHECK_FREE(pair->car);
	CHECK_FREE(pair->cdr);
	free(pair);
}

void Scheme_FreeNumber(scheme_number * number) {
	if (number == NULL) return;
	free(number);
}

void Scheme_FreeBoolean(scheme_boolean * boolean) {
	if (boolean == NULL) return;
	free(boolean);
}

void Scheme_FreeString(scheme_string * string) {
	if (string == NULL) return;
	if (string->string) free(string->string);
	free(string);
}

void Scheme_FreeSymbol(scheme_symbol * symbol) {
	if (symbol == NULL) return;
	if (symbol->symbol) free(symbol->symbol);
	free(symbol);
}

void Scheme_FreeLambda(scheme_lambda * lambda) {
	if (lambda == NULL) return;
	if (lambda->arg_ids) {
		/*int i;
		for (i = 0; i < lambda->arg_count; ++i) {
			free(lambda->arg_ids[i].symbol);
		}*/
		// the closure environment when freed will free
		// the argument symbol strings anyway
		free(lambda->arg_ids);
	}

	if (lambda->body) {
		int i;
		for (i = 0; i < lambda->body_count; ++i) {
			Scheme_DereferenceObject(&lambda->body[i]);
		} 
		free(lambda->body);
	}
	Scheme_FreeEnv(&lambda->closure);
	free(lambda);
}

void Scheme_FreeEnvObj(scheme_env_obj * env) {
	if (env == NULL) return;
	Scheme_FreeEnv(&env->env);
	free(env);
}

void Scheme_FreeCFunc(scheme_cfunc * cfunc) {
	if (cfunc) free(cfunc);
}

scheme_pair * Scheme_GetPair(scheme_object * obj) {
	if (obj->type != SCHEME_PAIR) {
		Scheme_SetError("Attempting to access non-pair object as a pair");
		return NULL;
	}

	return (scheme_pair *)obj->payload;
}

scheme_string * Scheme_GetString(scheme_object * obj) {
	if (obj->type != SCHEME_STRING) {
		Scheme_SetError("Attempting to access non-string object as a string");
		return NULL;
	}

	return (scheme_string *)obj->payload;
}

scheme_number * Scheme_GetNumber(scheme_object * obj) {
	if (obj->type != SCHEME_NUMBER) {
		Scheme_SetError("Attempting to access non-number object as a number");
		return NULL;
	}

	return (scheme_number *)obj->payload;
}

scheme_boolean * Scheme_GetBoolean(scheme_object * obj) {
	if (obj->type != SCHEME_BOOLEAN) {
		Scheme_SetError("Attempting to access non-boolean object as a boolean");
		return NULL;
	}

	return (scheme_boolean *)obj->payload;
}

scheme_symbol * Scheme_GetSymbol(scheme_object * obj) {
	if (obj->type != SCHEME_SYMBOL) {
		Scheme_SetError("Attempting to access non-symbol object as a symbol");
		return NULL;
	}

	return (scheme_symbol *)obj->payload;
}

scheme_lambda * Scheme_GetLambda(scheme_object * obj) {
	if (obj->type != SCHEME_LAMBDA) {
		Scheme_SetError("Attempting to access non-lambda object as a lambda");
		return NULL;
	}

	return (scheme_lambda *)obj->payload;
}

scheme_env_obj * Scheme_GetEnvObj(scheme_object * obj) {
	if (obj->type != SCHEME_ENV) {
		Scheme_SetError("Attempting to access non-environment object as a environment");
		return NULL;
	}

	return (scheme_env_obj *)obj->payload;
}

scheme_cfunc * Scheme_GetCFunc (scheme_object * obj) {
	if (obj->type != SCHEME_CFUNC) {
		Scheme_SetError("Attempting to access cfunc object as a cfunc");
		return NULL;
	}

	return (scheme_cfunc *)obj->payload;
}

scheme_object * Scheme_CreateNull( void ) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NULL);
	if (!code) return NULL;
	return obj;
} 

scheme_object * Scheme_CreatePair(scheme_object * car, scheme_object * cdr) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_PAIR);
	if (!code) return NULL;

	scheme_pair * pair = Scheme_GetPair(obj);
	pair->car = car;
	pair->cdr = cdr;

	return obj;
}

scheme_object * Scheme_CreateSymbol(char * symbol_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_SYMBOL);
	if (!code) return NULL;

	scheme_symbol * symbol = Scheme_GetSymbol(obj);
	symbol->symbol = symbol_str;

	return obj;
}

scheme_object * Scheme_CreateBoolean(char val) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_BOOLEAN);
	if (!code) return NULL;

	scheme_boolean * boolean = Scheme_GetBoolean(obj);
	boolean->val = val;

	return obj;
}

scheme_object * Scheme_CreateString(char * string_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_STRING);
	if (!code) return NULL;

	scheme_string * string = Scheme_GetString(obj);
	string->string = string_str;

	return obj;
}

scheme_object * Scheme_CreateSymbolLiteral(const char * symbol_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_SYMBOL);
	if (!code) return NULL;

	scheme_symbol * symbol = Scheme_GetSymbol(obj);
	symbol->symbol = strdup(symbol_str);

	return obj;

}

scheme_object * Scheme_CreateStringLiteral(const char * string_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_STRING);
	if (!code) return NULL;

	scheme_string * string = Scheme_GetString(obj);
	string->string = strdup(string_str);

	return obj;
}

scheme_object * Scheme_CreateInteger(long long integer) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NUMBER);
	if (!code) return NULL;

	scheme_number * num = Scheme_GetNumber(obj);
	num->type = NUMBER_INTEGER;
	num->integer_val = integer;

	return obj;
}

scheme_object * Scheme_CreateDouble(double value) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NUMBER);
	if (!code) return NULL;

	scheme_number * num = Scheme_GetNumber(obj);
	num->type = NUMBER_DOUBLE;
	num->double_val = value;

	return obj;
}

scheme_object * Scheme_CreateRational(long long numerator,
                                      long long denominator)
{
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NUMBER);
	if (!code) return NULL;

	scheme_number * num = Scheme_GetNumber(obj);
	num->type = NUMBER_RATIONAL;
	num->numerator = numerator;
	num->denominator = denominator;

	return obj;

}

scheme_object * Scheme_CreateLambda(int argc, char dot_args, scheme_symbol * args, int body_count,
	scheme_object ** body, scheme_env closure)
{
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_LAMBDA);
	if (!code) return NULL;

	scheme_lambda * l = Scheme_GetLambda(obj);
	l->arg_count = argc;
	l->dot_args = dot_args;
	l->arg_ids = args;
	l->body_count = body_count;
	l->body = body;
	l->closure = closure;

	return obj;
}

scheme_object * Scheme_CreateCFunc(int argc, char dot_args, char special_form,
	scheme_object* (*func)(scheme_object**,scheme_env*,size_t))
{
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_CFUNC);
	if (!code) return NULL;

	scheme_cfunc * c = Scheme_GetCFunc(obj);
	c->arg_count = argc;
	c->dot_args = dot_args;
	c->special_form = special_form;
	c->func = func;

	return obj;
}
