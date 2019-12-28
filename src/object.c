#include "object.h"

int Scheme_AllocateObject(scheme_object ** object, int type) {
	*object = malloc(sizeof(scheme_object));
	if (!*object) {
		Scheme_SetError("runtime malloc(scheme_object) error");
		return 0;
	}

	switch (type) {
	case SCHEME_NULL:
		(*object)->payload = NULL;
		return 1;
	case SCHEME_PAIR:
		(*object)->payload = malloc(sizeof(scheme_pair));
		break;
	case SCHEME_NUMBER:
		(*object)->payload = malloc(sizeof(scheme_number));
		break;
	case SCHEME_SYMBOL:
		(*object)->payload = malloc(sizeof(scheme_symbol));
		break;
	case SCHEME_STRING:
		(*object)->payload = malloc(sizeof(scheme_string));
		break;
	default:
		free(*object);
		Scheme_SetError("invalid type given to Scheme_AllocateObject");
		return 0;
	}

	if ((*object)->payload == NULL) {
		free(*object);
		Scheme_SetError("runtime malloc(payload) error");
		return 0;
	}

	(*object)->type = type;

	return 1;
}

void Scheme_FreeObject(scheme_object * object) {
	if (object == NULL) return;

	#define freereturn(p) {p(object->payload);free(object);return;}
	switch (object->type) {
	case SCHEME_NULL:
		free(object);
		return;
	case SCHEME_PAIR:   freereturn(Scheme_FreePair);
	case SCHEME_NUMBER: freereturn(Scheme_FreeNumber);
	case SCHEME_SYMBOL: freereturn(Scheme_FreeSymbol);
	case SCHEME_STRING: freereturn(Scheme_FreeString);
	default: return;
	}
	#undef freereturn
}

void Scheme_FreePair(scheme_pair * pair) {
	if (pair == NULL) return;

	Scheme_FreeObject(pair->car);
	Scheme_FreeObject(pair->cdr);
	free(pair);
}

void Scheme_FreeNumber(scheme_number * number) {
	if (number == NULL) return;
	free(number);
}

void Scheme_FreeString(scheme_string * string) {
	if (string == NULL) return;
	if (string->string) free(string->string);
	free(string);
}

void Scheme_FreeSymbol(scheme_symbol * symbol) {
	if (symbol == NULL) return;
	if (symbol->symbol) free(symbol->symbol);
	free(symbol);
}

scheme_pair * Scheme_GetPair(scheme_object * obj) {
	if (obj->type != SCHEME_PAIR) {
		Scheme_SetError("Attempting to access non-pair object as a pair");
		return NULL;
	}

	return (scheme_pair *)obj->payload;
}

scheme_string * Scheme_GetString(scheme_object * obj) {
	if (obj->type != SCHEME_STRING) {
		Scheme_SetError("Attempting to access non-string object as a string");
		return NULL;
	}

	return (scheme_string *)obj->payload;
}

scheme_number * Scheme_GetNumber(scheme_object * obj) {
	if (obj->type != SCHEME_NUMBER) {
		Scheme_SetError("Attempting to access non-number object as a number");
		return NULL;
	}

	return (scheme_number *)obj->payload;
}

scheme_symbol * Scheme_GetSymbol(scheme_object * obj) {
	if (obj->type != SCHEME_SYMBOL) {
		Scheme_SetError("Attempting to access non-symbol object as a symbol");
		return NULL;
	}

	return (scheme_symbol *)obj->payload;
}

scheme_object * Scheme_CreateNull( void ) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NULL);
	if (!code) return NULL;
	return obj;
} 

scheme_object * Scheme_CreatePair(scheme_object * car, scheme_object * cdr) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_PAIR);
	if (!code) return NULL;

	scheme_pair * pair = Scheme_GetPair(obj);
	pair->car = car;
	pair->cdr = cdr;

	return obj;
}

scheme_object * Scheme_CreateSymbol(char * symbol_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_SYMBOL);
	if (!code) return NULL;

	scheme_symbol * symbol = Scheme_GetSymbol(obj);
	symbol->symbol = symbol_str;

	return obj;

}

scheme_object * Scheme_CreateString(char * string_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_STRING);
	if (!code) return NULL;

	scheme_string * string = Scheme_GetString(obj);
	string->string = string_str;

	return obj;
}

scheme_object * Scheme_CreateSymbolLiteral(const char * symbol_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_SYMBOL);
	if (!code) return NULL;

	int len = strlen(symbol_str);
	char * str = malloc(len + 1);
	strcpy(str, symbol_str);

	scheme_symbol * symbol = Scheme_GetSymbol(obj);
	symbol->symbol = str;

	return obj;

}

scheme_object * Scheme_CreateStringLiteral(const char * string_str) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_STRING);
	if (!code) return NULL;

	int len = strlen(string_str);
	char * str = malloc(len + 1);
	strcpy(str, string_str);

	scheme_string * string = Scheme_GetString(obj);
	string->string = str;

	return obj;
}

scheme_object * Scheme_CreateInteger(long long integer) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NUMBER);
	if (!code) return NULL;

	scheme_number * num = Scheme_GetNumber(obj);
	num->type = NUMBER_INTEGER;
	num->integer_val = integer;

	return obj;
}

scheme_object * Scheme_CreateDouble(double value) {
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NUMBER);
	if (!code) return NULL;

	scheme_number * num = Scheme_GetNumber(obj);
	num->type = NUMBER_DOUBLE;
	num->double_val = value;

	return obj;
}

scheme_object * Scheme_CreateRational(long long numerator,
                                      long long denominator)
{
	scheme_object * obj;
	int code = Scheme_AllocateObject(&obj, SCHEME_NUMBER);
	if (!code) return NULL;

	scheme_number * num = Scheme_GetNumber(obj);
	num->type = NUMBER_RATIONAL;
	num->numerator = numerator;
	num->denominator = denominator;

	return obj;

}
