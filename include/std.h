#pragma once

#include "object.h"

void __Math_Complement__(scheme_number * left, scheme_number * right);

long long gcd(long long a, long long b);

void __IntToRational__(scheme_number * num);
void __IntToDouble__(scheme_number * num);
void __RationalToDouble__(scheme_number * num);
void __NormaliseRational__(scheme_number * num);

scheme_object * __Scheme_CallAdd__(scheme_object ** objs, size_t count);
scheme_object * __Scheme_CallSub__(scheme_object ** objs, size_t count);
scheme_object * __Scheme_CallMul__(scheme_object ** objs, size_t count);
scheme_object * __Scheme_CallDiv__(scheme_object ** objs, size_t count);
scheme_object * __Scheme_Add__(scheme_number * nums, int count);
scheme_object * __Scheme_Sub__(scheme_number * nums, int count);
scheme_object * __Scheme_Mul__(scheme_number * nums, int count);
scheme_object * __Scheme_Div__(scheme_number * nums, int count);
