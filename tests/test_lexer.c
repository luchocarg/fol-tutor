#include "tests.h"
#include <stdio.h>

void test_whitespace_and_control(void) {
    Lexer l = { .source = "  \n\t  P", .cursor = 0 };
    Token t = get_next_token(&l);
    assert(t.type == TOKEN_IDENT);
    assert(t.is_variable == 0);
    printf("[OK] Test: Whitespace & Control characters\n");
}

void test_identifiers_and_capitalization(void) {
    Lexer l = { .source = "?x P ?Q123 y_z", .cursor = 0 };
    
    Token t1 = get_next_token(&l);
    assert(t1.type == TOKEN_IDENT); assert(t1.is_variable == 1);
    assert(strncmp(t1.start, "x", 1) == 0);

    Token t2 = get_next_token(&l);
    assert(t2.type == TOKEN_IDENT); assert(t2.is_variable == 0);

    Token t3 = get_next_token(&l);
    assert(t3.type == TOKEN_IDENT); assert(t3.is_variable == 1);

    Token t4 = get_next_token(&l);
    assert(t4.type == TOKEN_IDENT); assert(t4.is_variable == 0);

    printf("[OK] Test: Identifiers - '?' prefix for variables\n");
}

void test_logic_keywords(void) {
    Lexer l = { .source = "forall exists and or not", .cursor = 0 };
    
    assert(get_next_token(&l).type == TOKEN_FORALL);
    assert(get_next_token(&l).type == TOKEN_EXISTS);
    assert(get_next_token(&l).type == TOKEN_AND);
    assert(get_next_token(&l).type == TOKEN_OR);
    assert(get_next_token(&l).type == TOKEN_NOT);
    printf("[OK] Test: Logic Keywords\n");
}

void test_unicode_symbols(void) {
    Lexer l = { .source = "∀ ∧ ∨ ¬", .cursor = 0 };
    
    assert(get_next_token(&l).type == TOKEN_FORALL);
    assert(get_next_token(&l).type == TOKEN_AND);
    assert(get_next_token(&l).type == TOKEN_OR);
    assert(get_next_token(&l).type == TOKEN_NOT);
    printf("[OK] Test: UTF-8 Symbols\n");
}

void test_arrows_and_multibyte(void) {
    Lexer l = { .source = "-> =>", .cursor = 0 };
    
    assert(get_next_token(&l).type == TOKEN_IMPLIES);
    assert(get_next_token(&l).type == TOKEN_IMPLIES);
    printf("[OK] Test: Multibyte Arrows\n");
}

void test_structural_symbols(void) {
    Lexer l = { .source = "( . , )", .cursor = 0 };
    
    assert(get_next_token(&l).type == TOKEN_LEFT_PARENT);
    assert(get_next_token(&l).type == TOKEN_DOT);
    assert(get_next_token(&l).type == TOKEN_COMMA);
    assert(get_next_token(&l).type == TOKEN_RIGHT_PARENT);
    printf("[OK] Test: Structural Symbols\n");
}

void test_eof_handling(void) {
    Lexer l = {.source = "P", .cursor = 0};
    get_next_token(&l);
    Token t = get_next_token(&l);
    assert(t.type == TOKEN_EOF);
    printf("[OK] Test: EOF Handling\n");
}

void test_keywords_vs_identifiers(void) {
    Lexer l = { .source = "and android", .cursor = 0 };
    
    assert(get_next_token(&l).type == TOKEN_AND);
    Token t2 = get_next_token(&l);
    assert(t2.type == TOKEN_IDENT); assert(t2.is_variable == 0);
    
    printf("[OK] Test: Keywords vs Identifiers (Maximal Munch)\n");
}

void test_full_lexer_flow(void) {
    Lexer l = { .source = "∀?x_1. (P(?x_1) => Q) -> ¬R", .cursor = 0 };

    assert(get_next_token(&l).type == TOKEN_FORALL);
    Token var = get_next_token(&l);
    assert(var.type == TOKEN_IDENT); assert(var.is_variable == 1);
    assert(get_next_token(&l).type == TOKEN_DOT);
    assert(get_next_token(&l).type == TOKEN_LEFT_PARENT);
    Token pred = get_next_token(&l);
    assert(pred.type == TOKEN_IDENT); assert(pred.is_variable == 0);
    assert(get_next_token(&l).type == TOKEN_LEFT_PARENT);
    Token arg = get_next_token(&l);
    assert(arg.type == TOKEN_IDENT); assert(arg.is_variable == 1);
    assert(get_next_token(&l).type == TOKEN_RIGHT_PARENT);
    assert(get_next_token(&l).type == TOKEN_IMPLIES);
    Token q = get_next_token(&l);
    assert(q.type == TOKEN_IDENT); assert(q.is_variable == 0);
    assert(get_next_token(&l).type == TOKEN_RIGHT_PARENT);
    assert(get_next_token(&l).type == TOKEN_IMPLIES);
    assert(get_next_token(&l).type == TOKEN_NOT);
    Token r = get_next_token(&l);
    assert(r.type == TOKEN_IDENT); assert(r.is_variable == 0);
    assert(get_next_token(&l).type == TOKEN_EOF);
    printf("[OK] Test: Complex Formula Flow\n");
}

void test_sticky_symbols(void) {
    Lexer l = { .source = "P(?x,?y)∧Q", .cursor = 0 };
    Token p = get_next_token(&l); assert(p.type == TOKEN_IDENT); assert(p.is_variable == 0);
    assert(get_next_token(&l).type == TOKEN_LEFT_PARENT);
    Token x = get_next_token(&l); assert(x.type == TOKEN_IDENT); assert(x.is_variable == 1);
    assert(get_next_token(&l).type == TOKEN_COMMA);
    Token y = get_next_token(&l); assert(y.type == TOKEN_IDENT); assert(y.is_variable == 1);
    assert(get_next_token(&l).type == TOKEN_RIGHT_PARENT);
    assert(get_next_token(&l).type == TOKEN_AND);
    Token q = get_next_token(&l); assert(q.type == TOKEN_IDENT); assert(q.is_variable == 0);
    printf("[OK] Test: Sticky symbols\n");
}

void test_invalid_chars(void) {
    Lexer l = { .source = "$ @", .cursor = 0 };
    assert(get_next_token(&l).type == TOKEN_ERROR);
    assert(get_next_token(&l).type == TOKEN_ERROR);
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