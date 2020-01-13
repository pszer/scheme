#pragma once

#include "object.h"

scheme_object * __Exit__(scheme_object ** objs, scheme_object * env, size_t count);

scheme_object * __Scheme_cons__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_car__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_cdr__(scheme_object ** objs, scheme_object * env, size_t count);
//scheme_object * __Scheme_caar__(scheme_object ** objs, scheme_object * env, size_t count);
//scheme_object * __Scheme_cddr__(scheme_object ** objs, scheme_object * env, size_t count);

scheme_object * __Pred_eq__(scheme_object ** objs, scheme_object * env, size_t count);

void __Math_Complement__(scheme_number * left, scheme_number * right);

long long gcd(long long a, long long b);

void __IntToRational__(scheme_number * num);
void __IntToDouble__(scheme_number * num);
void __RationalToDouble__(scheme_number * num);
void __NormaliseRational__(scheme_number * num);

scheme_object * __Scheme_CallAdd__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_CallSub__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_CallMul__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_CallDiv__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_Add__(scheme_number * nums, int count);
scheme_object * __Scheme_Sub__(scheme_number * nums, int count);
scheme_object * __Scheme_Mul__(scheme_number * nums, int count);
scheme_object * __Scheme_Div__(scheme_number * nums, int count);

scheme_object * __Scheme_CallAEqual__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_CallALessThan__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_CallALessThanEqual__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_CallAGreaterThan__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_CallAGreaterThanEqual__(scheme_object ** objs, scheme_object * env, size_t count);
scheme_object * __Scheme_Arithmetic_Equal__(scheme_number * nums, int count);
scheme_object * __Scheme_Arithmetic_LessThan__(scheme_number * nums, int count);
scheme_object * __Scheme_Arithmetic_LessThanEqual__(scheme_number * nums, int count);
scheme_object * __Scheme_Arithmetic_GreaterThan__(scheme_number * nums, int count);
scheme_object * __Scheme_Arithmetic_GreaterThanEqual__(scheme_number * nums, int count);

scheme_object * __Scheme_CallDisplay__(scheme_object ** objs, scheme_object * env, size_t count);
