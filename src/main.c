#include "lexer.h"

void test_lexer(struct lexer * lex) {
	int token;
	while ((token = Lexer_NextToken(lex)) != TOKEN_EOF) {
		switch (token) {
		case TOKEN_STRING:
			printf("\"%s\" ", lex->string);
			free(lex->string);
			break;
		case TOKEN_SYMBOL:
			printf("'%s ", lex->symbol);
			free(lex->symbol);
			break;
		case TOKEN_NUMBER:
			printf("%f ", lex->number);
			break;
		default:
			printf("'%c' ", (char)token);
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
