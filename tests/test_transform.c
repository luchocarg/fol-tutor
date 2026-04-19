#include "tests.h"
#include "transform.h"
#include <stdio.h>
#include <string.h>

void test_transform_simple_implication(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"P => Q", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = remove_implications(n1);
    assert_ast(n1, "(∨ (¬ P) Q)");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: Simple Implication Transformation\n");
}

void test_transform_nested_implication(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"(P => Q) => R", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = remove_implications(n1);
    assert_ast(n1, "(∨ (¬ (∨ (¬ P) Q)) R)");
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();

    Lexer l2 = {"P => (Q => R)", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    n2 = remove_implications(n2);
    assert_ast(n2, "(∨ (¬ P) (∨ (¬ Q) R))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Nested Implication Transformation\n");
}

void test_transform_quantifier_implication(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"∀X.(P(X) => Q(X))", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = remove_implications(n1);
    assert_ast(n1, "(∀ X (∨ (¬ (P X)) (Q X)))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: Implication inside Quantifiers\n");
}

void test_transform_mixed_logic(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"(P ∧ Q) => (R ∨ S)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = remove_implications(n1);
    assert_ast(n1, "(∨ (¬ (∧ P Q)) (∨ R S))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: Mixed Operators with Implication\n");
}

void test_nnf_double_negation(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"¬¬P", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = apply_nnf(n1);
    assert_ast(n1, "P");
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();
    Lexer l2 = {"¬¬¬P", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    n2 = apply_nnf(n2);
    assert_ast(n2, "(¬ P)");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: NNF Double Negation\n");
}

void test_nnf_de_morgan(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"¬(P ∧ Q)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = apply_nnf(n1);
    assert_ast(n1, "(∨ (¬ P) (¬ Q))");
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();
    Lexer l2 = {"¬(P ∨ Q)", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    n2 = apply_nnf(n2);
    assert_ast(n2, "(∧ (¬ P) (¬ Q))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: NNF De Morgan Laws\n");
}

void test_nnf_quantifiers(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"¬∀X.P(X)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = apply_nnf(n1);
    assert_ast(n1, "(∃ X (¬ (P X)))");
    free_ast(n1);

    free_symbol_table(st);
    st = create_symbol_table();
    Lexer l2 = {"¬∃X.P(X)", 0};
    ASTNode* n2 = parse_formula(&l2, st);
    n2 = apply_nnf(n2);
    assert_ast(n2, "(∀ X (¬ (P X)))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: NNF Quantifier Negation\n");
}

void test_nnf_complex_recursive(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"¬(∀X.P(X) ∨ ∃Y.Q(Y))", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = apply_nnf(n1);
    assert_ast(n1, "(∧ (∃ X (¬ (P X))) (∀ Y (¬ (Q Y))))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: NNF Complex Recursive Push\n");
}

void test_alpha_rename_simple(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"∀X.P(X)", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    reset_alpha_counter();
    apply_alpha_renaming(n1, NULL);
    assert_ast(n1, "(∀ X_1 (P X_1))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: Alpha Renaming Simple Quantifier\n");
}

void test_alpha_rename_nested_scopes(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"∀X.(P(X) ∧ ∃X.Q(X))", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    reset_alpha_counter();
    apply_alpha_renaming(n1, NULL);
    assert_ast(n1, "(∀ X_1 (∧ (P X_1) (∃ X_2 (Q X_2))))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: Alpha Renaming Nested Scopes\n");
}

void test_alpha_rename_complex_terms(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"∀X.P(f(X, a))", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    reset_alpha_counter();
    apply_alpha_renaming(n1, NULL);
    assert_ast(n1, "(∀ X_1 (P (f X_1 a)))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: Alpha Renaming inside Complex Terms\n");
}

void test_pnf_left_extraction(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"(∀X_1.P(X_1)) ∧ Q", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = apply_pnf(n1);
    assert_ast(n1, "(∀ X_1 (∧ (P X_1) Q))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: PNF Left Extraction\n");
}

void test_pnf_right_extraction(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"P ∨ (∃Y_1.Q(Y_1))", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = apply_pnf(n1);
    assert_ast(n1, "(∃ Y_1 (∨ P (Q Y_1)))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: PNF Right Extraction\n");
}

void test_pnf_multiple_extraction(void) {
    SymbolTable* st = create_symbol_table();

    Lexer l1 = {"(∀X_1.P(X_1)) ∧ (∃X_2.Q(X_2))", 0};
    ASTNode* n1 = parse_formula(&l1, st);
    n1 = apply_pnf(n1);
    assert_ast(n1, "(∀ X_1 (∃ X_2 (∧ (P X_1) (Q X_2))))");
    free_ast(n1);

    free_symbol_table(st);
    printf("[OK] Test: PNF Multiple Extraction\n");
}

void run_transform_tests(void) {
    printf("Running Transform Tests...\n");

    test_transform_simple_implication();
    test_transform_nested_implication();
    test_transform_quantifier_implication();
    test_transform_mixed_logic();
    
    printf("[OK] Transform module finished.\n");
    printf("Running NNF Transform Tests...\n");

    test_nnf_double_negation();
    test_nnf_de_morgan();
    test_nnf_quantifiers();
    test_nnf_complex_recursive();
    
    printf("[OK] NNF module finished.\n");
    printf("Running Alpha Renaming Tests...\n");

    test_alpha_rename_simple();
    test_alpha_rename_nested_scopes();
    test_alpha_rename_complex_terms();

    printf("[OK] Alpha Renaming module finished.\n");
    printf("Running PNF Transform Tests...\n");

    test_pnf_left_extraction();
    test_pnf_right_extraction();
    test_pnf_multiple_extraction();

    printf("[OK] PNF module finished.\n");
}