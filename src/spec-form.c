#include "std.h"
#include "scheme.h"

scheme_object * Scheme_Special_Define(scheme_object ** objs, scheme_object* env, size_t count) {
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

		/*int i;
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

		scheme_object * closure;
		Scheme_ReferenceObject(&closure, env);

		scheme_object * lambda = Scheme_CreateLambda(argc, 0, def_args, body_count, body, closure);*/

		scheme_object ** lambda_args = malloc(sizeof(scheme_object*) * count);
		int i;
		lambda_args[0] = def_list_pair->cdr;
		for (i = 1; i < count; ++i) {
			lambda_args[i] = objs[i];
		}

		scheme_object * lambda = Scheme_Special_Lambda(lambda_args, env, count);
		free(lambda_args);

		scheme_env    * env_pointer = Scheme_GetEnvObj(env);
		if (!env_pointer) {
			Scheme_SetError("bad environment");
			return NULL;
		}
		Scheme_DefineEnv(env_pointer, Scheme_CreateDefine(strdup(def_name), lambda));

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

		scheme_env    * env_pointer = Scheme_GetEnvObj(env);
		if (!env_pointer) {
			Scheme_SetError("bad environment");
			return NULL;
		}

		Scheme_DefineEnv(env_pointer, Scheme_CreateDefine(strdup(sym->symbol), val));
		return Scheme_CreateSymbol(strdup(sym->symbol));
	}

	Scheme_SetError("(define ...) : malformed syntax");
	return NULL;
}

scheme_object * Scheme_Special_Lambda(scheme_object ** objs, scheme_object* env, size_t count) {
	int i;
	int argc;

	if (!objs[0] || objs[0]->type == SCHEME_NULL) {
		argc = 0;
	} else if (objs[0]->type != SCHEME_PAIR) {
		Scheme_SetError("(lambda (args) ...) : malformed syntax : expected args list");
		return NULL;
	} else {
		argc = Scheme_ListLength(objs[0]);
	}

	scheme_symbol * def_args = malloc(sizeof(scheme_symbol) * (argc));
	if (argc) {
		scheme_object * pair_i = objs[0];
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
	}

	int body_count = count - 1;
	scheme_object ** body = malloc(sizeof(scheme_object *) * body_count);
	for (i = 0; i < body_count; ++i) {
		Scheme_ReferenceObject(&body[i], objs[i+1]);
	}

	scheme_object * closure;
	Scheme_ReferenceObject(&closure, env);

	scheme_object * lambda = Scheme_CreateLambda(argc, 0, def_args, body_count, body, closure);
	return lambda;
}

scheme_object * Scheme_Special_If(scheme_object ** objs, scheme_object* env, size_t count) {
	scheme_object * cond_eval = Scheme_Eval(objs[0], env);
	char cond = Scheme_BoolTest(cond_eval);
	Scheme_DereferenceObject(&cond_eval);

	if (cond) {
		return Scheme_Eval(objs[1], env);
	} else {
		return Scheme_Eval(objs[2], env);
	}
}
