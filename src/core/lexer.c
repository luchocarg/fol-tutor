#include "core/lexer.h"
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
    {"forall",  TOKEN_FORALL,  6},
    {"exists",  TOKEN_EXISTS,  6},
    {"and",     TOKEN_AND,     3},
    {"or",      TOKEN_OR,      2},
    {"not",     TOKEN_NOT,     3},
    {"implies", TOKEN_IMPLIES, 7},
    {"bot",     TOKEN_FALSUM,  3},
    {NULL,      TOKEN_ERROR,   0}
};

static const UTF8Map UNICODE_TABLE[] = {
    {"∀", TOKEN_FORALL, 3},
    {"∃", TOKEN_EXISTS, 3},
    {"∧", TOKEN_AND,    3},
    {"∨", TOKEN_OR,     3},
    {"¬", TOKEN_NOT,    2},
    {"⊥", TOKEN_FALSUM, 3},
    {"⇒", TOKEN_IMPLIES, 3},
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

static Token match_sequence(Lexer* l, const UTF8Map* table) {
    const char* start = &l->source[l->cursor];
    for (int i = 0; table[i].utf8_seq != NULL; i++) {
        if (strncmp(start, table[i].utf8_seq, table[i].length) == 0) {
            l->cursor += (int)table[i].length;
            Token t = {table[i].type, start, table[i].length, 0};
            return t;
        }
    }
    Token err = {TOKEN_ERROR, start, 0, 0};
    return err;
}

Token get_next_token(Lexer* l) {
    lexer_skip_whitespace(l);

    const char* start = &l->source[l->cursor];
    if (start[0] == '\0') {
        Token t = {TOKEN_EOF, NULL, 0, 0};
        return t;
    }

    int is_variable = 0;
    if (start[0] == '?') {
        is_variable = 1;
        l->cursor++; // skip '?'
        start = &l->source[l->cursor];
    }

    if (isalpha((unsigned char)l->source[l->cursor]) || l->source[l->cursor] == '_') {
        const char* id_start = &l->source[l->cursor];
        int id_begin = l->cursor;

        while (l->source[l->cursor] != '\0' &&
              (isalnum((unsigned char)l->source[l->cursor]) || l->source[l->cursor] == '_')) {
            l->cursor++;
        }

        size_t len = (size_t)(l->cursor - id_begin);
        
        if (!is_variable) {
            TokenType kw = match_keyword(id_start, len);
            if (kw != TOKEN_ERROR) {
                Token t = {kw, id_start, len, 0};
                return t;
            }
        }

        Token t = {TOKEN_IDENT, id_start, len, is_variable};
        return t;
    }

    if (is_variable) {
        l->error_char = start[0];
        Token err = {TOKEN_ERROR, start, 0, 0};
        return err;
    }

    Token t = match_sequence(l, MULTIBYTE_TABLE);
    if (t.type != TOKEN_ERROR) return t;

    if ((unsigned char)start[0] >= 0x80) {
        t = match_sequence(l, UNICODE_TABLE);
        if (t.type != TOKEN_ERROR) return t;
    }

    l->cursor++;
    TokenType type = classify_single_char(start[0]);
    if (type == TOKEN_ERROR) {
        l->error_char = start[0];
    }
    Token single = {type, start, 1, 0};
    return single;
}

Token peek_token(Lexer* l) {
    int prev_cursor = l->cursor;
    Token t = get_next_token(l);
    l->cursor = prev_cursor;
    return t;
}