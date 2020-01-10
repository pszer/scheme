#include "parser.h"
#include "scheme.h"

void test_lexer(struct lexer * lex) {
	int token;

	scheme_object * obj;

	while ((token = Lexer_NextToken(lex)) != TOKEN_EOF) {
		switch (token) {
		case TOKEN_STRING:
			obj = Scheme_CreateString(lex->string);
			Scheme_Display(obj);
			Scheme_Newline();
			Scheme_FreeObject(obj);
			break;
		case TOKEN_SYMBOL:
			obj = Scheme_CreateSymbol(lex->symbol);
			Scheme_Display(obj);
			Scheme_Newline();
			Scheme_FreeObject(obj);
			break;
		case TOKEN_NUMBER:
			if (lex->number_type == NUMBER_DOUBLE)
				obj = Scheme_CreateDouble(lex->double_val);
			else if (lex->number_type == NUMBER_INTEGER)
				obj = Scheme_CreateInteger(lex->integer_val);
			Scheme_Display(obj);
			Scheme_Newline();
			Scheme_FreeObject(obj);
			break;
		default:
			printf("%c ", (char)token);
			Scheme_Newline();
			break;
		}
	}
	Scheme_Newline();

	char * err;
	if ((err = Lexer_GetError()) != NULL) {
		printf("Lexer: %s\n", err);
	}
}

#include <stdlib.h>

int main(int argc, char ** argv) {
	struct lexer lex;
	FILE * file = fopen("test.scm", "r");
	if (!file) {
		printf("Cannot open file\n");
		return 0;
	}

	if (!Lexer_LoadFromFile(&lex, file)) {
		printf("Error\n");
		return 0;
	}

	Scheme_DefineStartupEnv();

	//test_lexer(&lex);
	do {
		scheme_object * obj = Parser_Parse(&lex);
		if (!obj) break;

		Scheme_Display(Scheme_Eval(obj, &USER_INITIAL_ENVIRONMENT));
		printf("\n");
		scheme_object * car, * cdr, * base = obj;

		Scheme_FreeObject(obj);
	} while (Lexer_CurrToken(&lex) != TOKEN_EOF);

	Scheme_FreeStartupEnv();
	
	fclose(file);
	Lexer_Free(&lex);
	return 0;
}
