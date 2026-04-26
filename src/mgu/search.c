#define _POSIX_C_SOURCE 200809L
#include "core/cnf.h"
#include "mgu.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static ClauseNode* append_node(ClauseNode** head, ClauseNode** tail,
                                Clause* c, char* mgu_str) {
    ClauseNode* n = malloc(sizeof(ClauseNode));
    if (c->id == -1) c->id = get_next_clause_id();
    n->c       = c;
    n->mgu_str = mgu_str;
    n->next    = NULL;
    if (!*head) *head = n;
    else        (*tail)->next = n;
    *tail = n;
    return n;
}

static bool are_literals_identical(Literal* l1, Literal* l2) {
    if (l1->is_negative != l2->is_negative) return false;
    if (strcmp(l1->predicate_name, l2->predicate_name) != 0) return false;
    if (l1->arity != l2->arity) return false;
    for (int i = 0; i < l1->arity; i++) {
        char b1[256] = "", b2[256] = "";
        term_to_formula(l1->args[i], b1);
        term_to_formula(l2->args[i], b2);
        if (strcmp(b1, b2) != 0) return false;
    }
    return true;
}

static bool is_duplicate(Clause* c, ClauseNode* head) {
    while (head) {
        Clause* h = head->c;
        if (h->count == c->count) {
            bool all_match = true;
            for (int i = 0; i < c->count && all_match; i++) {
                bool found = false;
                for (int j = 0; j < h->count; j++) {
                    if (are_literals_identical(c->literals[i], h->literals[j])) {
                        found = true; break;
                    }
                }
                if (!found) all_match = false;
            }
            if (all_match) return true;
        }
        head = head->next;
    }
    return false;
}

static bool is_tautology(Clause* c) {
    for (int i = 0; i < c->count; i++) {
        for (int j = 0; j < c->count; j++) {
            if (i == j) continue;
            Literal* a = c->literals[i];
            Literal* b = c->literals[j];
            if (a->is_negative == b->is_negative) continue;
            if (strcmp(a->predicate_name, b->predicate_name) != 0) continue;
            if (a->arity != b->arity) continue;
            bool same_args = true;
            for (int k = 0; k < a->arity && same_args; k++) {
                char ba[256] = "", bb[256] = "";
                term_to_formula(a->args[k], ba);
                term_to_formula(b->args[k], bb);
                if (strcmp(ba, bb) != 0) same_args = false;
            }
            if (same_args) return true;
        }
    }
    return false;
}

Clause* run_automated_resolution(ClauseSet* initial_set,
                                  ClauseNode** out_all_clauses,
                                  int max_steps) {
    ClauseNode* head = NULL;
    ClauseNode* tail = NULL;

    for (int i = 0; i < initial_set->count; i++) {
        Clause* c = initial_set->clauses[i];
        append_node(&head, &tail, c, NULL);
        
        for (int f1 = 0; f1 < c->count; f1++) {
            for (int f2 = f1 + 1; f2 < c->count; f2++) {
                Substitution* fsigma = NULL;
                Clause* factored = factor_clause(c, f1, f2, &fsigma);
                if (factored) {
                    if (!is_tautology(factored) && !is_duplicate(factored, head)) {
                        char* fmgu = malloc(1024);
                        sprintf(fmgu, "Initial factoring of %d,%d", f1, f2);
                        append_node(&head, &tail, factored, fmgu);
                    } else {
                        free_clause(factored);
                    }
                    if (fsigma) free_substitution(fsigma);
                }
            }
        }
    }

    ClauseNode* current1 = head;
    int steps = 0;
    Clause* empty = NULL;

    while (current1 && steps < max_steps) {
        ClauseNode* current2 = head;
        while (current2 && steps < max_steps) {
            if (current1 == current2) { current2 = current2->next; continue; }

            Clause* c1 = current1->c;
            Clause* c2 = current2->c;

            for (int i = 0; i < c1->count && steps < max_steps; i++) {
                for (int j = 0; j < c2->count && steps < max_steps; j++) {
                    Literal* l1 = c1->literals[i];
                    Literal* l2 = c2->literals[j];

                    if (l1->is_negative == l2->is_negative) continue;
                    if (strcmp(l1->predicate_name, l2->predicate_name) != 0) continue;

                    Clause* c2_copy = create_empty_clause();
                    c2_copy->capacity = c2->count;
                    free(c2_copy->literals);
                    c2_copy->literals = malloc((size_t)c2_copy->capacity * sizeof(Literal*));
                    for (int k = 0; k < c2->count; k++)
                        c2_copy->literals[c2_copy->count++] = copy_literal(c2->literals[k]);
                    standardize_apart_clause(c2_copy, c1);

                    bool ok;
                    Substitution* sigma = calculate_mgu(c1->literals[i],
                                                        c2_copy->literals[j],
                                                        &ok);
                    if (ok) {
                        Clause* res = create_resolvent(c1, i, c2_copy, j, sigma);

                        res->parent1 = c1;
                        res->parent2 = c2;

                        char mgu_buf[1024] = "";
                        calculate_simultaneous_mgu_string(sigma, mgu_buf);
                        free_substitution(sigma);

                        if (res->count == 0) {
                            char* mgu_copy = strdup(mgu_buf);
                            append_node(&head, &tail, res, mgu_copy);
                            free_clause(c2_copy);
                            *out_all_clauses = head;
                            return res;
                        }

                        if (!is_tautology(res) && !is_duplicate(res, head)) {
                            char* mgu_copy = strdup(mgu_buf);
                            append_node(&head, &tail, res, mgu_copy);
                            
                            for (int f1 = 0; f1 < res->count; f1++) {
                                for (int f2 = f1 + 1; f2 < res->count; f2++) {
                                    Substitution* fsigma = NULL;
                                    Clause* factored = factor_clause(res, f1, f2, &fsigma);
                                    if (factored) {
                                        if (!is_tautology(factored) && !is_duplicate(factored, head)) {
                                            char* fmgu = malloc(1024);
                                            strcpy(fmgu, "factor");
                                            char id1[12], id2[12];
                                            sprintf(id1, "%d", f1); sprintf(id2, "%d", f2);
                                            strcat(fmgu, id1); strcat(fmgu, ","); strcat(fmgu, id2);
                                            append_node(&head, &tail, factored, fmgu);
                                        } else {
                                            free_clause(factored);
                                        }
                                        if (fsigma) free_substitution(fsigma);
                                    }
                                }
                            }
                            steps++;
                        } else {
                            free_clause(res);
                        }
                    } else {
                        if (sigma) free_substitution(sigma);
                    }

                    free_clause(c2_copy);
                }
            }
            current2 = current2->next;
        }
        current1 = current1->next;
    }

    *out_all_clauses = head;
    return empty;
}
