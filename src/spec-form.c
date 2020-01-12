#include "std.h"
#include "scheme.h"

scheme_object * Scheme_Special_Define(scheme_object ** objs, scheme_env* env, size_t count) {
	char definition_type = objs[0]->type;

	// function definition
	if (definition_type == SCHEME_PAIR) {
		int len = Scheme_ListLength(objs[0]);
		int argc = len-1;

		if (len == 0) {
			Scheme_SetError("(define (func ...) [body]) : malformed syntax");
			return NULL;
		}

		scheme_pair * def_list_pair = Scheme_GetPair(objs[0]);
		if (!def_list_pair->car || def_list_pair->car->type != SCHEME_SYMBOL) {
			Scheme_SetError("(define (func ...) [body]) : non-symbol given as procedure name");
			return NULL;
		}

		char * def_name = Scheme_GetSymbol(def_list_pair->car)->symbol;

		int i;
		scheme_object * pair_i = def_list_pair->cdr;
		scheme_symbol * def_args = malloc(sizeof(scheme_symbol) * (argc));
		for (i = 0; i < argc; ++i) {
			char type = pair_i->type;
			if (type != SCHEME_PAIR) {
				Scheme_SetError("(define (func ...) [body]) : bad argument list");
				free(def_args);
				return NULL;
			}

			scheme_pair * pair_pair = Scheme_GetPair(pair_i);
			if (!pair_pair->car) {
				Scheme_SetError("(define (func ...) [body]) : bad argument list");
				free(def_args);
				return NULL;
			}

			char car_type = pair_pair->car->type;
			if (car_type != SCHEME_SYMBOL) {
				Scheme_SetError("(define (func ...) [body]) : non-symbol in argument list");
				free(def_args);
				return NULL;
			}

			scheme_symbol * arg_sym = Scheme_GetSymbol(pair_pair->car);
			def_args[i].symbol = strdup(arg_sym->symbol);
			pair_i = pair_pair->cdr;
		}

		int body_count = count - 1;
		scheme_object ** body = malloc(sizeof(scheme_object *) * body_count);
		for (i = 0; i < body_count; ++i) {
			Scheme_ReferenceObject(&body[i], objs[i+1]);
		}

		scheme_env closure = Scheme_CreateEnv(env, argc+1);
		for (i = 0; i < argc; ++i) {
			Scheme_DefineEnv(&closure, Scheme_CreateDefine(def_args[i].symbol, NULL));
		}

		//Scheme_DefineEnv(env, Scheme_CreateDefine(sym->symbol, val));
		scheme_object * lambda = Scheme_CreateLambda(argc, 0, def_args, body_count, body, closure);
		Scheme_DefineEnv(env, Scheme_CreateDefine(strdup(def_name), lambda));

		return Scheme_CreateSymbol(strdup(def_name));
	} else // variable definition
	if (definition_type == SCHEME_SYMBOL) {
		if (count > 2) {
			Scheme_SetError("(define variable val) : bad arg count");
			return NULL;
		}

		scheme_object * val;
		//Scheme_ReferenceObject(&val, Scheme_Eval(objs[1], env));
		val = Scheme_Eval(objs[1], env);
		scheme_symbol * sym = Scheme_GetSymbol(objs[0]);

		Scheme_DefineEnv(env, Scheme_CreateDefine(strdup(sym->symbol), val));
		return Scheme_CreateSymbol(strdup(sym->symbol));
	}

	Scheme_SetError("(define ...) : malformed syntax");
	return NULL;
}

scheme_object * Scheme_Special_If(scheme_object ** objs, scheme_env* env, size_t count) {
	scheme_object * cond_eval = Scheme_Eval(objs[0], env);
	char cond = Scheme_BoolTest(cond_eval);
	Scheme_DereferenceObject(&cond_eval);

	if (cond) {
		return Scheme_Eval(objs[1], env);
	} else {
		return Scheme_Eval(objs[2], env);
	}
}
