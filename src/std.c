#include "std.h"

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

scheme_object * __Scheme_CallAdd__(scheme_object ** objs, size_t count) {
	scheme_number nums[count];

	size_t i;
	for (i = 0; i < count; ++i) {
		if (objs[i]->type != SCHEME_NUMBER) {
			Scheme_SetError("+ expects only number arguments");
			return NULL;
		}

		nums[i] = *Scheme_GetNumber(objs[i]);
	}

	return __Scheme_Add__(nums, count);
}

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
		}

		++i;
	}

	return result;
}
