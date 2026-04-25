#define _POSIX_C_SOURCE 200809L
#include "tests.h"
#include "core/checkers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ASTNode* test_parse(const char* input, SymbolTable* st) {
    Lexer l = { .source = input, .cursor = 0 };
    Parser p = { .l = &l, .st = st, .depth = 0 };
    return parse_formula(&p);
}

void test_checker_nnf(void) {
    SymbolTable* st = create_symbol_table();
    
    ASTNode* n1 = test_parse("¬(P ∧ Q)", st);
    assert(is_formula_nnf(n1) == false);
    free_ast(n1);

    ASTNode* n2 = test_parse("(¬P) ∨ (¬Q)", st);
    assert(is_formula_nnf(n2) == true);
    free_ast(n2);

    ASTNode* n3 = test_parse("¬¬P", st);
    assert(is_formula_nnf(n3) == false);
    free_ast(n3);

    free_symbol_table(st);
    printf("[OK] Test: NNF Checker\n");
}

void test_checker_rectified(void) {
    SymbolTable* st = create_symbol_table();

    // not rectified
    ASTNode* n1 = test_parse("∀?x.P(?x) ∧ ∃?x.Q(?x)", st);
    assert(n1 != NULL);
    assert(is_formula_rectified(n1) == false);
    free_ast(n1);

    // rectified
    ASTNode* n2 = test_parse("∀?x.P(?x) ∧ ∃?y.Q(?y)", st);
    assert(n2 != NULL);
    assert(is_formula_rectified(n2) == true);
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Rectified Checker\n");
}

void test_checker_pnf(void) {
    SymbolTable* st = create_symbol_table();

    ASTNode* n1 = test_parse("∀?x.∃?y.(P(?x) ∨ Q(?y))", st);
    assert(n1 != NULL);
    assert(is_formula_pnf(n1) == true);
    free_ast(n1);

    ASTNode* n2 = test_parse("(∀?x.P(?x)) ∨ (∃?y.Q(?y))", st);
    assert(n2 != NULL);
    assert(is_formula_pnf(n2) == false);
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: PNF Checker\n");
}

void test_checker_skolemized(void) {
    SymbolTable* st = create_symbol_table();

    // skolem
    ASTNode* n1 = test_parse("∀?x.P(?x, f(?x))", st);
    assert(n1 != NULL);
    assert(is_formula_skolemized(n1) == true);
    free_ast(n1);

    // not skolem
    ASTNode* n2 = test_parse("∀?x.∃?y.P(?x, ?y)", st);
    assert(n2 != NULL);
    assert(is_formula_skolemized(n2) == false);
    free_ast(n2);

    free_symbol_table(st);
    printf("[OK] Test: Skolemized Checker\n");
}

void run_checkers_tests(void) {
    printf("Running Property Checker Tests...\n");
    test_checker_nnf();
    test_checker_rectified();
    test_checker_pnf();
    test_checker_skolemized();
    printf("[OK] All Checker modules finished.\n");
}
