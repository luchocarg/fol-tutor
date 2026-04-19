#include "tests.h"
#include <stdio.h>

void test_parse_atoms(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"P(x, y)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    assert_ast(n1, "(P x y)"); 
    free_ast(n1);

    Lexer l2 = {"⊥", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    assert_ast(n2, "⊥");
    free_ast(n2);
    
    free_symbol_table(st);
    printf("[OK] Test: Atoms and Falsum\n");
}

void test_parse_unary_and_dot(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"∀X.P(X)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    assert_ast(n1, "(∀ X (P X))");
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l2 = {"∀X.∃Y.¬P(X,Y)", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    assert_ast(n2, "(∀ X (∃ Y (¬ (P X Y))))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Quantifiers and Negation with Dot\n");
}

void test_parse_precedence_mixed(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"∀X.P(X) ∧ Q", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    assert_ast(n1, "(∧ (∀ X (P X)) Q)"); 
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l2 = {"∀X.(P(X) ∧ Q)", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    assert_ast(n2, "(∀ X (∧ (P X) Q))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Mixed Precedence and Scope\n");
}

void test_parse_complex_terms(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"P(f(x, g(y)), h(z))", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    assert_ast(n1, "(P (f x (g y)) (h z))");
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l2 = {"Q(a, f(b))", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    assert_ast(n2, "(Q a (f b))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Parsing complex terms\n");
}

void test_image_cases_standard(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"∀X.E(X, X)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    assert_ast(n1, "(∀ X (E X X))");
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l2 = {"∀X.P(X) => ∀X.Q(f(X))", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    assert_ast(n2, "(⇒ (∀ X (P X)) (∀ X (Q (f X))))");
    free_ast(n2);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l3 = {"∀X.¬P(X) ∧ ∃X.P(X)", 0};
    ASTNode* n3 = parse_formula(&l3, st);
    assert_ast(n3, "(∧ (∀ X (¬ (P X))) (∃ X (P X)))");
    free_ast(n3);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l4 = {"∀X.∃Y.P(X, Y) => ∃Y.∀X.P(X, Y)", 0};
    ASTNode* n4 = parse_formula(&l4, st);
    assert_ast(n4, "(⇒ (∀ X (∃ Y (P X Y))) (∃ Y (∀ X (P X Y))))");
    free_ast(n4);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l5 = {"∀X.(P(X) => S) => (∃X.P(X)) => S", 0};
    ASTNode* n5 = parse_formula(&l5, st);
    assert_ast(n5, "(⇒ (∀ X (⇒ (P X) S)) (⇒ (∃ X (P X)) S))");
    free_ast(n5);

    free_symbol_table(st);
    printf("[OK] Test: Standard Cases\n");
}

void test_arity_consistency(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"P(x) ∧ P(x, y)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    assert(n1 == NULL);
    
    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l2 = {"Q(f(x)) ∨ Q(f(x, y))", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    assert(n2 == NULL); 

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l3_pre = {"P(x)", 0};
    ASTNode* n3_pre = parse_formula(&l3_pre, st);
    assert(n3_pre != NULL);
    free_ast(n3_pre);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l3_fail = {"Q(P(x))", 0};
    ASTNode* n3_fail = parse_formula(&l3_fail, st);
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