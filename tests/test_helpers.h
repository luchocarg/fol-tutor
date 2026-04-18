#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "lexer.h"

void assert_token(Token t, TokenType expected_type, size_t expected_len, const char* expected_text) {
    if (t.type != expected_type) {
        fprintf(stderr, "Type error: expected %d, obtained %d\n", expected_type, t.type);
    }
    assert(t.type == expected_type);
    assert(t.length == expected_len);
    if (expected_text != NULL) {
        assert(strncmp(t.start, expected_text, expected_len) == 0);
    }
}

#endif