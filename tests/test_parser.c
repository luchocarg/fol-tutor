#include "tests.h"
#include <stdio.h>

static ASTNode* test_parse(const char* input, SymbolTable* st) {
    Lexer l = {input, 0};
    Parser p = {&l, st};
    return parse_formula(&p);
}

void test_parse_atoms(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n1 = test_parse("P(x, y)", st);
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
    ASTNode* n1 = test_parse("∀X.P(X)", st);
    assert_ast(n1, "(∀ X (P X))");
    free_ast(n1);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("∀X.∃Y.¬P(X,Y)", st);
    assert_ast(n2, "(∀ X (∃ Y (¬ (P X Y))))");
    free_ast(n2);
    free_symbol_table(st);

    printf("[OK] Test: Quantifiers and Negation with Dot\n");
}

void test_parse_precedence_mixed(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("∀X.P(X) ∧ Q", st);
    assert_ast(n1, "(∧ (∀ X (P X)) Q)"); 
    free_ast(n1);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("∀X.(P(X) ∧ Q)", st);
    assert_ast(n2, "(∀ X (∧ (P X) Q))");
    free_ast(n2);
    free_symbol_table(st);

    printf("[OK] Test: Mixed Precedence and Scope\n");
}

void test_parse_complex_terms(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("P(f(x, g(y)), h(z))", st);
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
    ASTNode* n1 = test_parse("∀X.E(X, X)", st);
    assert_ast(n1, "(∀ X (E X X))");
    free_ast(n1);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("∀X.P(X) => ∀X.Q(f(X))", st);
    assert_ast(n2, "(⇒ (∀ X (P X)) (∀ X (Q (f X))))");
    free_ast(n2);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n3 = test_parse("∀X.¬P(X) ∧ ∃X.P(X)", st);
    assert_ast(n3, "(∧ (∀ X (¬ (P X))) (∃ X (P X)))");
    free_ast(n3);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n4 = test_parse("∀X.∃Y.P(X, Y) => ∃Y.∀X.P(X, Y)", st);
    assert_ast(n4, "(⇒ (∀ X (∃ Y (P X Y))) (∃ Y (∀ X (P X Y))))");
    free_ast(n4);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n5 = test_parse("∀X.(P(X) => S) => (∃X.P(X)) => S", st);
    assert_ast(n5, "(⇒ (∀ X (⇒ (P X) S)) (⇒ (∃ X (P X)) S))");
    free_ast(n5);
    free_symbol_table(st);

    printf("[OK] Test: Standard Cases\n");
}

void test_arity_consistency(void) {
    SymbolTable* st;

    st = create_symbol_table();
    ASTNode* n1 = test_parse("P(x) ∧ P(x, y)", st);
    assert(n1 == NULL);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n2 = test_parse("Q(f(x)) ∨ Q(f(x, y))", st);
    assert(n2 == NULL); 
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n3_pre = test_parse("P(x)", st);
    assert(n3_pre != NULL);
    free_ast(n3_pre);
    free_symbol_table(st);

    st = create_symbol_table();
    ASTNode* n3_pre2 = test_parse("P(x)", st); 
    free_ast(n3_pre2);
    ASTNode* n3_fail = test_parse("Q(P(x))", st);
    assert(n3_fail == NULL);
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