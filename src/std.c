#include "std.h"
#include "scheme.h"
#include "parser.h"

scheme_object * __Exit__(scheme_object ** objs, scheme_object * env, size_t count) {
	SCHEME_INTERPRETER_HALT = 1;
	return NULL;
}

scheme_object * __Scheme_List__(scheme_object ** objs, scheme_object * env, size_t count) {
	if (count == 0) {
		return Scheme_CreatePairWithoutRef(NULL, NULL);
	}

	scheme_object * base = Scheme_CreatePair(objs[count-1], NULL);

	int i;
	for (i = count-2; i >= 0; --i) {
		scheme_object * car;
		Scheme_ReferenceObject(&car, objs[i]);

		base = Scheme_CreatePairWithoutRef(car, base);
	}

	return base;
}

scheme_object * __Scheme_cons__(scheme_object ** objs, scheme_object * env, size_t count) {
	return Scheme_CreatePair(objs[0], objs[1]);
}

scheme_object * __Scheme_car__(scheme_object ** objs, scheme_object * env, size_t count) {
	if (!objs[0] || objs[0]->type != SCHEME_PAIR) {
		Scheme_SetError("car on non-pair object");
		return NULL;
	}

	scheme_object * car;
	Scheme_ReferenceObject(&car, Scheme_Car(objs[0]));
	return car;
}

scheme_object * __Scheme_cdr__(scheme_object ** objs, scheme_object * env, size_t count) {
	if (!objs[0] || objs[0]->type != SCHEME_PAIR) {
		Scheme_SetError("car on non-pair object");
		return NULL;
	}

	scheme_object * cdr;
	Scheme_ReferenceObject(&cdr, Scheme_Cdr(objs[0]));
	return cdr;
}

scheme_object * __Pred_eq__(scheme_object ** objs, scheme_object * env, size_t count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_BOOLEAN);
	scheme_boolean * boolean = Scheme_GetBoolean(result);

	scheme_object * a = objs[0];
	scheme_object * b = objs[0];

	char a_is_sym = a && a->type == SCHEME_SYMBOL;
	char b_is_sym = b && b->type == SCHEME_SYMBOL;

	if (a_is_sym != b_is_sym) {
		boolean->val = 0;
	} else {
		if (a_is_sym && b_is_sym) {
			scheme_symbol * a_sym = Scheme_GetSymbol(a);
			scheme_symbol * b_sym = Scheme_GetSymbol(b);
			boolean->val = a_sym->sym->str == b_sym->sym->str;
		} else {
			if (a && b)
				boolean->val = a->payload == b->payload;
			else
				boolean->val = 0;
		}
	}

	return result;
}

scheme_object * __Pred_null__(scheme_object ** objs, scheme_object * env, size_t count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_BOOLEAN);
	scheme_boolean * boolean = Scheme_GetBoolean(result);
	boolean->val = Scheme_IsNull(objs[0]);
	return result;
}

void __Math_Complement__(scheme_number * left, scheme_number * right) {
	int ltype = left->type;
	int rtype = right->type;

	if (ltype == rtype) return;

	if (ltype == NUMBER_DOUBLE) {
		if (rtype == NUMBER_INTEGER) {
			__IntToDouble__(right);
			return;
		} else if (rtype == NUMBER_RATIONAL) {
			__RationalToDouble__(right);
		}
	}

	if (rtype == NUMBER_DOUBLE) {
		if (ltype == NUMBER_INTEGER) {
			__IntToDouble__(left);
			return;
		} else if (ltype == NUMBER_RATIONAL) {
			__RationalToDouble__(left);
		}
	}

	if (ltype == NUMBER_RATIONAL)
		if (rtype == NUMBER_INTEGER)
			__IntToRational__(right);	
	if (rtype == NUMBER_RATIONAL)
		if (ltype == NUMBER_INTEGER)
			__IntToRational__(left);	
}

long long gcd(long long a, long long b) {
	if (b > a) gcd(b, a);
	while (b) {
		long long t = b;
		b = a % b;
		a = t;
	}
	return a;
}

#include <stdio.h>
void __NormaliseRational__(scheme_number * num) {
	if (num->type != NUMBER_RATIONAL) return;
	long long divisor = gcd(num->numerator, num->denominator);

	if (divisor == num->denominator) {
		long long integer = num->numerator / divisor;
		num->type = NUMBER_INTEGER;
		num->integer_val = integer;
	} else {
		num->numerator /= divisor;
		num->denominator /= divisor;
	}
}

void __IntToRational__(scheme_number * num) {
	num->type = NUMBER_RATIONAL;
	num->numerator = num->integer_val;
	num->denominator = 1;
}

void __IntToDouble__(scheme_number * num) {
	num->type = NUMBER_DOUBLE;
	num->double_val = num->integer_val;
}

