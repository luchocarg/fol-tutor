#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_FORALL, TOKEN_EXISTS, TOKEN_NOT,
    TOKEN_AND, TOKEN_OR, TOKEN_IMPLIES,
    TOKEN_ID_UPPER, TOKEN_ID_LOWER,
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

void lexer_skip_whitespace(Lexer* l);
Token lex_identifier(Lexer* l);
Token get_next_token(Lexer* l);
Token peek_token(Lexer* l);

#endif