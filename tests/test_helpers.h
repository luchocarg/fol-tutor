#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "lexer.h"
#include "parser.h"
#include <assert.h>
#include <string.h>

static inline void assert_token(Token t, TokenType expected_type, size_t expected_len, const char* expected_text) {
    assert(t.type == expected_type);
    assert(t.length == expected_len);
    if (expected_text != NULL) {
        assert(strncmp(t.start, expected_text, expected_len) == 0);
    }
}

void term_to_sexpr(Term* t, char* buf);
void ast_to_sexpr(ASTNode* n, char* buf);
void assert_ast(ASTNode* node, const char* expected_sexpr);

#endif