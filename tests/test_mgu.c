#define _POSIX_C_SOURCE 200809L
#include "tests.h"
#include "core/cnf.h"
#include "mgu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static Literal* test_parse_literal(const char* input, SymbolTable* st) {
    Lexer l = { .source = input, .cursor = 0 };
    Parser p = { .l = &l, .st = st, .depth = 0 };
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
    for (int i = 0; i < lit->arity; i++) lit->args[i] = copy_term(atom->terms[i]);
    
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
    Literal* l1 = test_parse_literal("P(?x, ?y)", st);
    Literal* l2 = test_parse_literal("P(a, b)", st);
    
    char result[512];
    calculate_mgu_string(l1, l2, result, sizeof(result));
    
    assert(strstr(result, "x = a") != NULL);
    assert(strstr(result, "y = b") != NULL);
    
    free_test_literal(l1);
    free_test_literal(l2);
    free_symbol_table(st);
    printf("[OK] Test: MGU String Formatting\n");
}

void test_mgu_occurs_check(void) {
    SymbolTable* st = create_symbol_table();
    Literal* l1 = test_parse_literal("P(?x)", st);
    Literal* l2 = test_parse_literal("P(f(?x))", st);
    
    char result[512];
    calculate_mgu_string(l1, l2, result, sizeof(result));
    assert(strcmp(result, "Fail") == 0);
    
    free_test_literal(l1);
    free_test_literal(l2);
    free_symbol_table(st);
    printf("[OK] Test: MGU Occurs Check Failure\n");
}

void test_mgu_chained_composition(void) {
    SymbolTable* st = create_symbol_table();
    Literal* l1 = test_parse_literal("P(?x, ?y)", st);
    Literal* l2 = test_parse_literal("P(?y, a)", st);
    
    char result[512];
    calculate_mgu_string(l1, l2, result, sizeof(result));
    
    assert(strstr(result, "x = a") != NULL);
    assert(strstr(result, "y = a") != NULL);
    
    free_test_literal(l1);
    free_test_literal(l2);
    free_symbol_table(st);
    printf("[OK] Test: MGU Chained Substitution Composition\n");
}

