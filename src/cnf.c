#define _POSIX_C_SOURCE 200809L
#include "core/cnf.h"
#include <stdlib.h>
#include <string.h>

ClauseSet* create_clause_set(void) {
    ClauseSet* set = malloc(sizeof(ClauseSet));
    if (!set) return NULL;
    set->capacity = 16;
    set->count = 0;
    set->clauses = malloc((size_t)set->capacity * sizeof(Clause*));
    return set;
}

static Clause* create_clause(void) {
    Clause* c = malloc(sizeof(Clause));
    if (!c) return NULL;
    c->capacity = 8;
    c->count = 0;
    c->literals = malloc((size_t)c->capacity * sizeof(Literal*));
    return c;
}

static void add_clause(ClauseSet* set, Clause* c) {
    if (set->count == set->capacity) {
        set->capacity *= 2;
        set->clauses = realloc(set->clauses, (size_t)set->capacity * sizeof(Clause*));
    }
    set->clauses[set->count++] = c;
}

static void add_literal(Clause* c, Literal* l) {
    if (c->count == c->capacity) {
        c->capacity *= 2;
        c->literals = realloc(c->literals, (size_t)c->capacity * sizeof(Literal*));
    }
    c->literals[c->count++] = l;
}

Literal* extract_literal_from_node(ASTNode* node) {
    Literal* l = malloc(sizeof(Literal));
    l->is_negative = false;

    if (node->type == NODE_UNARY && node->op == TOKEN_NOT) {
        l->is_negative = true;
        node = node->left; 
    }

    if (node->type == NODE_ATOM) {
        l->predicate_name = strdup(node->name); 
        l->arity = node->arity;
        
        l->args = malloc((size_t)l->arity * sizeof(Term*));
        for (int i = 0; i < l->arity; i++) {
            l->args[i] = copy_term(node->terms[i]); 
        }
    } else {
        free(l);
        return NULL; 
    }
    return l;
}

static void extract_disjunction(ASTNode* node, Clause* current_clause) {
    if (!node) return;
    if (node->type == NODE_BINARY && node->op == TOKEN_OR) {
        extract_disjunction(node->left, current_clause);
        extract_disjunction(node->right, current_clause);
    } else {
        Literal* l = extract_literal_from_node(node);
        if (l) add_literal(current_clause, l);
    }
}

static void process_matrix(ASTNode* node, ClauseSet* set) {
    if (!node) return;
    if (node->type == NODE_BINARY && node->op == TOKEN_AND) {
        process_matrix(node->left, set);
        process_matrix(node->right, set);
    } else {
        Clause* new_clause = create_clause();
        extract_disjunction(node, new_clause);
        add_clause(set, new_clause);
    }
}

ClauseSet* ast_to_clause_set(ASTNode* matrix_root) {
    ClauseSet* set = create_clause_set();
    process_matrix(matrix_root, set);
    return set;
}

Clause* get_clause(ClauseSet* set, int index) {
    if (!set || index < 0 || index >= set->count) return NULL;
    return set->clauses[index];
}

Literal* get_literal(Clause* clause, int index) {
    if (!clause || index < 0 || index >= clause->count) return NULL;
    return clause->literals[index];
}

void free_clause_set(ClauseSet* set) {
    if (!set) return;
    for (int i = 0; i < set->count; i++) {
        Clause* c = set->clauses[i];
        if (c) {
            for (int j = 0; j < c->count; j++) {
                Literal* l = c->literals[j];
                if (l) {
                    free(l->predicate_name);
                    for (int k = 0; k < l->arity; k++) {
                        free_term(l->args[k]); 
                    }
                    free(l->args);
                    free(l);
                }
            }
            free(c->literals);
            free(c);
        }
    }
    free(set->clauses);
    free(set);
}

void free_literal(Literal* l) {
    if (!l) return;
    
    if (l->predicate_name) {
        free(l->predicate_name);
    }
    
    if (l->args) {
        for (int i = 0; i < l->arity; i++) {
            free_term(l->args[i]); 
        }
        free(l->args);
    }
    
    free(l);
}

Literal* copy_literal(Literal* src) {
    if (!src) return NULL;
    Literal* dst = malloc(sizeof(Literal));
    dst->is_negative = src->is_negative;
    dst->predicate_name = strdup(src->predicate_name);
    dst->arity = src->arity;
    dst->args = malloc((size_t)dst->arity * sizeof(Term*));
    for (int i = 0; i < dst->arity; i++) {
        dst->args[i] = copy_term(src->args[i]);
    }
    return dst;
}

void free_clause(Clause* c) {
    if (!c) return;
    for (int i = 0; i < c->count; i++) free_literal(c->literals[i]);
    free(c->literals);
    free(c);
}

void clause_to_formula(Clause* c, char* buf) {
    if (c->count == 0) {
        strcat(buf, "⊥");
        return;
    }
    strcat(buf, "{");
    for (int i = 0; i < c->count; i++) {
        Literal* l = c->literals[i];
        if (l->is_negative) strcat(buf, "¬");
        strcat(buf, l->predicate_name);
        if (l->arity > 0) {
            strcat(buf, "(");
            for (int k = 0; k < l->arity; k++) {
                char t_buf[256] = "";
                term_to_formula(l->args[k], t_buf);
                strcat(buf, t_buf);
                if (k < l->arity - 1) strcat(buf, ", ");
            }
            strcat(buf, ")");
        }
        if (i < c->count - 1) strcat(buf, ", ");
    }
    strcat(buf, "}");
}