#include "scheme.h"
#include "std.h"

scheme_env SYSTEM_GLOBAL_ENVIRONMENT;
scheme_env USER_INITIAL_ENVIRONMENT;

#define CREATESYSDEF(func, name, argc, dotargs) \
	Scheme_DefineEnv(&SYSTEM_GLOBAL_ENVIRONMENT, Scheme_CreateDefineLiteral(name, \
		Scheme_CreateCFunc(argc,dotargs,func))) 
void Scheme_DefineStartupEnv( void ) {
	SYSTEM_GLOBAL_ENVIRONMENT = Scheme_CreateEnv(NULL, 128);
	USER_INITIAL_ENVIRONMENT = Scheme_CreateEnv(&SYSTEM_GLOBAL_ENVIRONMENT, 128);

	CREATESYSDEF(__Scheme_CallAdd__, "+", 1, 1);
	CREATESYSDEF(__Scheme_CallSub__, "-", 1, 1);
	CREATESYSDEF(__Scheme_CallMul__, "*", 1, 1);
	CREATESYSDEF(__Scheme_CallDiv__, "/", 1, 1);
}

void Scheme_FreeStartupEnv( void ) {
	Scheme_FreeEnv(&SYSTEM_GLOBAL_ENVIRONMENT);
	Scheme_FreeEnv(&USER_INITIAL_ENVIRONMENT);
}

scheme_object * Scheme_Eval(scheme_object * obj, scheme_env * env) {
	if (obj == NULL) return NULL;

	switch (obj->type) {
	case SCHEME_PAIR: {
		scheme_pair * pair;
		scheme_object * apply_result, * node;
		scheme_object ** array;
		int length, i;

		pair = Scheme_GetPair(obj);

		length = Scheme_ListLength(pair->cdr);

		array = malloc(length * sizeof(scheme_object *));
		node = pair->cdr;
		for (i = 0; i < length; ++i) {
			scheme_pair * p = Scheme_GetPair(node);
			array[i] = Scheme_Eval(p->car, env);
			node = p->cdr;
		}

		apply_result = Scheme_Apply(Scheme_Eval(pair->car, env), array, length);
		free(array);
		if (error_str) return NULL;
		return apply_result;
		break; }
	case SCHEME_SYMBOL: {
		scheme_symbol * sym;
		scheme_define * def;

		sym = Scheme_GetSymbol(obj);
		def = Scheme_GetEnv(env, sym->symbol);
		if (def == NULL) {
			Scheme_SetError("unbound variable");
			return NULL;
		}

		return Scheme_Eval(def->object, env);
		}
	default:
		return obj;
	}
}

scheme_object * Scheme_Apply(scheme_object * func, scheme_object ** args, int arg_count) {
	if (func->type == SCHEME_LAMBDA) {
		return NULL;
	} else if (func->type == SCHEME_CFUNC) {
		scheme_cfunc * cfunc = Scheme_GetCFunc(func);
		if (arg_count < cfunc->arg_count) {
			Scheme_SetError("bad arg count");
			return NULL;
		} else if (arg_count > cfunc->arg_count && !cfunc->dot_args) {
			Scheme_SetError("bad arg count");
			return NULL;
		}
		
		scheme_object * result = cfunc->func(args, arg_count);
		return result;
	} else {
		Scheme_SetError("tried to call non-applicable object");
		return NULL;
	}
}

void Scheme_DisplayList(scheme_object * obj) {
	if (!obj) return;
	if (obj->type != SCHEME_PAIR) {
		Scheme_Display(obj);
		return;
	}

	Scheme_Display(Scheme_Car(obj));
	scheme_object * cdr = Scheme_Cdr(obj);
	if (cdr) {
		printf(" , ");
		Scheme_DisplayList(Scheme_Cdr(obj));
	}
}

void Scheme_Display(scheme_object * obj) {
	if (obj == NULL || obj->type == SCHEME_NULL) {
		printf("()");
		return;
	}

	scheme_number * num;
	scheme_string * str;
	scheme_symbol * sym;

	switch (obj->type) {
	case SCHEME_SYMBOL:
		sym = Scheme_GetSymbol(obj);
		printf("%s", sym->symbol);
		break;

	case SCHEME_STRING:
		str = Scheme_GetString(obj);
		printf("%s", str->string);
		break;

	case SCHEME_NUMBER:
		num = Scheme_GetNumber(obj);
		switch (num->type) {
		case NUMBER_INTEGER:
			printf("%lli", num->integer_val);
			break;
		case NUMBER_RATIONAL:
			printf("%lli/%lli", num->numerator, num->denominator);
			break;
		case NUMBER_DOUBLE:
			printf("%f", num->double_val);
			break;
		}
		break;

	case SCHEME_PAIR:
		putchar('(');
		Scheme_DisplayList(obj);
		printf(") ");
		break;
	}
}

void Scheme_Newline( void ) {
	putchar('\n');
}
