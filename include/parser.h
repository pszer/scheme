#pragma once

#include "lexer.h"

scheme_object * Parser_Parse(struct lexer * lex);
scheme_object * Parser_ParseExpression(struct lexer * lex);

scheme_object * Parser_ParseNumber (struct lexer * lex); 
scheme_object * Parser_ParseBoolean(struct lexer * lex); 
scheme_object * Parser_ParseSymbol (struct lexer * lex);
scheme_object * Parser_ParseString (struct lexer * lex);
scheme_object * Parser_ParseList   (struct lexer * lex);
scheme_object * Parser_ParseQuote  (struct lexer * lex);
