#include "lexer.h"
#include <ctype.h>
#include <string.h>

typedef struct {
    const char* keyword;
    TokenType type;
    size_t length;
} KeywordMap;

typedef struct {
    const char* utf8_seq;
    TokenType type;
    size_t length;
} UTF8Map;

static const KeywordMap KEYWORD_TABLE[] = {
    {"forall", TOKEN_FORALL, 6},
    {"exists", TOKEN_EXISTS, 6},
    {"and",    TOKEN_AND,    3},
    {"or",     TOKEN_OR,     2},
    {"not",    TOKEN_NOT,    3},
    {NULL,     TOKEN_ERROR,  0}
};

static const UTF8Map UNICODE_TABLE[] = {
    {"∀", TOKEN_FORALL, 3},
    {"∃", TOKEN_EXISTS, 3},
    {"∧", TOKEN_AND,    3},
    {"∨", TOKEN_OR,     3},
    {"¬", TOKEN_NOT,    2},
    {"⊥", TOKEN_FALSUM, 3},
    {NULL, TOKEN_ERROR, 0}
};

static const UTF8Map MULTIBYTE_TABLE[] = {
    {"->", TOKEN_IMPLIES, 2},
    {"=>", TOKEN_IMPLIES, 2},
    {NULL, TOKEN_ERROR,   0}
};

static TokenType match_keyword(const char* start, size_t len) {
    for (int i = 0; KEYWORD_TABLE[i].keyword != NULL; i++) {
        if (len == KEYWORD_TABLE[i].length && strncmp(start, KEYWORD_TABLE[i].keyword, len) == 0) {
            return KEYWORD_TABLE[i].type;
        }
    }
    return TOKEN_ERROR;
}

static TokenType classify_single_char(char c) {
    switch (c) {
        case '(': return TOKEN_LEFT_PARENT;
        case ')': return TOKEN_RIGHT_PARENT;
        case '.': return TOKEN_DOT;
        case ',': return TOKEN_COMMA;
        default:  return TOKEN_ERROR;
    }
}

void lexer_skip_whitespace(Lexer* l) {
    while (l->source[l->cursor] != '\0' && isspace((unsigned char)l->source[l->cursor])) {
        l->cursor++;
    }
}

Token lex_identifier(Lexer* l) {
    const char* start = &l->source[l->cursor];
    int start_pos = l->cursor;

    while (l->source[l->cursor] != '\0' && 
          (isalnum((unsigned char)l->source[l->cursor]) || l->source[l->cursor] == '_')) {
        l->cursor++;
    }

    size_t len = (size_t)(l->cursor - start_pos);
    TokenType type = match_keyword(start, len);

    if (type == TOKEN_ERROR) {
        type = isupper((unsigned char)start[0]) ? TOKEN_ID_UPPER : TOKEN_ID_LOWER;
    }

    return (Token){type, start, len};
}

static Token match_sequence(Lexer* l, const UTF8Map* table) {
    const char* start = &l->source[l->cursor];
    for (int i = 0; table[i].utf8_seq != NULL; i++) {
        if (strncmp(start, table[i].utf8_seq, table[i].length) == 0) {
            l->cursor += (int)table[i].length;
            return (Token){table[i].type, start, table[i].length};
        }
    }
    return (Token){TOKEN_ERROR, start, 0};
}

Token get_next_token(Lexer* l) {
    lexer_skip_whitespace(l);

    const char* start = &l->source[l->cursor];
    if (start[0] == '\0') return (Token){TOKEN_EOF, NULL, 0};

    if (isalpha((unsigned char)start[0]) || start[0] == '_') {
        return lex_identifier(l);
    }

    Token t = match_sequence(l, MULTIBYTE_TABLE);
    if (t.type != TOKEN_ERROR) return t;

    if ((unsigned char)start[0] >= 0x80) {
        t = match_sequence(l, UNICODE_TABLE);
        if (t.type != TOKEN_ERROR) return t;
    }

    l->cursor++;
    return (Token){classify_single_char(start[0]), start, 1};
}

Token peek_token(Lexer* l) {
    int prev_cursor = l->cursor;
    Token t = get_next_token(l);
    l->cursor = prev_cursor;
    return t;
}