#include "scheme.h"

int SCHEME_INTERPRETER_HALT = 0;
scheme_object DO_TAIL_CALL;

scheme_object * SYSTEM_GLOBAL_ENVIRONMENT_OBJ;
scheme_object * USER_INITIAL_ENVIRONMENT_OBJ;
scheme_env * SYSTEM_GLOBAL_ENVIRONMENT;
scheme_env * USER_INITIAL_ENVIRONMENT;

symbol * ELSE_SYMBOL = NULL;

int Scheme_SymbolEq(symbol * a, symbol * b) {
	return a && b && (a->str == b->str);
}

#define CREATESYSDEF(func, name, argc, dotargs, special_form) \
	Scheme_DefineEnv(SYSTEM_GLOBAL_ENVIRONMENT, Scheme_CreateDefineString(strdup(name), \
		Scheme_CreateCFunc(argc,dotargs,special_form,func)))
#define CREATESPEC(func, name, tok) \
	Scheme_DefineEnv(SYSTEM_GLOBAL_ENVIRONMENT, Scheme_CreateDefineString(strdup(name), \
		Scheme_CreateCFunc(tok ## _ARGC,tok ## _DOT,1,func)))

void Scheme_DefineStartupEnv( void ) {
	SYSTEM_GLOBAL_ENVIRONMENT_OBJ = Scheme_CreateEnvObj(NULL, 128);
	USER_INITIAL_ENVIRONMENT_OBJ  = Scheme_CreateEnvObj(SYSTEM_GLOBAL_ENVIRONMENT_OBJ, 128);

	SYSTEM_GLOBAL_ENVIRONMENT = Scheme_GetEnvObj(SYSTEM_GLOBAL_ENVIRONMENT_OBJ);
	USER_INITIAL_ENVIRONMENT  = Scheme_GetEnvObj(USER_INITIAL_ENVIRONMENT_OBJ);

	CREATESPEC(Scheme_Special_Define, "define", SPEC_DEFINE);
	CREATESPEC(Scheme_Special_Lambda, "lambda", SPEC_LAMBDA);
	CREATESPEC(Scheme_Special_If, "if", SPEC_IF);
	CREATESPEC(Scheme_Special_Quote, "quote", SPEC_QUOTE);
	CREATESPEC(Scheme_Special_Cond, "cond", SPEC_COND);
	CREATESPEC(Scheme_Special_Let, "let", SPEC_LET);

	CREATESYSDEF(__Scheme_cons__, "cons", 2, 0, 0);
	CREATESYSDEF(__Scheme_car__,  "car", 1, 0, 0);
	CREATESYSDEF(__Scheme_cdr__,  "cdr", 1, 0, 0);
	CREATESYSDEF(__Scheme_List__, "list", 0, 1, 0);

	CREATESYSDEF(__Scheme_CallDisplay__, "display", 1, 0, 0);
	CREATESYSDEF(__Scheme_CallNewline__, "newline", 0, 0, 0);

	CREATESYSDEF(__Scheme_CallAdd__, "+", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallSub__, "-", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallMul__, "*", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallDiv__, "/", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallAEqual__, "=", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallALessThan__, "<", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallALessThanEqual__, "<=", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallAGreaterThan__, ">", 1, 1, 0);
	CREATESYSDEF(__Scheme_CallAGreaterThanEqual__, ">=", 1, 1, 0);

	CREATESYSDEF(__Scheme_Quotient__,  "quotient", 2, 0, 0);
	CREATESYSDEF(__Scheme_Modulo,      "modulo", 2, 0, 0);
	CREATESYSDEF(__Scheme_Remainder__, "remainder", 2, 0, 0);

	CREATESYSDEF(__Pred_eq__,   "eq?", 2, 0, 0);
	CREATESYSDEF(__Pred_null__, "null?", 1, 0, 0);

	CREATESYSDEF(__Exit__, "exit", 0, 0, 0);

	ELSE_SYMBOL = AddSymbol(strdup("else"));
}

void Scheme_FreeStartupEnv( void ) {
	Scheme_FreeObject(USER_INITIAL_ENVIRONMENT_OBJ);
	Scheme_FreeObject(SYSTEM_GLOBAL_ENVIRONMENT_OBJ);
}

scheme_call * call_stack;
scheme_call * call_stack_end;
void Scheme_InitCallStack(int stack_size) {
	call_stack = malloc(sizeof(scheme_call) * SCHEME_STACK_SIZE);
	call_stack_end = call_stack;
}

void Scheme_FreeCallStack(void) {
	if (call_stack) free(call_stack);
}

int Scheme_PushCallStack(scheme_call call) {
	// tail call check
	if (call_stack_end != call_stack) {
		scheme_call * last_call = call_stack_end-1;

		int do_tail_call = 0;
		if (call.is_cfunc_call && last_call->is_cfunc_call)
			do_tail_call = (call.cfunc == last_call->cfunc);
		else if (!call.is_cfunc_call && !last_call->is_cfunc_call)
			do_tail_call = (call.proc == last_call->proc);

		if (do_tail_call) {
			scheme_env * last_call_env = Scheme_GetEnvObj(last_call->env);
			scheme_env * this_call_env = Scheme_GetEnvObj(call.env);
			this_call_env->parent = last_call_env->parent;

			Scheme_DereferenceObject(&last_call->env);
			*last_call = call;
			return 1;
		}
	}

	*call_stack_end = call;
	++call_stack_end;
	return 0;
}

scheme_object * Scheme_PopCallStack(void) {
	if (call_stack_end == call_stack) {
		Scheme_SetError("call stack underflow");
	}

	scheme_object * result = Scheme_CallStack();
	--call_stack_end;
	return result;
}

scheme_object * Scheme_CallStack(void) {
	scheme_call * call = call_stack_end - 1;

tail_call:
	if (!call->is_cfunc_call) {
		/* lambda call */
		scheme_object * return_val = NULL;
		scheme_lambda * lambda = call->proc;

		int i;
		for (i = 0; i < lambda->body_count; ++i) {
			scheme_object * expr = lambda->body[i];
			scheme_object * body_eval = Scheme_Eval(expr,  call->env);
			if (i == lambda->body_count-1) {
				return_val = body_eval;
				break;
			} else {
				Scheme_DereferenceObject(&body_eval);
			}
		}

		if (return_val == &DO_TAIL_CALL) {
			goto tail_call;
		} else {
			Scheme_DereferenceObject(&call->env);
			return return_val;
		}
	} else {
		/* cfunc call */
		return call->cfunc->func(call->args, call->env, call->arg_count);
	}
}

char Scheme_CanTailCallLambda(scheme_lambda * lambda) {
	scheme_call * call;
	if (call_stack_end == call_stack) return 0;
	call = call_stack_end-1;
	return !call->is_cfunc_call && call->proc == lambda;
}

void Scheme_DisplayCallStack(void) {
	if (call_stack_end == call_stack) return;
	puts("-- STACK TRACE --");

	scheme_call * call = call_stack_end-1;
	while (1) {
		if (call->is_cfunc_call) {
			puts("<cfunc>");
		} else {
			scheme_object temp;
			temp.type = SCHEME_LAMBDA;
			temp.payload = call->proc;
			Scheme_Display(&temp);
			Scheme_Newline();
		}

		if (call == call_stack)
			break;
		--call;
	}
}

scheme_object * Scheme_Eval(scheme_object * obj, scheme_object * env) {
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

			if (error_str) {
				int j;
				for (j = 0; j < i && !is_special_form; ++j) {
					Scheme_DereferenceObject(array + j);
				}

				return NULL;
			}

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
		scheme_env    * env_pointer = Scheme_GetEnvObj(env);
		if (!env_pointer) {
			Scheme_SetError("bad environment");
			return NULL;
		}
		scheme_define * def;

		sym = Scheme_GetSymbol(obj);
		def = Scheme_GetEnv(env_pointer, sym->sym);
		if (def == NULL) {
			Scheme_SetError("unbound variable");
			return NULL;
		}

		scheme_object * ref;
		Scheme_ReferenceObject(&ref, def->object);
		return ref;
		}
	default: {
		scheme_object * ref;
		Scheme_ReferenceObject(&ref, obj);
		return ref; }
	}
}

scheme_object * Scheme_Apply(scheme_object * func, scheme_object ** args, int arg_count, scheme_object * env) {
	if (func->type == SCHEME_LAMBDA) {
		scheme_lambda * lambda = Scheme_GetLambda(func);
		return Scheme_ApplyLambda(lambda, args, arg_count, env);
	} else if (func->type == SCHEME_CFUNC) {
		scheme_cfunc * cfunc = Scheme_GetCFunc(func);
		if (!cfunc->special_form)
			return Scheme_ApplyCFunc(cfunc, args, arg_count, env);
		else
			return Scheme_ApplyCFuncSpecial(cfunc, args, arg_count, env);
	} else {
		Scheme_Display(func);
		Scheme_Newline();
		Scheme_SetError("tried to call non-applicable object");
		return NULL;
	}
}

scheme_object * Scheme_ApplyCFunc(scheme_cfunc * cfunc, scheme_object ** args, int arg_count, scheme_object * env) {
	if (!cfunc) return NULL;

	if (arg_count < cfunc->arg_count) {
		Scheme_SetError("bad arg count");
		return NULL;
	} else if (arg_count > cfunc->arg_count && !cfunc->dot_args) {
		Scheme_SetError("bad arg count");
		return NULL;
	}

	scheme_call cfunc_call;
	cfunc_call.is_cfunc_call = 1;
	cfunc_call.cfunc = cfunc;
	cfunc_call.args = args;
	cfunc_call.arg_count = arg_count;
	cfunc_call.env = env;
	//cfunc_call.env = env;

	Scheme_PushCallStack(cfunc_call);
	return Scheme_PopCallStack();	
	//scheme_object * result = cfunc->func(args, env, arg_count);
	//return result;
}

scheme_object * Scheme_ApplyCFuncSpecial(scheme_cfunc * cfunc, scheme_object ** args, int arg_count, scheme_object * env) {
	if (!cfunc) return NULL;

	if (arg_count < cfunc->arg_count) {
		Scheme_SetError("bad arg count");
		return NULL;
	} else if (arg_count > cfunc->arg_count && !cfunc->dot_args) {
		Scheme_SetError("bad arg count");
		return NULL;
	}

	return cfunc->func(args, env, arg_count);
}

scheme_object * Scheme_ApplyLambda(scheme_lambda * lambda, scheme_object ** args, int arg_count, scheme_object * env) {
	if (arg_count < lambda->arg_count) {
		Scheme_SetError("λ call error : too few arguments");
		return NULL;
	} else if (!lambda->dot_args && arg_count > lambda->arg_count) {
		Scheme_SetError("λ call error : too many arguments");
		return NULL;
	}

	int i;
	//scheme_object * new_env_obj = Scheme_CreateEnvObj(env, lambda->arg_count+1);
	scheme_object * new_env_obj;
	Scheme_ReferenceObject(&new_env_obj, lambda->closure);
	scheme_env    * new_env = (scheme_env*)new_env_obj->payload;
	for (i = 0; i < arg_count; ++i) {
		symbol * sym;
		ReferenceSymbol(&sym, lambda->arg_ids[i]);

		scheme_object * arg_val;
		Scheme_ReferenceObject(&arg_val, args[i]);
		Scheme_DefineEnv(new_env, Scheme_CreateDefine(sym, arg_val));
	}

	scheme_call call;
	call.is_cfunc_call = 0;
	call.proc = lambda;
	call.env = new_env_obj;

	if (Scheme_PushCallStack(call))
		return &DO_TAIL_CALL;
	return Scheme_PopCallStack();
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
		printf("%s", sym->sym->str);
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

	case SCHEME_LAMBDA: {
		scheme_lambda * lambda = Scheme_GetLambda(obj);
		int i;
		printf("λ(");
		for (i = 0; i < lambda->arg_count; ++i) {
			printf("%s", lambda->arg_ids[i]->str);
			if (i != lambda->arg_count-1) putchar(' ');
		}
		putchar(')');

		for (i = 0; i < lambda->body_count; ++i) {
			Scheme_Display(lambda->body[i]);
			if (i != lambda->body_count-1)
				putchar(' ');
		}
	} break;

	case SCHEME_CFUNC:
		printf("<cfunc>");
		break;

	case SCHEME_ENV:
		printf("<env>");
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

	return 1;
}
