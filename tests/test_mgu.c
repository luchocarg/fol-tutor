#define _POSIX_C_SOURCE 200809L
#include "tests.h"
#include "core/cnf.h"
#include "mgu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static Literal* test_parse_literal(const char* input, SymbolTable* st) {
    Lexer l = {input, 0};
    Parser p = {&l, st};
    ASTNode* node = parse_formula(&p);
    if (!node) return NULL;
    
    Literal* lit = malloc(sizeof(Literal));
    lit->is_negative = false;
    ASTNode* atom = node;
    
    if (node->type == NODE_UNARY) {
        lit->is_negative = true;
        atom = node->left;
    }
    
    lit->predicate_name = strdup(atom->name);
    lit->arity = atom->arity;
    lit->args = malloc(sizeof(Term*) * (size_t)lit->arity);
    for (int i = 0; i < lit->arity; i++) {
        lit->args[i] = copy_term(atom->terms[i]);
    }
    
    free_ast(node);
    return lit;
}

static void free_test_literal(Literal* l) {
    if (!l) return;
    free(l->predicate_name);
    for (int i = 0; i < l->arity; i++) free_term(l->args[i]);
    free(l->args);
    free(l);
}

void test_mgu_simple_string(void) {
    SymbolTable* st = create_symbol_table();
    Literal* l1 = test_parse_literal("P(X, Y)", st);
    Literal* l2 = test_parse_literal("P(a, b)", st);
    
    char result[512];
    calculate_mgu_string(l1, l2, result);
    
    assert(strstr(result, "X := a") != NULL);
    assert(strstr(result, "Y := b") != NULL);
    
    free_test_literal(l1);
    free_test_literal(l2);
    free_symbol_table(st);
    printf("[OK] Test: MGU String Formatting\n");
}

void test_mgu_occurs_check(void) {
    SymbolTable* st = create_symbol_table();
    Literal* l1 = test_parse_literal("P(X)", st);
    Literal* l2 = test_parse_literal("P(f(X))", st);
    
    char result[512];
    calculate_mgu_string(l1, l2, result);
    
    assert(strcmp(result, "Fail") == 0);
    
    free_test_literal(l1);
    free_test_literal(l2);
    free_symbol_table(st);
    printf("[OK] Test: MGU Occurs Check Failure\n");
}

void test_mgu_chained_composition(void) {
    SymbolTable* st = create_symbol_table();
    Literal* l1 = test_parse_literal("P(X, Y)", st);
    Literal* l2 = test_parse_literal("P(Y, a)", st);
    
    char result[512];
    calculate_mgu_string(l1, l2, result);
    
    assert(strstr(result, "X := a") != NULL);
    assert(strstr(result, "Y := a") != NULL);
    
    free_test_literal(l1);
    free_test_literal(l2);
    free_symbol_table(st);
    printf("[OK] Test: MGU Chained Substitution Composition\n");
}

void test_mgu_literal_propagation(void) {
    SymbolTable* st = create_symbol_table();
    
    Literal* l1 = test_parse_literal("P(X)", st);
    Literal* l2 = test_parse_literal("P(f(Z))", st);
    
    bool success;
    Substitution* sigma = calculate_mgu(l1, l2, &success);
    assert(success == true);
    assert(sigma != NULL);
    
    Literal* l_target = test_parse_literal("Q(X, W)", st);
    apply_substitution_to_literal(l_target, sigma);
    
    char buffer[256] = "";
    strcat(buffer, l_target->predicate_name);
    strcat(buffer, "(");
    
    char term_buf1[128] = "";
    term_to_formula(l_target->args[0], term_buf1);
    strcat(buffer, term_buf1);
    strcat(buffer, ", ");
    
    char term_buf2[128] = "";
    term_to_formula(l_target->args[1], term_buf2);
    strcat(buffer, term_buf2);
    strcat(buffer, ")");
    
    assert(strcmp(buffer, "Q(f(Z), W)") == 0);
    
    free_substitution(sigma);
    free_test_literal(l1);
    free_test_literal(l2);
    free_test_literal(l_target);
    free_symbol_table(st);
    printf("[OK] Test: MGU Structural Propagation on Literals\n");
}

void test_mgu_clause_propagation(void) {
    SymbolTable* st = create_symbol_table();
    
    Clause* c = malloc(sizeof(Clause));
    c->capacity = 2;
    c->count = 2;
    c->literals = malloc(2 * sizeof(Literal*));
    c->literals[0] = test_parse_literal("P(X, Y)", st);
    c->literals[1] = test_parse_literal("Q(Y)", st);
    
    Literal* l_mgu1 = test_parse_literal("R(Y)", st);
    Literal* l_mgu2 = test_parse_literal("R(c)", st);
    bool success;
    Substitution* sigma = calculate_mgu(l_mgu1, l_mgu2, &success);
    
    apply_substitution_to_clause(c, sigma);
    
    char term_buf[128] = "";
    term_to_formula(c->literals[0]->args[1], term_buf);
    assert(strcmp(term_buf, "c") == 0);
    
    term_buf[0] = '\0';
    term_to_formula(c->literals[1]->args[0], term_buf);
    assert(strcmp(term_buf, "c") == 0);
    
    free_substitution(sigma);
    free_test_literal(l_mgu1);
    free_test_literal(l_mgu2);
    free_test_literal(c->literals[0]);
    free_test_literal(c->literals[1]);
    free(c->literals);
    free(c);
    free_symbol_table(st);
    printf("[OK] Test: MGU Deep Propagation on Clauses\n");
}

void run_mgu_tests(void) {
    printf("Running MGU Unification & Propagation Tests...\n");
    test_mgu_simple_string();
    test_mgu_occurs_check();
    test_mgu_chained_composition();
    test_mgu_literal_propagation();
    test_mgu_clause_propagation();
    printf("[OK] All MGU modules finished.\n");
}