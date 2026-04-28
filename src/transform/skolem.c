#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include "transform.h"
#include "core/ast.h"

static int skolem_counter = 0;

static char* make_skolem_name(void) {
    char buf[32];
    buf[0] = 'f';
    buf[1] = '_';
    int id = ++skolem_counter;
    int pos = 2;
    char tmp[16];
    int t_pos = 0;
    if (id == 0) tmp[t_pos++] = '0';
    while (id > 0) {
        tmp[t_pos++] = (char)((id % 10) + '0');
        id /= 10;
    }
    while (t_pos > 0) buf[pos++] = tmp[--t_pos];
    buf[pos] = '\0';
    return strdup(buf);
}


static void substitute_in_term(Term** t_ptr, const char* var_name, Term* sk_term) {
    Term* t = *t_ptr;
    if (!t) return;

    if (t->type == TERM_VARIABLE && strcmp(t->name, var_name) == 0) {
        free_term(t);
        *t_ptr = copy_term(sk_term);
        return;
    }

    for (int i = 0; i < t->arity; i++) {
        substitute_in_term(&t->args[i], var_name, sk_term);
    }
}

static void apply_skolem_substitution(ASTNode* n, const char* var_name, Term* sk_term) {
    if (!n) return;
    if (n->type == NODE_ATOM) {
        for (int i = 0; i < n->arity; i++) {
            substitute_in_term(&n->terms[i], var_name, sk_term);
        }
    } else {
        apply_skolem_substitution(n->left, var_name, sk_term);
        apply_skolem_substitution(n->right, var_name, sk_term);
    }
}

ASTNode* transform_skolemize_recursive(ASTNode* n, const char** univ_stack, int depth) {
    if (!n) return NULL;

    if (n->type == NODE_QUANTIFIER) {
        if (n->op == TOKEN_FORALL) {
            univ_stack[depth] = n->name;
            n->left = transform_skolemize_recursive(n->left, univ_stack, depth + 1);
            return n;
        } 
        else if (n->op == TOKEN_EXISTS) {
            Term* sk_term = malloc(sizeof(Term));
            sk_term->type = TERM_FUNCTION;
            sk_term->name = make_skolem_name();
            sk_term->arity = depth;
            sk_term->args = NULL;
            
            if (depth > 0) {
                sk_term->args = malloc(sizeof(Term*) * (size_t)depth);
                for (int i = 0; i < depth; i++) {
                    sk_term->args[i] = create_variable_term(univ_stack[i]);
                }
            }

            apply_skolem_substitution(n->left, n->name, sk_term);

            ASTNode* next = n->left;
            free(n->name);
            free(n);
            
            free_term(sk_term);

            return transform_skolemize_recursive(next, univ_stack, depth);
        }
    }

    n->left = transform_skolemize_recursive(n->left, univ_stack, depth);
    n->right = transform_skolemize_recursive(n->right, univ_stack, depth);
    return n;
}

ASTNode* transform_skolemize(ASTNode* n) {
    const char* univ_stack[128];
    skolem_counter = 0;
    return transform_skolemize_recursive(n, univ_stack, 0);
}