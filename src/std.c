#include "std.h"
#include "scheme.h"

scheme_object * __Exit__(scheme_object ** objs, scheme_object * env, size_t count) {
	SCHEME_INTERPRETER_HALT = 1;
	return NULL;
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

#define __CALL_ARITHMETIC(callname, func) \
scheme_object * callname(scheme_object ** objs, scheme_object * env, size_t count) { \
	scheme_number nums[count]; \
	size_t i; \
	for (i = 0; i < count; ++i) { \
		if (objs[i]->type != SCHEME_NUMBER) { \
			Scheme_SetError("+ expects only number arguments"); \
			return NULL; \
		} \
		nums[i] = *Scheme_GetNumber(objs[i]); \
	} \
	return func(nums, count); \
} 

__CALL_ARITHMETIC(__Scheme_CallAdd__, __Scheme_Add__);
__CALL_ARITHMETIC(__Scheme_CallSub__, __Scheme_Sub__);
__CALL_ARITHMETIC(__Scheme_CallMul__, __Scheme_Mul__);
__CALL_ARITHMETIC(__Scheme_CallDiv__, __Scheme_Div__);
__CALL_ARITHMETIC(__Scheme_CallAEqual__, __Scheme_Arithmetic_Equal__);
__CALL_ARITHMETIC(__Scheme_CallALessThan__, __Scheme_Arithmetic_LessThan__);
__CALL_ARITHMETIC(__Scheme_CallALessThanEqual__, __Scheme_Arithmetic_LessThanEqual__);
__CALL_ARITHMETIC(__Scheme_CallAGreaterThan__, __Scheme_Arithmetic_GreaterThan__);
__CALL_ARITHMETIC(__Scheme_CallAGreaterThanEqual__, __Scheme_Arithmetic_GreaterThanEqual__);

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
