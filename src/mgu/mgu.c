#define _POSIX_C_SOURCE 200809L
#include "mgu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Term* get_substitution(Substitution* s, const char* var_name) {
    while (s) {
        if (strcmp(s->var_name, var_name) == 0) return s->term;
        s = s->next;
    }
    return NULL;
}

static bool occurs_check(const char* var_name, Term* t, Substitution* s) {
    if (!t) return false;
    
    Term* sub = get_substitution(s, t->name);
    if (sub && t->type == TERM_VARIABLE) {
        return occurs_check(var_name, sub, s);
    }

    if (t->type == TERM_VARIABLE && strcmp(var_name, t->name) == 0) return true;

    for (int i = 0; i < t->arity; i++) {
        if (occurs_check(var_name, t->args[i], s)) return true;
    }
    return false;
}

static void apply_sub_to_term(Term** t_ptr, const char* var_name, Term* replacement) {
    Term* t = *t_ptr;
    if (!t) return;

    if (t->type == TERM_VARIABLE && strcmp(t->name, var_name) == 0) {
        free_term(t);
        *t_ptr = copy_term(replacement);
        return;
    }

    for (int i = 0; i < t->arity; i++) {
        apply_sub_to_term(&t->args[i], var_name, replacement);
    }
}

static void add_substitution(Substitution** s, const char* var_name, Term* term) {
    Substitution* curr = *s;
    while (curr) {
        apply_sub_to_term(&curr->term, var_name, term);
        curr = curr->next;
    }

    Substitution* new_sub = malloc(sizeof(Substitution));
    new_sub->var_name = strdup(var_name);
    new_sub->term = copy_term(term);
    new_sub->next = *s;
    *s = new_sub;
}

bool unify_terms(Term* t1, Term* t2, Substitution** s) {
    while (t1->type == TERM_VARIABLE) {
        Term* sub = get_substitution(*s, t1->name);
        if (!sub) break;
        t1 = sub;
    }
    while (t2->type == TERM_VARIABLE) {
        Term* sub = get_substitution(*s, t2->name);
        if (!sub) break;
        t2 = sub;
    }

    if (t1->type == TERM_VARIABLE && t2->type == TERM_VARIABLE && strcmp(t1->name, t2->name) == 0) {
        return true;
    }

    if (t1->type == TERM_VARIABLE) {
        if (occurs_check(t1->name, t2, *s)) return false;
        add_substitution(s, t1->name, t2);
        return true;
    }

    if (t2->type == TERM_VARIABLE) {
        if (occurs_check(t2->name, t1, *s)) return false;
        add_substitution(s, t2->name, t1);
        return true;
    }

    if (strcmp(t1->name, t2->name) != 0 || t1->arity != t2->arity) return false;

    for (int i = 0; i < t1->arity; i++) {
        if (!unify_terms(t1->args[i], t2->args[i], s)) return false;
    }

    return true;
}

void free_substitution(Substitution* s) {
    while (s) {
        Substitution* next = s->next;
        free(s->var_name);
        free_term(s->term);
        free(s);
        s = next;
    }
}

Substitution* calculate_mgu(Literal* l1, Literal* l2, bool* success) {
    *success = true;
    if (strcmp(l1->predicate_name, l2->predicate_name) != 0 || l1->arity != l2->arity) {
        *success = false;
        return NULL;
    }

    Substitution* s = NULL;
    for (int i = 0; i < l1->arity; i++) {
        if (!unify_terms(l1->args[i], l2->args[i], &s)) {
            *success = false;
            free_substitution(s);
            return NULL;
        }
    }
    return s;
}

void apply_substitution_to_term(Term** t_ptr, Substitution* s) {
    Term* t = *t_ptr;
    if (!t || !s) return;

    if (t->type == TERM_VARIABLE) {
        Term* replacement = get_substitution(s, t->name);
        if (replacement) {
            free_term(t);
            *t_ptr = copy_term(replacement);
            return; 
        }
    }

    for (int i = 0; i < t->arity; i++) {
        apply_substitution_to_term(&t->args[i], s);
    }
}

void apply_substitution_to_literal(Literal* l, Substitution* s) {
    if (!l || !s) return;
    for (int i = 0; i < l->arity; i++) {
        apply_substitution_to_term(&l->args[i], s);
    }
}

void apply_substitution_to_clause(Clause* c, Substitution* s) {
    if (!c || !s) return;
    for (int i = 0; i < c->count; i++) {
        apply_substitution_to_literal(c->literals[i], s);
    }
}

void calculate_mgu_string(Literal* l1, Literal* l2, char* output_buffer) {
    output_buffer[0] = '\0';
    bool success;
    Substitution* s = calculate_mgu(l1, l2, &success);

    if (!success) {
        strcpy(output_buffer, "Fail");
        return;
    }

    if (!s) {
        strcpy(output_buffer, "{}");
        return;
    }

    strcat(output_buffer, "{");
    Substitution* curr = s;
    while (curr) {
        char term_buf[512] = "";
        term_to_formula(curr->term, term_buf);
        
        char pair_buf[550];
        snprintf(pair_buf, sizeof(pair_buf), "%s := %s", curr->var_name, term_buf);
        strcat(output_buffer, pair_buf);

        if (curr->next) strcat(output_buffer, ", ");
        curr = curr->next;
    }
    strcat(output_buffer, "}");

    free_substitution(s);
}

Clause* create_resolvent(Clause* c1, int index1, Clause* c2, int index2, Substitution* sigma) {
    Clause* res = malloc(sizeof(Clause));
    res->capacity = (c1->count + c2->count);
    res->count = 0;
    res->literals = malloc((size_t)res->capacity * sizeof(Literal*));

    for (int i = 0; i < c1->count; i++) {
        if (i == index1) continue;
        Literal* lit = copy_literal(c1->literals[i]);
        apply_substitution_to_literal(lit, sigma);
        res->literals[res->count++] = lit;
    }

    for (int i = 0; i < c2->count; i++) {
        if (i == index2) continue;
        Literal* lit = copy_literal(c2->literals[i]);
        apply_substitution_to_literal(lit, sigma);
        res->literals[res->count++] = lit;
    }

    return res;
}