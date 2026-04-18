#include <stddef.h>

#ifndef LEXER_H
#define LEXER_H

typedef enum {
	TOKEN_FORALL, TOKEN_EXISTS, TOKEN_NOT,

	TOKEN_AND, TOKEN_OR, TOKEN_IMPLIES,

	TOKEN_LOWER_INDENT, TOKEN_UPPER_INDENT,

	TOKEN_LEFT_PARENT, TOKEN_RIGHT_PARENT, TOKEN_DOT, TOKEN_COMMA,

	TOKEN_FALSUM, TOKEN_EOF, TOKEN_ERROR
} TokenType;

typedef struct {
	TokenType type;
	const char* start;
	size_t length;
} Token;

typedef struct {
    const char* source;
    int cursor;
} Lexer;

void skip_whitespace_and_control(Lexer* l);
Token lex_identifier(Lexer* l);
Token get_next_token(Lexer* l);

#endif