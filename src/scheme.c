#include "scheme.h"

void Scheme_DisplayList(scheme_object * obj) {
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
