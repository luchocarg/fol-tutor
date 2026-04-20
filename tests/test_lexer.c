#include "tests.h"
#include <stdio.h>

void test_whitespace_and_control(void) {
    Lexer l = {"  \n\t  P", 0};
    Token t = get_next_token(&l);
    assert_token(t, TOKEN_ID_UPPER, 1, "P");
    printf("[OK] Test: Whitespace & Control characters\n");
}

void test_identifiers_and_capitalization(void) {
    Lexer l = {"x P Q123 y_z", 0};
    
    assert_token(get_next_token(&l), TOKEN_ID_LOWER, 1, "x");
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 1, "P");
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 4, "Q123");
    assert_token(get_next_token(&l), TOKEN_ID_LOWER, 3, "y_z");
    printf("[OK] Test: Identifiers Capitalization\n");
}

void test_logic_keywords(void) {
    Lexer l = {"forall exists and or not", 0};
    
    assert_token(get_next_token(&l), TOKEN_FORALL, 6, "forall");
    assert_token(get_next_token(&l), TOKEN_EXISTS, 6, "exists");
    assert_token(get_next_token(&l), TOKEN_AND, 3, "and");
    assert_token(get_next_token(&l), TOKEN_OR, 2, "or");
    assert_token(get_next_token(&l), TOKEN_NOT, 3, "not");
    printf("[OK] Test: Logic Keywords\n");
}

void test_unicode_symbols(void) {
    Lexer l = {"∀ ∧ ∨ ¬", 0};
    
    assert_token(get_next_token(&l), TOKEN_FORALL, 3, "∀");
    assert_token(get_next_token(&l), TOKEN_AND, 3, "∧");
    assert_token(get_next_token(&l), TOKEN_OR, 3, "∨");
    assert_token(get_next_token(&l), TOKEN_NOT, 2, "¬");
    printf("[OK] Test: UTF-8 Symbols\n");
}

void test_arrows_and_multibyte(void) {
    Lexer l = {"-> =>", 0};
    
    assert_token(get_next_token(&l), TOKEN_IMPLIES, 2, "->");
    assert_token(get_next_token(&l), TOKEN_IMPLIES, 2, "=>");
    printf("[OK] Test: Multibyte Arrows\n");
}

void test_structural_symbols(void) {
    Lexer l = {"( . , )", 0};
    
    assert_token(get_next_token(&l), TOKEN_LEFT_PARENT, 1, "(");
    assert_token(get_next_token(&l), TOKEN_DOT, 1, ".");
    assert_token(get_next_token(&l), TOKEN_COMMA, 1, ",");
    assert_token(get_next_token(&l), TOKEN_RIGHT_PARENT, 1, ")");
    printf("[OK] Test: Structural Symbols\n");
}

void test_eof_handling(void) {
    Lexer l = {"P", 0};
    get_next_token(&l);
    Token t = get_next_token(&l);
    assert(t.type == TOKEN_EOF);
    printf("[OK] Test: EOF Handling\n");
}

void test_keywords_vs_identifiers(void) {
    Lexer l = {"and android", 0};
    
    assert_token(get_next_token(&l), TOKEN_AND, 3, "and");
    assert_token(get_next_token(&l), TOKEN_ID_LOWER, 7, "android");
    
    printf("[OK] Test: Keywords vs Identifiers (Maximal Munch)\n");
}

void test_full_lexer_flow(void) {
    Lexer l = {"∀X_1. (P(x_1) => Q) -> ¬R", 0};

    assert_token(get_next_token(&l), TOKEN_FORALL, 3, "∀");
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 3, "X_1");
    assert_token(get_next_token(&l), TOKEN_DOT, 1, ".");
    assert_token(get_next_token(&l), TOKEN_LEFT_PARENT, 1, "(");
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 1, "P");
    assert_token(get_next_token(&l), TOKEN_LEFT_PARENT, 1, "(");
    assert_token(get_next_token(&l), TOKEN_ID_LOWER, 3, "x_1");
    assert_token(get_next_token(&l), TOKEN_RIGHT_PARENT, 1, ")");
    assert_token(get_next_token(&l), TOKEN_IMPLIES, 2, "=>");
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 1, "Q");
    assert_token(get_next_token(&l), TOKEN_RIGHT_PARENT, 1, ")");
    assert_token(get_next_token(&l), TOKEN_IMPLIES, 2, "->");
    assert_token(get_next_token(&l), TOKEN_NOT, 2, "¬");
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 1, "R");
    assert(get_next_token(&l).type == TOKEN_EOF);
    printf("[OK] Test: Complex Formula Flow\n");
}

void test_sticky_symbols(void) {
    Lexer l = {"P(x,y)∧Q", 0};
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 1, "P");
    assert_token(get_next_token(&l), TOKEN_LEFT_PARENT, 1, "(");
    assert_token(get_next_token(&l), TOKEN_ID_LOWER, 1, "x");
    assert_token(get_next_token(&l), TOKEN_COMMA, 1, ",");
    assert_token(get_next_token(&l), TOKEN_ID_LOWER, 1, "y");
    assert_token(get_next_token(&l), TOKEN_RIGHT_PARENT, 1, ")");
    assert_token(get_next_token(&l), TOKEN_AND, 3, "∧");
    assert_token(get_next_token(&l), TOKEN_ID_UPPER, 1, "Q");
    printf("[OK] Test: Sticky symbols\n");
}

void test_invalid_chars(void) {
    Lexer l = {"$ @", 0};
    assert_token(get_next_token(&l), TOKEN_ERROR, 1, "$");
    assert_token(get_next_token(&l), TOKEN_ERROR, 1, "@");
    printf("[OK] Test: Invalid characters handling\n");
}

void run_lexer_tests(void) {
    printf("Running Lexer Tests...\n");

    test_whitespace_and_control();
    test_identifiers_and_capitalization();
    test_logic_keywords();
    test_unicode_symbols();
    test_arrows_and_multibyte();
    test_structural_symbols();
    test_eof_handling();
    test_full_lexer_flow();
    test_keywords_vs_identifiers();
    test_sticky_symbols();
    test_invalid_chars();
    
    printf("[OK] Lexer module finished.\n");
}