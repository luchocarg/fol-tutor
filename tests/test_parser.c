#include "tests.h"
#include <stdio.h>

static ASTNode* test_parse(const char* input, SymbolTable* st) {
    Lexer l = { .source = input, .cursor = 0 };
    Parser p = { .l = &l, .st = st, .depth = 0 };
    return parse_formula(&p);
}

void test_parse_atoms(void) {
    SymbolTable* st = create_symbol_table();
    ASTNode* n1 = test_parse("P(?x, ?y)", st);
    assert_ast(n1, "(P x y)"); 
    free_ast(n1);

    ASTNode* n2 = test_parse("⊥", st);
    assert_ast(n2, "⊥");
    free_ast(n2);
    
    free_symbol_table(st);
    printf("[OK] Test: Atoms and Falsum\n");
}

void test_parse_unary_and_dot(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("∀?x.P(?x)", st);
    assert_ast(n1, "(∀ x (P x))");
    free_ast(n1);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("∀?x.∃?y.¬P(?x,?y)", st);
    assert_ast(n2, "(∀ x (∃ y (¬ (P x y))))");
    free_ast(n2);
    free_symbol_table(st);

    printf("[OK] Test: Quantifiers and Negation with Dot\n");
}

void test_parse_precedence_mixed(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("∀?x.P(?x) ∧ Q", st);
    assert_ast(n1, "(∧ (∀ x (P x)) Q)");
    free_ast(n1);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("∀?x.(P(?x) ∧ Q)", st);
    assert_ast(n2, "(∀ x (∧ (P x) Q))");
    free_ast(n2);
    free_symbol_table(st);

    printf("[OK] Test: Mixed Precedence and Scope\n");
}

void test_parse_complex_terms(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("P(f(?x, g(?y)), h(?z))", st);
    assert_ast(n1, "(P (f x (g y)) (h z))");
    free_ast(n1);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("Q(a, f(b))", st);
    assert_ast(n2, "(Q a (f b))");
    free_ast(n2);
    free_symbol_table(st);

    printf("[OK] Test: Parsing complex terms\n");
}

void test_image_cases_standard(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("∀?x.E(?x, ?x)", st);
    assert_ast(n1, "(∀ x (E x x))");
    free_ast(n1);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("∀?x.P(?x) => ∀?x.Q(f(?x))", st);
    assert_ast(n2, "(⇒ (∀ x (P x)) (∀ x (Q (f x))))");
    free_ast(n2);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n3 = test_parse("∀?x.¬P(?x) ∧ ∃?x.P(?x)", st);
    assert_ast(n3, "(∧ (∀ x (¬ (P x))) (∃ x (P x)))");
    free_ast(n3);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n4 = test_parse("∀?x.∃?y.P(?x, ?y) => ∃?y.∀?x.P(?x, ?y)", st);
    assert_ast(n4, "(⇒ (∀ x (∃ y (P x y))) (∃ y (∀ x (P x y))))");
    free_ast(n4);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n5 = test_parse("∀?x.(P(?x) => S) => (∃?x.P(?x)) => S", st);
    assert_ast(n5, "(⇒ (∀ x (⇒ (P x) S)) (⇒ (∃ x (P x)) S))");
    free_ast(n5);
    free_symbol_table(st);

    printf("[OK] Test: Standard Cases\n");
}

void test_arity_consistency(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("P(?x) ∧ P(?x, ?y)", st);
    assert(n1 == NULL);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("Q(f(?x)) ∨ Q(f(?x, ?y))", st);
    assert(n2 == NULL); 
    free_symbol_table(st);

    printf("[OK] Test: Arity and Symbol Type Consistency\n");
}

void run_parser_tests(void) {
    printf("Running Parser Tests...\n");

    test_parse_atoms();
    test_parse_unary_and_dot();
    test_parse_precedence_mixed();
    test_parse_complex_terms();
    test_arity_consistency();
    test_image_cases_standard();
    
    printf("[OK] Parser module finished.\n");
}