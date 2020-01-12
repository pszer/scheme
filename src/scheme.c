#include "scheme.h"

scheme_env SYSTEM_GLOBAL_ENVIRONMENT;
scheme_env USER_INITIAL_ENVIRONMENT;
int SCHEME_INTERPRETER_HALT = 0;

#define CREATESYSDEF(func, name, argc, dotargs, special_form) \
	Scheme_DefineEnv(&SYSTEM_GLOBAL_ENVIRONMENT, Scheme_CreateDefineLiteral(name, \
		Scheme_CreateCFunc(argc,dotargs,special_form,func)))
#define CREATESPEC(func, name, tok) \
	Scheme_DefineEnv(&SYSTEM_GLOBAL_ENVIRONMENT, Scheme_CreateDefineLiteral(name, \
		Scheme_CreateCFunc(tok ## _ARGC,tok ## _DOT,1,func)))

void Scheme_DefineStartupEnv( void ) {
	SYSTEM_GLOBAL_ENVIRONMENT = Scheme_CreateEnv(NULL, 128);
	USER_INITIAL_ENVIRONMENT  = Scheme_CreateEnv(&SYSTEM_GLOBAL_ENVIRONMENT, 128);

	CREATESPEC(Scheme_Special_Define, "define", SPEC_DEFINE);
	CREATESPEC(Scheme_Special_If, "if", SPEC_IF);

	CREATESYSDEF(__Scheme_CallAdd__, "+", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallSub__, "-", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallMul__, "*", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallDiv__, "/", 1, 1, 0);
	CREATESYSDEF(__Exit__, "exit", 0, 0, 0);
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

		if (!pair->cdr)
			length=0;
		else
			length = Scheme_ListLength(pair->cdr);

		scheme_object * application = Scheme_Eval(pair->car, env);
		if (!application) return NULL;

		char is_special_form = 0;
		if (application->type == SCHEME_CFUNC) {
			scheme_cfunc * cfunc = Scheme_GetCFunc(application);
			is_special_form = cfunc->special_form;
		}

		array = malloc(length * sizeof(scheme_object *));
		node = pair->cdr;
		for (i = 0; i < length; ++i) {
			scheme_pair * p = Scheme_GetPair(node);
			if (!is_special_form)
				array[i] = Scheme_Eval(p->car, env);
			else
				array[i] = p->car;
			node = p->cdr;
		}

		apply_result = Scheme_Apply(application, array, length, env);

		Scheme_DereferenceObject(&application);
		if (!is_special_form) {
			for (i = 0; i < length; ++i) {
				Scheme_DereferenceObject(array + i);
			}
		}

		free(array);
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
	default: {
		scheme_object * ref;
		Scheme_ReferenceObject(&ref, obj);
		return ref; }
	}
}

scheme_object * Scheme_Apply(scheme_object * func, scheme_object ** args, int arg_count, scheme_env * env) {
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
		
		scheme_object * result = cfunc->func(args, env, arg_count);
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
	scheme_boolean * boolean;

	switch (obj->type) {
	case SCHEME_SYMBOL:
		sym = Scheme_GetSymbol(obj);
		printf("%s", sym->symbol);
		break;

	case SCHEME_STRING:
		str = Scheme_GetString(obj);
		printf("%s", str->string);
		break;

	case SCHEME_BOOLEAN:
		boolean = Scheme_GetBoolean(obj);
		printf("#%c", boolean->val ? 't' : 'f');
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

char Scheme_BoolTest(scheme_object * obj) {
	if (!obj) return 0;
	if (obj->type == SCHEME_BOOLEAN) {
		return Scheme_GetBoolean(obj)->val;
	} else if (obj->type == SCHEME_NUMBER) {
		scheme_number * num = Scheme_GetNumber(obj);

		switch (num->type) {
		case NUMBER_INTEGER : return num->integer_val != 0;
		case NUMBER_RATIONAL: return num->numerator   != 0;
		case NUMBER_DOUBLE  : return num->double_val  != 0.0;
		default: return 0;
		}
	} 

	return 0;
}
