#ifndef TESTS_H
#define TESTS_H

#include "core/lexer.h"
#include "core/parser.h"
#include "transform.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void run_lexer_tests(void);
void run_parser_tests(void);
void run_transform_tests(void);
void run_cnf_tests(void);
void run_mgu_tests(void);
void run_checkers_tests(void);
void run_resolution_tests(void);

static inline void assert_token(Token t, TokenType expected_type, size_t expected_len, const char* expected_text) {
    assert(t.type == expected_type);
    assert(t.length == expected_len);
    if (expected_text != NULL) {
        assert(strncmp(t.start, expected_text, expected_len) == 0);
    }
}

static inline void assert_ast(ASTNode* node, const char* expected_sexpr) {
    if (node == NULL) {
        exit(1);
    }
    char obtained[2048] = "";
    ast_to_sexpr(node, obtained);
    if (strcmp(obtained, expected_sexpr) != 0) {
        exit(1);
    }
}

#endif