void test_mgu_literal_propagation(void) {
    SymbolTable* st = create_symbol_table();
    
    Literal* l1 = test_parse_literal("P(?x)", st);
    Literal* l2 = test_parse_literal("P(f(?z))", st);
    
    bool success;
    Substitution* sigma = calculate_mgu(l1, l2, &success);
    assert(success == true);
    assert(sigma != NULL);
    
    Literal* l_target = test_parse_literal("Q(?x, ?w)", st);
    apply_substitution_to_literal(l_target, sigma);
    
    char term_buf1[128] = "";
    term_to_formula(l_target->args[0], term_buf1, sizeof(term_buf1));
    char term_buf2[128] = "";
    term_to_formula(l_target->args[1], term_buf2, sizeof(term_buf2));

    char buffer[512] = "";
    snprintf(buffer, sizeof(buffer), "%s(%s, %s)", l_target->predicate_name, term_buf1, term_buf2);
    
    assert(strcmp(buffer, "Q(f(?z), ?w)") == 0);
    
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
    c->capacity = 2; c->count = 2;
    c->id = 0; c->parent1 = NULL; c->parent2 = NULL;
    c->literals = malloc(2 * sizeof(Literal*));
    c->literals[0] = test_parse_literal("P(?x, ?y)", st);
    c->literals[1] = test_parse_literal("Q(?y)", st);
    
    Literal* l_mgu1 = test_parse_literal("R(?y)", st);
    Literal* l_mgu2 = test_parse_literal("R(c)", st);
    bool success;
    Substitution* sigma = calculate_mgu(l_mgu1, l_mgu2, &success);
    
    apply_substitution_to_clause(c, sigma);
    
    char term_buf[128] = "";
    term_to_formula(c->literals[0]->args[1], term_buf, sizeof(term_buf));
    assert(strcmp(term_buf, "c") == 0);
    
    term_buf[0] = '\0';
    term_to_formula(c->literals[1]->args[0], term_buf, sizeof(term_buf));
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

void test_simultaneous_mgu(void) {
    SymbolTable* st = create_symbol_table();
    Literal* lits1[2];
    lits1[0] = test_parse_literal("P(?x, a)", st);
    lits1[1] = test_parse_literal("P(b, ?y)", st);
    
    Literal* lits2[1];
    lits2[0] = test_parse_literal("¬P(b, a)", st);
    
    bool success;
    Substitution* sigma = calculate_simultaneous_mgu(lits1, 2, lits2, 1, &success);
    assert(success == true);
    
    char buffer[256] = "";
    calculate_simultaneous_mgu_string(sigma, buffer, sizeof(buffer));
    assert(strstr(buffer, "x = b") != NULL);
    assert(strstr(buffer, "y = a") != NULL);
    
    free_substitution(sigma);
    free_test_literal(lits1[0]);
    free_test_literal(lits1[1]);
    free_test_literal(lits2[0]);
    free_symbol_table(st);
    printf("[OK] Test: Simultaneous MGU\n");
}

void test_general_resolvent(void) {
    SymbolTable* st = create_symbol_table();
    Clause* c1 = create_empty_clause();
    free(c1->literals);
    c1->literals = malloc(2 * sizeof(Literal*));
    c1->literals[0] = test_parse_literal("P(?x)", st);
    c1->literals[1] = test_parse_literal("Q(?x)", st);
    c1->count = 2;
    
    Clause* c2 = create_empty_clause();
    free(c2->literals);
    c2->literals = malloc(2 * sizeof(Literal*));
    c2->literals[0] = test_parse_literal("¬P(a)", st);
    c2->literals[1] = test_parse_literal("R(b)", st);
    c2->count = 2;
    
    int m1[] = {0};
    int m2[] = {0};
    
    bool success;
    Substitution* sigma = calculate_mgu(c1->literals[0], c2->literals[0], &success);
    assert(success);
    
    Clause* res = create_general_resolvent(c1, m1, 1, c2, m2, 1, sigma);
    assert(res->count == 2);
    
    char buf[256] = "";
    clause_to_formula(res, buf, sizeof(buf));
    assert(strstr(buf, "Q(a)") != NULL);
    assert(strstr(buf, "R(b)") != NULL);
    
    free_clause(res);
    free_substitution(sigma);
    free_test_literal(c1->literals[0]);
    free_test_literal(c1->literals[1]);
    free(c1->literals); free(c1);
    free_test_literal(c2->literals[0]);
    free_test_literal(c2->literals[1]);
    free(c2->literals); free(c2);
    free_symbol_table(st);
    printf("[OK] Test: General Resolvent\n");
}

void test_standardize_apart(void) {
    SymbolTable* st = create_symbol_table();
    Clause* c1 = create_empty_clause();
    free(c1->literals);
    c1->literals = malloc(sizeof(Literal*));
    c1->literals[0] = test_parse_literal("P(?x)", st);
    c1->count = 1;

    Clause* c2 = create_empty_clause();
    free(c2->literals);
    c2->literals = malloc(sizeof(Literal*));
    c2->literals[0] = test_parse_literal("Q(?x)", st);
    c2->count = 1;

    standardize_apart_clause(c2, c1);
    char buf[256] = "";
    clause_to_formula(c2, buf, sizeof(buf));
    assert(strstr(buf, "?x_1") != NULL);

    free_test_literal(c1->literals[0]); free(c1->literals); free(c1);
    free_test_literal(c2->literals[0]); free(c2->literals); free(c2);
    free_symbol_table(st);
    printf("[OK] Test: Standardize Apart\n");
}

void test_parse_substitution(void) {
    Substitution* s = parse_substitution_string("?x=a; ?y=f(b)");
    assert(s != NULL);
    
    Term* tx = get_substitution(s, "?x");
    assert(tx != NULL);
    assert(strcmp(tx->name, "a") == 0);
    
    Term* ty = get_substitution(s, "?y");
    assert(ty != NULL);
    assert(strcmp(ty->name, "f") == 0);
    assert(ty->arity == 1);
    
    free_substitution(s);
    printf("[OK] Test: Substitution Parsing\n");
}

void test_factoring(void) {
    SymbolTable* st = create_symbol_table();
    Clause* c = create_empty_clause();
    free(c->literals);
    c->literals = malloc(3 * sizeof(Literal*));
    c->literals[0] = test_parse_literal("P(?x)", st);
    c->literals[1] = test_parse_literal("P(a)", st);
    c->literals[2] = test_parse_literal("Q(?y)", st);
    c->count = 3;
    
    Substitution* sigma = NULL;
    Clause* res = factor_clause(c, 0, 1, &sigma);
    assert(res != NULL);
    assert(res->count == 2);
    
    char buf[256] = "";
    clause_to_formula(res, buf, sizeof(buf));
    assert(strstr(buf, "P(a)") != NULL);
    assert(strstr(buf, "Q(") != NULL);
    
    free_substitution(sigma);
    free_clause(res);
    for(int i=0; i<3; i++) free_test_literal(c->literals[i]);
    free(c->literals); free(c);
    free_symbol_table(st);
    printf("[OK] Test: Clause Factoring\n");
}

void test_mgu_trace(void) {
    SymbolTable* st = create_symbol_table();
    Literal* l1 = test_parse_literal("P(?x, f(?x))", st);
    Literal* l2 = test_parse_literal("P(a, f(a))", st);
    
    char trace[4096] = "";
    calculate_mgu_trace(l1, l2, trace, sizeof(trace));
    
    assert(strlen(trace) > 0);
    assert(strstr(trace, "Assignment") != NULL);
    assert(strstr(trace, "Success") != NULL);
    
    free_test_literal(l1);
    free_test_literal(l2);
    free_symbol_table(st);
    printf("[OK] Test: MGU Tracing\n");
}

void run_mgu_tests(void) {
    printf("Running MGU Unification & Propagation Tests...\n");
    test_mgu_simple_string();
    test_mgu_occurs_check();
    test_mgu_chained_composition();
    test_mgu_literal_propagation();
    test_mgu_clause_propagation();
    test_simultaneous_mgu();
    test_general_resolvent();
    test_standardize_apart();
    test_parse_substitution();
    test_factoring();
    test_mgu_trace();
    printf("[OK] All MGU modules finished.\n");
}