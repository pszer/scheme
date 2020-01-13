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

		symbol * def_sym;
		ReferenceSymbol(&def_sym, Scheme_GetSymbol(def_list_pair->car)->sym);

		scheme_object ** lambda_args = malloc(sizeof(scheme_object*) * count);
		int i;
		lambda_args[0] = def_list_pair->cdr;
		for (i = 1; i < count; ++i) {
			lambda_args[i] = objs[i];
		}

		scheme_object * lambda = Scheme_Special_Lambda(lambda_args, env, count);
		free(lambda_args);

		scheme_env * env_pointer = Scheme_GetEnvObj(env);
		if (!env_pointer) {
			Scheme_SetError("bad environment");
			return NULL;
		}
		Scheme_DefineEnv(env_pointer, Scheme_CreateDefine(def_sym, lambda));

		return Scheme_CreateSymbolFromSymbol(def_sym);
	} else // variable definition
	if (definition_type == SCHEME_SYMBOL) {
		if (count > 2) {
			Scheme_SetError("(define variable val) : bad arg count");
			return NULL;
		}

		scheme_object * val;
		Scheme_ReferenceObject(&val, objs[1]);
		//val = Scheme_Eval(objs[1], env);
		symbol * def_sym;
		ReferenceSymbol(&def_sym, Scheme_GetSymbol(objs[0])->sym);

		scheme_env    * env_pointer = Scheme_GetEnvObj(env);
		if (!env_pointer) {
			Scheme_SetError("bad environment");
			return NULL;
		}

		Scheme_DefineEnv(env_pointer, Scheme_CreateDefine(def_sym, val));
		return Scheme_CreateSymbolFromSymbol(def_sym);
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

	symbol ** def_args = malloc(sizeof(symbol) * argc);
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
			ReferenceSymbol(&def_args[i], arg_sym->sym);
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

scheme_object * Scheme_Special_Quote(scheme_object ** objs, scheme_object* env, size_t count) {
	scheme_object * o;
	Scheme_ReferenceObject(&o, objs[0]);
	return o;

}

scheme_object * Scheme_Special_Cond(scheme_object ** objs, scheme_object* env, size_t count) {
	size_t i;

	for (i = 0; i < count; ++i) {
		scheme_object * base_obj = objs[i];
		
		if (!base_obj || base_obj->type != SCHEME_PAIR) {
			Scheme_SetError("(cond (predicate [clauses ...]) ...) : malformed syntax");
			return NULL;
		}

		scheme_pair * base_pair = Scheme_GetPair(base_obj);

		char predicate_bool = 0;

		scheme_object * predicate_expr = base_pair->car;
		scheme_object * clause_expr    = base_pair->cdr;
		scheme_object * predicate_val  = NULL;

		// check if special 'else' keyword
		if (!Scheme_IsNull(predicate_expr) && predicate_expr->type == SCHEME_SYMBOL
			&& Scheme_SymbolEq(Scheme_GetSymbol(predicate_expr)->sym, ELSE_SYMBOL))
		{
			predicate_bool = 1;
		} else {
			predicate_val = Scheme_Eval(predicate_expr, env);

			if (error_str) {
				return NULL;
			}

			predicate_bool = Scheme_BoolTest(predicate_val);
		}

		if (!predicate_bool) {
			Scheme_DereferenceObject(&predicate_val);
			continue;
		}

		if (Scheme_IsNull(clause_expr))
			return predicate_val;
		else
			Scheme_DereferenceObject(&predicate_val);

		while (1) {
			if (clause_expr->type != SCHEME_PAIR) {
				Scheme_SetError("(cond (predicate [clauses ...]) ...) : malformed syntax");
				return NULL;
			}

			scheme_pair * clause_pair = Scheme_GetPair(clause_expr);
			scheme_object * clause_val = Scheme_Eval(clause_pair->car, env);

			if (Scheme_IsNull(clause_pair->cdr)) {
				return clause_val;
			} else {
				Scheme_DereferenceObject(&clause_val);
				clause_expr = clause_pair->cdr;
			}
		}
	}

	return NULL;
}

scheme_object * Scheme_Special_Let(scheme_object ** objs, scheme_object* env, size_t count) {
	scheme_object * var_list_obj = objs[0];

	if (Scheme_IsNull(var_list_obj)) {
		Scheme_SetError("(let ((symbol value) ...) [clauses ...]) : malformed syntax");
		return NULL;
	}

	int var_list_len = Scheme_ListLength(var_list_obj);

	scheme_object * new_env_obj = Scheme_CreateEnvObj(env, var_list_len);
	scheme_env    * new_env     = Scheme_GetEnvObj(new_env_obj);

	int i;
	for (i = 0; i < var_list_len; ++i) {
		if (var_list_obj->type != SCHEME_PAIR) {
			Scheme_SetError("(let ((symbol value) ...) [clauses ...]) : malformed syntax");
			return NULL;
		}

		scheme_pair * var_list_pair = Scheme_GetPair(var_list_obj);
		scheme_object * var_obj = var_list_pair->car;
		if (Scheme_IsNull(var_obj) || var_obj->type != SCHEME_PAIR) {
			Scheme_SetError("(let ((symbol value) ...) [clauses ...]) : malformed syntax");
			Scheme_DereferenceObject(&new_env_obj);
			return NULL;
		}

		scheme_pair * var_pair = Scheme_GetPair(var_obj);

		scheme_object * var_sym  = var_pair->car,
		              * var_expr = var_pair->cdr;

		if (Scheme_IsNull(var_expr) || var_expr->type != SCHEME_PAIR) {
			Scheme_SetError("(let ((symbol value) ...) [clauses ...]) : malformed syntax");
			Scheme_DereferenceObject(&new_env_obj);
			return NULL;
		}
		var_expr = Scheme_GetPair(var_expr)->car;

		if (Scheme_IsNull(var_sym) || var_sym->type != SCHEME_SYMBOL) {
			Scheme_SetError("(let ((symbol value) ...) [clauses ...]) : malformed syntax");
			Scheme_DereferenceObject(&new_env_obj);
			return NULL;
		}

		symbol * sym;
		scheme_object * val = Scheme_Eval(var_expr, env);
		ReferenceSymbol(&sym, Scheme_GetSymbol(var_sym)->sym);

		Scheme_DefineEnv(new_env, Scheme_CreateDefine(sym, val));
	}

	int end = count-1;
	scheme_object * return_val = NULL;
	for (i = 1; i < count; ++i) {
		scheme_object * val = Scheme_Eval(objs[i], new_env_obj);
		if (i == end)
			return_val = val;
		else
			Scheme_DereferenceObject(&val);
	}

	Scheme_DereferenceObject(&new_env_obj);
	return return_val;
}
