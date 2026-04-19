#ifndef TESTS_H
#define TESTS_H

#include "lexer.h"
#include "parser.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void run_lexer_tests(void);
void run_parser_tests(void);
void run_transform_tests(void);

void test_nnf_double_negation(void);
void test_nnf_de_morgan(void);
void test_nnf_quantifiers(void);
void test_nnf_complex_recursive(void);

void test_pnf_left_extraction(void);
void test_pnf_right_extraction(void);
void test_pnf_multiple_extraction(void);

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
    obtained[0] = '\0'; 
    ast_to_sexpr(node, obtained);
        if (strcmp(obtained, expected_sexpr) != 0) {
        exit(1);
    }
}

#endif