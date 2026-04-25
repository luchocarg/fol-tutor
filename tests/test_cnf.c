#define _POSIX_C_SOURCE 200809L
#include "tests.h"
#include "core/cnf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static ASTNode* test_parse(const char* input, SymbolTable* st) {
    Lexer l = { .source = input, .cursor = 0 };
    Parser p = { .l = &l, .st = st, .depth = 0 };
    return parse_formula(&p);
}

void test_cnf_single_clause(void) {
    SymbolTable* st = create_symbol_table();
    
    ASTNode* ast = test_parse("P(?x_1, ?x_2) ∨ ¬Q(f_1(?x_3))", st);
    ClauseSet* set = ast_to_clause_set(ast);
    
    assert(set->count == 1);
    Clause* c0 = get_clause(set, 0);
    assert(c0 != NULL);
    assert(c0->count == 2);
    
    Literal* l0 = get_literal(c0, 0);
    assert(l0 != NULL);
    assert(strcmp(l0->predicate_name, "P") == 0);
    assert(l0->is_negative == false);
    assert(l0->arity == 2);
    assert(l0->args[0]->type == TERM_VARIABLE);   
    assert(strcmp(l0->args[0]->name, "?x_1") == 0);
    
    Literal* l1 = get_literal(c0, 1);
    assert(l1 != NULL);
    assert(strcmp(l1->predicate_name, "Q") == 0);
    assert(l1->is_negative == true);
    assert(l1->arity == 1);
    assert(l1->args[0]->type == TERM_FUNCTION);
    assert(strcmp(l1->args[0]->name, "f_1") == 0);
    assert(l1->args[0]->args[0]->type == TERM_VARIABLE);
    
    free_clause_set(set);
    free_ast(ast);
    free_symbol_table(st);
    printf("[OK] Test: CNF Single Clause Extraction\n");
}

void test_cnf_multiple_clauses(void) {
    SymbolTable* st = create_symbol_table();
    
    ASTNode* ast = test_parse("(P(?x_1) ∨ Q(?x_2)) ∧ ¬R(?x_3)", st);
    ClauseSet* set = ast_to_clause_set(ast);
    
    assert(set->count == 2);
    
    Clause* c0 = get_clause(set, 0);
    assert(c0->count == 2);
    assert(strcmp(get_literal(c0, 0)->predicate_name, "P") == 0);
    assert(strcmp(get_literal(c0, 1)->predicate_name, "Q") == 0);
    
    Clause* c1 = get_clause(set, 1);
    assert(c1->count == 1);
    Literal* l_r = get_literal(c1, 0);
    assert(strcmp(l_r->predicate_name, "R") == 0);
    assert(l_r->is_negative == true);
    assert(l_r->arity == 1);
    
    free_clause_set(set);
    free_ast(ast);
    free_symbol_table(st);
    printf("[OK] Test: CNF Multiple Clauses Separation\n");
}

void test_cnf_getters_out_of_bounds(void) {
    SymbolTable* st = create_symbol_table();
    
    ASTNode* ast = test_parse("P(?x_1)", st);
    ClauseSet* set = ast_to_clause_set(ast);
    
    assert(get_clause(set, 1) == NULL);
    assert(get_clause(set, -1) == NULL);
    
    Clause* c0 = get_clause(set, 0);
    assert(get_literal(c0, 1) == NULL);
    assert(get_literal(c0, -1) == NULL);
    
    free_clause_set(set);
    free_ast(ast);
    free_symbol_table(st);
    printf("[OK] Test: CNF Getters Out of Bounds Protection\n");
}

void test_cnf_deep_copy_isolation(void) {
    SymbolTable* st = create_symbol_table();
    
    ASTNode* ast = test_parse("P(?x_1) ∧ P(?x_1)", st);
    ClauseSet* set = ast_to_clause_set(ast);
    
    Literal* l0 = get_literal(get_clause(set, 0), 0);
    Literal* l1 = get_literal(get_clause(set, 1), 0);
    
    free(l0->args[0]->name);
    l0->args[0]->name = strdup("?x_MODIFIED");
    
    assert(strcmp(l1->args[0]->name, "?x_1") == 0);
    
    free_clause_set(set);
    free_ast(ast);
    free_symbol_table(st);
    printf("[OK] Test: CNF Deep Copy Memory Isolation\n");
}

void run_cnf_tests(void) {
    printf("Running CNF Flat Structure Tests...\n");

    test_cnf_single_clause();
    test_cnf_multiple_clauses();
    test_cnf_getters_out_of_bounds();
    test_cnf_deep_copy_isolation();

    printf("[OK] All CNF modules finished.\n");
}