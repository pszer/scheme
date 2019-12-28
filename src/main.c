#include "lexer.h"
#include "list.h"

void test_lexer(struct lexer * lex) {
	int token;

	scheme_object * obj;

	while ((token = Lexer_NextToken(lex)) != TOKEN_EOF) {
		switch (token) {
		case TOKEN_STRING:
			obj = Scheme_CreateString(lex->string);
			printf("\"%s\" ", Scheme_GetString(obj)->string);
			Scheme_FreeObject(obj);
			break;
		case TOKEN_SYMBOL:
			obj = Scheme_CreateSymbol(lex->symbol);
			printf("'%s ", Scheme_GetSymbol(obj)->symbol);
			Scheme_FreeObject(obj);
			break;
		case TOKEN_NUMBER:
			obj = Scheme_CreateDouble(lex->number);
			printf("%f ", Scheme_GetNumber(obj)->double_val);
			Scheme_FreeObject(obj);
			break;
		default:
			printf("%c ", (char)token);
			break;
		}
	}
	putchar('\n');

	char * err;
	if ((err = Lexer_GetError()) != NULL) {
		printf("Lexer: %s\n", err);
	}
}

int main( int argc, char ** argv ) {
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

	test_lexer(&lex);

	fclose(file);
	Lexer_Free(&lex);
	return 0;
}