void __RationalToDouble__(scheme_number * num) {
	num->type = NUMBER_DOUBLE;
	num->double_val = num->numerator / (double)num->denominator;
}

#define __CALL_ARITHMETIC(callname, func, err) \
scheme_object * callname(scheme_object ** objs, scheme_object * env, size_t count) { \
	scheme_number nums[count]; \
	size_t i; \
	for (i = 0; i < count; ++i) { \
		if (objs[i]->type != SCHEME_NUMBER) { \
			Scheme_SetError(err " expects only number arguments"); \
			return NULL; \
		} \
		nums[i] = *Scheme_GetNumber(objs[i]); \
	} \
	return func(nums, count); \
} 

__CALL_ARITHMETIC(__Scheme_CallAdd__, __Scheme_Add__, "+");
__CALL_ARITHMETIC(__Scheme_CallSub__, __Scheme_Sub__, "-");
__CALL_ARITHMETIC(__Scheme_CallMul__, __Scheme_Mul__, "*");
__CALL_ARITHMETIC(__Scheme_CallDiv__, __Scheme_Div__, "/");
__CALL_ARITHMETIC(__Scheme_CallAEqual__, __Scheme_Arithmetic_Equal__, "=");
__CALL_ARITHMETIC(__Scheme_CallALessThan__, __Scheme_Arithmetic_LessThan__, "<");
__CALL_ARITHMETIC(__Scheme_CallALessThanEqual__, __Scheme_Arithmetic_LessThanEqual__, "<=");
__CALL_ARITHMETIC(__Scheme_CallAGreaterThan__, __Scheme_Arithmetic_GreaterThan__, ">");
__CALL_ARITHMETIC(__Scheme_CallAGreaterThanEqual__, __Scheme_Arithmetic_GreaterThanEqual__, ">=");

scheme_object * __Scheme_Add__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_NUMBER);

	scheme_number * r_num = Scheme_GetNumber(result);
	*r_num = *nums;

	int i = 1;
	while (i != count) {
		scheme_number * to_add = nums + i;
		__Math_Complement__(r_num, to_add);

		switch (r_num->type) {
		case NUMBER_INTEGER:
			r_num->integer_val += to_add->integer_val;
			break;
		case NUMBER_DOUBLE:
			r_num->double_val += to_add->double_val;
			break;
		case NUMBER_RATIONAL: {
			long long topleft  = r_num->numerator * to_add->denominator;
			long long topright = to_add->numerator * r_num->denominator;
			long long new_denom = to_add->denominator * r_num->denominator;
			r_num->numerator = topleft + topright;
			r_num->denominator = new_denom;
			break; }
		}

		++i;
	}

	if (r_num->type == NUMBER_RATIONAL)
		__NormaliseRational__(r_num);
	return result;
}

scheme_object * __Scheme_Sub__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_NUMBER);

	scheme_number * r_num = Scheme_GetNumber(result);
	*r_num = *nums;

	int i = 1;
	while (i != count) {
		scheme_number * to_add = nums + i;
		__Math_Complement__(r_num, to_add);

		switch (r_num->type) {
		case NUMBER_INTEGER:
			r_num->integer_val -= to_add->integer_val;
			break;
		case NUMBER_DOUBLE:
			r_num->double_val -= to_add->double_val;
			break;
		case NUMBER_RATIONAL: {
			long long topleft  = r_num->numerator * to_add->denominator;
			long long topright = to_add->numerator * r_num->denominator;
			long long new_denom = to_add->denominator * r_num->denominator;
			r_num->numerator = topleft - topright;
			r_num->denominator = new_denom;
			break; }
		}

		++i;
	}

	if (r_num->type == NUMBER_RATIONAL)
		__NormaliseRational__(r_num);
	return result;
}

scheme_object * __Scheme_Mul__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_NUMBER);

	scheme_number * r_num = Scheme_GetNumber(result);
	*r_num = *nums;

	int i = 1;
	while (i != count) {
		scheme_number * to_add = nums + i;
		__Math_Complement__(r_num, to_add);

		switch (r_num->type) {
		case NUMBER_INTEGER:
			r_num->integer_val *= to_add->integer_val;
			break;
		case NUMBER_DOUBLE:
			r_num->double_val *= to_add->double_val;
			break;
		case NUMBER_RATIONAL:
			r_num->numerator *= to_add->numerator;
			r_num->denominator *= to_add->denominator;
			break;
		}

		++i;
	}

	if (r_num->type == NUMBER_RATIONAL)
		__NormaliseRational__(r_num);
	return result;
}

