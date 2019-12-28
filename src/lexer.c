#include "lexer.h"

char * __ERR_MSG__EXPECTED_QUOTE__ = "Expected terminating \" character";
char * __ERR_MSG__MALFORMED_NUMBER__ = "Malformed number literal";

char * Lexer_ErrorString = NULL;
char * Lexer_GetError(void) {
	char * temp = Lexer_ErrorString;
	Lexer_ErrorString = NULL;
	return temp;
}

void Lexer_SetError(char * str) {
	Lexer_ErrorString = str;
}

int InitStringBuffer(struct string_buffer * buffer, int size) {
	buffer->buffer = calloc(size, sizeof(char));
	if (!buffer->buffer) return 0;

	buffer->size = size;
	buffer->buffer[0] = '\0';
	buffer->pos = 0;

	return 1;
}

void FreeStringBuffer(struct string_buffer * buffer) {
	if (buffer && buffer->buffer) free(buffer->buffer);
}

void WriteStringBuffer(struct string_buffer * buffer, char c) {
	// extend buffer size
	if (buffer->pos >= buffer->size - 2) {
		buffer->size *= 2;
		buffer->buffer = realloc(buffer->buffer, buffer->size);
	}

	
	buffer->buffer[buffer->pos] = c;
	buffer->pos++;
	buffer->buffer[buffer->pos] = '\0';
}

int Lexer_LoadFromString(struct lexer * lex, const char * string) {
	int size = strlen(string);
	
	lex->buffer = malloc(sizeof(char) * (size + 1));
	if (!lex->buffer) return 0;

	strcpy(lex->buffer, string);
	lex->pos = lex->buffer;

	lex->curr_line = 1;
	lex->curr_char = 1;

	return 1;
}

int Lexer_LoadFromFile (struct lexer * lex, FILE * file) {
	long long size, old_pos;

	old_pos = ftell(file);
	if (fseek(file, 0L, SEEK_END)) return 0; // get file size
	size = ftell(file);
	if (fseek(file, old_pos, SEEK_SET)) return 0; // return to previous pos

	lex->buffer = malloc(sizeof(char) * (size + 1));
	if (!lex->buffer) return 0;

	fread(lex->buffer, size, 1, file);
	lex->buffer[size] = '\0';
	lex->pos = lex->buffer;

	lex->curr_line = 1;
	lex->curr_char = 1;

	return 1;
}

void Lexer_Free(struct lexer * lex) {
	if (lex && lex->buffer) free(lex->buffer);
}

inline char Lexer_CurrChar(struct lexer * lex) {
	return *lex->pos;
}

char Lexer_NextChar(struct lexer * lex) {
	char c = *(++lex->pos);
	if (c == '\n') {
		++lex->curr_line;
		lex->curr_char = 1;
	}
	return c;
}

int Lexer_NextToken(struct lexer * lex) {
	Lexer_GetError(); // nulls error flag
	char last;

	while (1) {
		Lexer_HandleWhitespaces(lex);
		Lexer_HandleComments(lex);

		switch (Lexer_GetCharType(Lexer_CurrChar(lex))) {
		case CHAR_EOF:
			return TOKEN_EOF;
		case CHAR_STRING:
			return Lexer_AnalyseString(lex);
		case CHAR_SYMBOL:
			return Lexer_AnalyseSymbol(lex);
		case CHAR_NUMBER:
			return Lexer_AnalyseNumber(lex);
		case CHAR_UNARY:
			last = Lexer_CurrChar(lex);
			Lexer_NextChar(lex);
			return last;
		default:
			return TOKEN_EOF;
		}
	}
}

void Lexer_HandleWhitespaces(struct lexer * lex) {
	while (isspace(Lexer_CurrChar(lex))) {
		Lexer_NextChar(lex);
	}
}

void Lexer_HandleComments(struct lexer * lex) {
	if (Lexer_CurrChar(lex) == ';') {
		while (1) {
			if (Lexer_NextChar(lex) == '\0') {
				goto done;
			} else if (Lexer_CurrChar(lex) == '\n') {
				Lexer_NextChar(lex);
				goto done;
			} else {
				Lexer_NextChar(lex);
			}
		}
	}
done:
	Lexer_HandleWhitespaces(lex);
}

int Lexer_AnalyseString(struct lexer * lex) {
	struct string_buffer buff;
	InitStringBuffer(&buff, 16);

	Lexer_NextChar(lex); // 'eat' current " char
	WriteStringBuffer(&buff, Lexer_CurrChar(lex));

	while (Lexer_NextChar(lex) != '"') {
		if (Lexer_CurrChar(lex) == '\0') {
			Lexer_SetError(__ERR_MSG__EXPECTED_QUOTE__);
			return TOKEN_EOF;
		}

		WriteStringBuffer(&buff, Lexer_CurrChar(lex));
	}

	Lexer_NextChar(lex); // 'eat' current " char

	lex->string = buff.buffer;
	return TOKEN_STRING;
}

int Lexer_AnalyseSymbol(struct lexer * lex) {
	struct string_buffer buff;
	InitStringBuffer(&buff, 8);

	WriteStringBuffer(&buff, Lexer_CurrChar(lex));

	while (Lexer_IsValidSymbolChar(Lexer_NextChar(lex))) {
		WriteStringBuffer(&buff, Lexer_CurrChar(lex));
	}

	lex->symbol = buff.buffer;
	return TOKEN_SYMBOL;
}

int Lexer_AnalyseNumber(struct lexer * lex) {
	struct string_buffer buff;
	InitStringBuffer(&buff, 8);

	while (!isspace(Lexer_CurrChar(lex)) &&
	       Lexer_CurrChar(lex) != '\0'   &&
	       Lexer_GetCharType(Lexer_CurrChar(lex)) != CHAR_UNARY)
	{
		WriteStringBuffer(&buff, Lexer_CurrChar(lex));
		Lexer_NextChar(lex);
	}

	// special case where there is a '- symbol
	if (strcmp(buff.buffer, "-") == 0) {
		lex->symbol = buff.buffer;
		return TOKEN_SYMBOL;
	}

	int type = NUMBER_INTEGER;
	char * c;
	for (c = buff.buffer; *c; ++c) {
		if (*c == '.') {
			type = NUMBER_DOUBLE;
			break;
		}
	}

	lex->number_type = type;

	char * endptr, * lastchar;

	if (type == NUMBER_DOUBLE)
		lex->double_val   = strtod(buff.buffer, &endptr);
	else
		lex->integer_val = strtoll(buff.buffer, &endptr, 10);

	lastchar = buff.buffer + buff.pos;

	if (lastchar != endptr) {
		Lexer_SetError(__ERR_MSG__MALFORMED_NUMBER__);
		FreeStringBuffer(&buff);
		return TOKEN_EOF;
	}

	FreeStringBuffer(&buff);
	return TOKEN_NUMBER;
}

int Lexer_GetCharType(char c) {
	if (c == '\0')  return CHAR_EOF;
	if (c == '"')   return CHAR_STRING;
	if (isdigit(c) || c == '-' || c == '.') return CHAR_NUMBER;
	if (Lexer_IsValidSymbolChar(c)) return CHAR_SYMBOL;
	else return CHAR_UNARY;
}

int Lexer_IsValidSymbolChar(char c) {
	return c != '(' && c != ')' && c != '\'' && (isalnum(c) || ispunct(c));
}
