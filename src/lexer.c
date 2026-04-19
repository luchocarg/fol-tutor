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

static const KeywordMap COMMAND_TABLE[] = {
    {"forall", TOKEN_FORALL, 6},
    {"exists", TOKEN_EXISTS, 6},
    {"and",    TOKEN_AND,    3},
    {"or",     TOKEN_OR,     2},
    {"not",    TOKEN_NOT,    3},
    {NULL,     TOKEN_ERROR,  0}
};

static const UTF8Map UTF8_TABLE[] = {
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

static TokenType classify_logic_symbol(const char* symbol_start_ptr, size_t symbol_length) {
    for (int i = 0; COMMAND_TABLE[i].keyword != NULL; i++) {
        if (symbol_length == COMMAND_TABLE[i].length) {
            if (strncmp(symbol_start_ptr, COMMAND_TABLE[i].keyword, symbol_length) == 0) {
                return COMMAND_TABLE[i].type;
            }
        }
    }
	
	return TOKEN_ERROR;
}
void skip_whitespace_and_control(Lexer* l) {
	while (l->source[l->cursor] != '\0') {
		unsigned char c = (unsigned char)l->source[l->cursor];

        if (isspace(c)) {
            l->cursor++;
        } else {
        	return;
        }
    }
}

Token lex_identifier(Lexer* l) {
    const char* start_ptr = &l->source[l->cursor];
    int start_pos = l->cursor;

    while (l->source[l->cursor] != '\0' && 
          (isalnum((unsigned char)l->source[l->cursor]) || l->source[l->cursor] == '_')) {
        l->cursor++;
    }

    int length = (int)l->cursor - start_pos;

    TokenType type = classify_logic_symbol(start_ptr, (size_t)length);

    if (type == TOKEN_ERROR) {
        if (isupper((unsigned char)start_ptr[0])) {
            type = TOKEN_UPPER_INDENT;
        } else {
            type = TOKEN_LOWER_INDENT;
        }
    }

    return (Token){type, start_ptr, (size_t)length};
}

Token get_next_token(Lexer* l) {
	skip_whitespace_and_control(l);

	if (l->source[l->cursor] == '\0') {
        return (Token){TOKEN_EOF, NULL, 0};
    }

    const char* start_ptr = &l->source[l->cursor];
    unsigned char c = (unsigned char)l->source[l->cursor];

	if (isalpha(c)) {
	    return lex_identifier(l);
	}

    if (c >= 0x80) {
    	for (int i = 0; UTF8_TABLE[i].utf8_seq != NULL; i++) {
    		if (strncmp(start_ptr, UTF8_TABLE[i].utf8_seq, UTF8_TABLE[i].length) == 0) {
    			l->cursor += (int)UTF8_TABLE[i].length;
    			return (Token){UTF8_TABLE[i].type, start_ptr, UTF8_TABLE[i].length};
    		}
    	}

    	l->cursor++;
        return (Token){TOKEN_ERROR, start_ptr, 1};
    }

	if (c == '-' || c == '=') {
        for (int i = 0; MULTIBYTE_TABLE[i].utf8_seq != NULL; i++) {
            if (strncmp(start_ptr, MULTIBYTE_TABLE[i].utf8_seq, MULTIBYTE_TABLE[i].length) == 0) {
                l->cursor += (int)MULTIBYTE_TABLE[i].length;
                return (Token){MULTIBYTE_TABLE[i].type, start_ptr, MULTIBYTE_TABLE[i].length};
            }
        }
    }

    char current = l->source[l->cursor++];
    switch (current) {
        case '(': return (Token){TOKEN_LEFT_PARENT,  start_ptr, 1};
        case ')': return (Token){TOKEN_RIGHT_PARENT, start_ptr, 1};
        case '.': return (Token){TOKEN_DOT,          start_ptr, 1};
        case ',': return (Token){TOKEN_COMMA,        start_ptr, 1};
        default:  return (Token){TOKEN_ERROR,        start_ptr, 1};
}

    return (Token){TOKEN_ERROR, start_ptr, 1};
}

Token peek_token(Lexer* l) {
    int temp_cursor = l->cursor;

    Token t = get_next_token(l);    
    
    l->cursor = temp_cursor;
    return t;
}