scheme_object * __Scheme_Div__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_NUMBER);

	scheme_number * r_num = Scheme_GetNumber(result);
	*r_num = *nums;

	int i = 1;
	while (i != count) {
		scheme_number * to_add = nums + i;
		__Math_Complement__(r_num, to_add);

		switch (r_num->type) {
		case NUMBER_INTEGER: {
			long long numer = r_num->integer_val;
			r_num->type = NUMBER_RATIONAL;
			r_num->numerator = numer;
			r_num->denominator = to_add->integer_val;
			break; }
		case NUMBER_DOUBLE:
			r_num->double_val /= to_add->double_val;
			break;
		case NUMBER_RATIONAL:
			r_num->numerator *= to_add->denominator;
			r_num->denominator *= to_add->numerator;
			break;
		}

		++i;
	}

	if (r_num->type == NUMBER_RATIONAL)
		__NormaliseRational__(r_num);
	return result;
}

scheme_object * __Scheme_Arithmetic_Equal__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_BOOLEAN);

	char bool_val = 1;

	scheme_number * left    = nums;
	scheme_number * end_num = nums + count - 1;

	while (left < end_num) {
		scheme_number * right = left + 1;

		__Math_Complement__(left, right);

		switch (left->type) {
		case NUMBER_INTEGER:
			if (left->integer_val != right->integer_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_DOUBLE:
			if (left->double_val != right->double_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_RATIONAL:
			if (left->numerator != right->numerator ||
			    left->denominator != right->denominator)
			{
				bool_val = 0;
				goto finish;
			}
			break;
		}

		++left;
	}

finish:
	Scheme_GetBoolean(result)->val = bool_val;
	return result;
}

scheme_object * __Scheme_Arithmetic_LessThan__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_BOOLEAN);

	char bool_val = 1;

	scheme_number * left    = nums;
	scheme_number * end_num = nums + count - 1;

	while (left < end_num) {
		scheme_number * right = left + 1;

		__Math_Complement__(left, right);

		switch (left->type) {
		case NUMBER_INTEGER:
			if (left->integer_val >= right->integer_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_DOUBLE:
			if (left->double_val >= right->double_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_RATIONAL: {
			long long l = left->numerator * right->denominator;
			long long r = right->numerator * left->denominator;
			if (l >= r)
			{
				bool_val = 0;
				goto finish;
			}
			break;
			}
		}

		++left;
	}

finish:
	Scheme_GetBoolean(result)->val = bool_val;
	return result;

}

scheme_object * __Scheme_Arithmetic_LessThanEqual__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_BOOLEAN);

	char bool_val = 1;

	scheme_number * left    = nums;
	scheme_number * end_num = nums + count - 1;

	while (left < end_num) {
		scheme_number * right = left + 1;

		__Math_Complement__(left, right);

		switch (left->type) {
		case NUMBER_INTEGER:
			if (left->integer_val > right->integer_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_DOUBLE:
			if (left->double_val > right->double_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_RATIONAL: {
			long long l = left->numerator * right->denominator;
			long long r = right->numerator * left->denominator;
			if (l > r)
			{
				bool_val = 0;
				goto finish;
			}
			break; }
		}

		++left;
	}

finish:
	Scheme_GetBoolean(result)->val = bool_val;
	return result;
}

scheme_object * __Scheme_Arithmetic_GreaterThan__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_BOOLEAN);

	char bool_val = 1;

	scheme_number * left    = nums;
	scheme_number * end_num = nums + count - 1;

	while (left < end_num) {
		scheme_number * right = left + 1;

		__Math_Complement__(left, right);

		switch (left->type) {
		case NUMBER_INTEGER:
			if (left->integer_val <= right->integer_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_DOUBLE:
			if (left->double_val <= right->double_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_RATIONAL: {
			long long l = left->numerator * right->denominator;
			long long r = right->numerator * left->denominator;
			if (l <= r)
			{
				bool_val = 0;
				goto finish;
			}
			break; }
		}

		++left;
	}

finish:
	Scheme_GetBoolean(result)->val = bool_val;
	return result;

}

scheme_object * __Scheme_Arithmetic_GreaterThanEqual__(scheme_number * nums, int count) {
	scheme_object * result;
	Scheme_AllocateObject(&result, SCHEME_BOOLEAN);

	char bool_val = 1;

	scheme_number * left    = nums;
	scheme_number * end_num = nums + count - 1;

	while (left < end_num) {
		scheme_number * right = left + 1;

		__Math_Complement__(left, right);

		switch (left->type) {
		case NUMBER_INTEGER:
			if (left->integer_val < right->integer_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_DOUBLE:
			if (left->double_val < right->double_val) {
				bool_val = 0;
				goto finish;
			}
			break;
		case NUMBER_RATIONAL: {
			long long l = left->numerator * right->denominator;
			long long r = right->numerator * left->denominator;
			if (l < r)
			{
				bool_val = 0;
				goto finish;
			}
			break; }
		}

		++left;
	}

finish:
	Scheme_GetBoolean(result)->val = bool_val;
	return result;

}

scheme_object * __Scheme_CallDisplay__(scheme_object ** objs, scheme_object * env, size_t count) {
	Scheme_Display(objs[0]);
	return NULL;
}

scheme_object * __Scheme_CallNewline__(scheme_object ** objs, scheme_object * env, size_t count) {
	Scheme_Newline();
	return NULL;
}

scheme_object * __Scheme_Quotient__(scheme_object ** objs, scheme_object * env, size_t count) {
	scheme_object * dividend = objs[0];
	scheme_object * divisor  = objs[1];

	if (Scheme_IsNull(dividend) || Scheme_IsNull(divisor)) {
		Scheme_SetError("quotient : expects integer arguments");
		return NULL;
	}

	if (dividend->type != SCHEME_NUMBER || divisor->type != SCHEME_NUMBER) {
		Scheme_SetError("quotient : expects integer arguments");
		return NULL;
	}

	scheme_number * a = Scheme_GetNumber(dividend);
	scheme_number * b = Scheme_GetNumber(divisor);

	if (a->type != NUMBER_INTEGER || b->type != NUMBER_INTEGER) {
		Scheme_SetError("quotient : expects integer arguments");
		return NULL;
	}

	long long quotient = a->integer_val / b->integer_val;
	return Scheme_CreateInteger(quotient);
}

scheme_object * __Scheme_Modulo(scheme_object ** objs, scheme_object * env, size_t count) {
	scheme_object * dividend = objs[0];
	scheme_object * divisor  = objs[1];

	if (Scheme_IsNull(dividend) || Scheme_IsNull(divisor)) {
		Scheme_SetError("modulo : expects integer arguments");
		return NULL;
	}

	if (dividend->type != SCHEME_NUMBER || divisor->type != SCHEME_NUMBER) {
		Scheme_SetError("modulo : expects integer arguments");
		return NULL;
	}

	scheme_number * a = Scheme_GetNumber(dividend);
	scheme_number * b = Scheme_GetNumber(divisor);

	if (a->type != NUMBER_INTEGER || b->type != NUMBER_INTEGER) {
		Scheme_SetError("modulo : expects integer arguments");
		return NULL;
	}

	long long modulo = a->integer_val % b->integer_val;
	if (modulo < 0) modulo = -modulo;
	if (b->integer_val < 0) modulo = -modulo;
	return Scheme_CreateInteger(modulo);
}

scheme_object * __Scheme_Remainder__(scheme_object ** objs, scheme_object * env, size_t count) {
	scheme_object * dividend = objs[0];
	scheme_object * divisor  = objs[1];

	if (Scheme_IsNull(dividend) || Scheme_IsNull(divisor)) {
		Scheme_SetError("remainder : expects integer arguments");
		return NULL;
	}

	if (dividend->type != SCHEME_NUMBER || divisor->type != SCHEME_NUMBER) {
		Scheme_SetError("remainder : expects integer arguments");
		return NULL;
	}

	scheme_number * a = Scheme_GetNumber(dividend);
	scheme_number * b = Scheme_GetNumber(divisor);

	if (a->type != NUMBER_INTEGER || b->type != NUMBER_INTEGER) {
		Scheme_SetError("remainder : expects integer arguments");
		return NULL;
	}

	long long remainder = a->integer_val % b->integer_val;
	if (remainder < 0) remainder = -remainder;
	if (a->integer_val < 0) remainder = -remainder;
	return Scheme_CreateInteger(remainder);
}

scheme_object * __Scheme_Load__(scheme_object ** objs, scheme_object * env, size_t count) {
	if (Scheme_IsNull(objs[0]) || objs[0]->type != SCHEME_STRING) {
		Scheme_SetError("load expects a string");
		return NULL;
	}

	FILE * file = fopen(Scheme_GetString(objs[0])->string, "r");
	if (!file) {
		Scheme_SetError("cannot load file");
		return NULL;
	}


	struct lexer lex;
	if (!Lexer_LoadFromFile(&lex, file)) {
		Scheme_SetError(Lexer_GetError());
		fclose(file);
		return NULL;
	}

	fclose(file);
	scheme_object * eval_result = NULL;
	while (!Lexer_EOF(&lex)) {
		scheme_object * obj = Parser_Parse(&lex);
		if (!obj) break;

		if (eval_result)
			Scheme_DereferenceObject(&eval_result);

		scheme_object * eval_result = Scheme_Eval(obj, USER_INITIAL_ENVIRONMENT_OBJ);
		Scheme_DereferenceObject(&eval_result);
		Scheme_DereferenceObject(&obj);

		if (error_str) {
			eval_result = NULL;
			break;
		}
	}

	Lexer_Free(&lex);
	return NULL;
}
