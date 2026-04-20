#include "tests.h"
#include <stdio.h>

static ASTNode* test_parse(const char* input, SymbolTable* st) {
    Lexer l = {input, 0};
    Parser p = {&l, st};
    return parse_formula(&p);
}

void test_transform_simple_implication(void) {
    SymbolTable* st = create_symbol_table();
    ASTNode* n = test_parse("P => Q", st);
    
    n = transform_remove_implications(n);
    assert_ast(n, "(∨ (¬ P) Q)");
    
    free_ast(n);
    free_symbol_table(st);
    printf("[OK] Test: Simple Implication Transformation\n");
}

void test_transform_nested_implication(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n1 = test_parse("(P => Q) => R", st);
    n1 = transform_remove_implications(n1);
    assert_ast(n1, "(∨ (¬ (∨ (¬ P) Q)) R)");
    free_ast(n1);

    ASTNode* n2 = test_parse("P => (Q => R)", st);
    n2 = transform_remove_implications(n2);
    assert_ast(n2, "(∨ (¬ P) (∨ (¬ Q) R))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Nested Implication Transformation\n");
}

void test_nnf_de_morgan(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n1 = test_parse("¬(P ∧ Q)", st);
    n1 = transform_to_nnf(n1);
    assert_ast(n1, "(∨ (¬ P) (¬ Q))");
    free_ast(n1);

    ASTNode* n2 = test_parse("¬(P ∨ Q)", st);
    n2 = transform_to_nnf(n2);
    assert_ast(n2, "(∧ (¬ P) (¬ Q))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: NNF De Morgan Laws\n");
}

void test_nnf_quantifiers(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n1 = test_parse("¬∀X.P(X)", st);
    n1 = transform_to_nnf(n1);
    assert_ast(n1, "(∃ X (¬ (P X)))");
    free_ast(n1);

    ASTNode* n2 = test_parse("¬∃X.P(X)", st);
    n2 = transform_to_nnf(n2);
    assert_ast(n2, "(∀ X (¬ (P X)))");
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: NNF Quantifier Negation\n");
}

void test_alpha_rename_nested_scopes(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n = test_parse("∀X.(P(X) ∧ ∃X.Q(X))", st);
    transform_reset_alpha_counter();
    transform_alpha_rename(n, NULL);
    
    assert_ast(n, "(∀ X_1 (∧ (P X_1) (∃ X_2 (Q X_2))))");
    
    free_ast(n);
    free_symbol_table(st);
    printf("[OK] Test: Alpha Renaming Nested Scopes\n");
}

void test_pnf_multiple_extraction(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n = test_parse("(∀X_1.P(X_1)) ∧ (∃X_2.Q(X_2))", st);
    n = transform_to_pnf(n);
    assert_ast(n, "(∀ X_1 (∃ X_2 (∧ (P X_1) (Q X_2))))");
    
    free_ast(n);
    free_symbol_table(st);
    printf("[OK] Test: PNF Multiple Extraction\n");
}

void test_skolem_basic(void) {
    SymbolTable* st = create_symbol_table();
    
    ASTNode* n1 = test_parse("∃X.P(X)", st);
    n1 = transform_skolemize(n1);
    assert_ast(n1, "(P f_1)"); 
    free_ast(n1);

    ASTNode* n2 = test_parse("∀X. ∃Y. Q(X, Y)", st);
    n2 = transform_skolemize(n2);
    assert_ast(n2, "(∀ X (Q X (f_1 X)))"); 
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Skolemization\n");
}

void test_cnf_distribution(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n = test_parse("P ∨ (Q ∧ R)", st);
    n = transform_distribute(n);
    
    assert_ast(n, "(∧ (∨ P Q) (∨ P R))");
    
    free_ast(n);
    free_symbol_table(st);
    printf("[OK] Test: CNF Distribution\n");
}

void test_push_universals(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n = test_parse("∀X.(P(X) ∧ Q(X))", st);
    n = transform_push_universals(n);
    
    assert_ast(n, "(∧ (∀ X (P X)) (∀ X (Q X)))");
    
    free_ast(n);
    free_symbol_table(st);
    printf("[OK] Test: Push Universals\n");
}

void test_cnf_to_sets(void) {
    SymbolTable* st = create_symbol_table();
    char buffer[1024];

    ASTNode* n1 = test_parse("P ∧ Q", st);
    buffer[0] = '\0';
    ast_to_cnf_sets(n1, buffer);
    assert(strcmp(buffer, "{{P}, {Q}}") == 0);
    free_ast(n1);

    ASTNode* n2 = test_parse("P ∨ Q", st);
    buffer[0] = '\0';
    ast_to_cnf_sets(n2, buffer);
    assert(strcmp(buffer, "{{P, Q}}") == 0);
    free_ast(n2);

    ASTNode* n3 = test_parse("P ∧ (F ∨ K) ∧ (F ∨ M)", st);
    buffer[0] = '\0';
    ast_to_cnf_sets(n3, buffer);
    assert(strcmp(buffer, "{{P}, {F, K}, {F, M}}") == 0);
    free_ast(n3);

    free_symbol_table(st);
    printf("[OK] Test: CNF Set Notation Output\n");
}

void run_transform_tests(void) {
    printf("Running Transformation Tests...\n");

    test_transform_simple_implication();
    test_transform_nested_implication();
    
    test_nnf_de_morgan();
    test_nnf_quantifiers();

    test_alpha_rename_nested_scopes();

    test_pnf_multiple_extraction();

    test_skolem_basic();

    test_cnf_distribution();

    test_push_universals();

    test_cnf_to_sets();

    printf("[OK] All Transformation modules finished.\n");
}