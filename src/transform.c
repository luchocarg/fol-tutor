#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include "transform.h"
#include "ast.h"

static char* make_fresh_name(int id) {
    char buf[32];
    int pos = 31;
    buf[pos--] = '\0';

    if (id == 0) {
        buf[pos--] = '0';
    } else {
        while (id > 0 && pos > 2) {
            buf[pos--] = (char)((id % 10) + '0');
            id /= 10;
        }
    }

    buf[pos--] = '_';
    buf[pos] = 'X';

    return strdup(&buf[pos]);
}

static const char* lookup_var(VarMap* map, const char* name) {
    for (; map; map = map->next) {
        if (strcmp(map->original, name) == 0) return map->replacement;
    }
    return name;
}

static void rename_in_term(Term* t, VarMap* map) {
    if (!t) return;

    if (t->type == TERM_VARIABLE) {
        const char* new_name = lookup_var(map, t->name);
        if (new_name != t->name) {
            free(t->name);
            t->name = strdup(new_name);
        }
    }

    for (int i = 0; i < t->arity; i++) {
        rename_in_term(t->args[i], map);
    }
}

static int alpha_counter = 0;
void transform_reset_alpha_counter(void) { alpha_counter = 0; }

void transform_alpha_rename(ASTNode* n, VarMap* map) {
    if (!n) return;

    if (n->type == NODE_QUANTIFIER) {
        char* fresh = make_fresh_name(++alpha_counter);
        char* old_name = n->name;
        
        VarMap new_scope = { old_name, fresh, map };
        
        n->name = fresh;
        transform_alpha_rename(n->left, &new_scope);

        if (old_name) free(old_name);
    } 
    else if (n->type == NODE_ATOM) {
        const char* new_atom_name = lookup_var(map, n->name);
        if (new_atom_name != n->name) {
            free(n->name);
            n->name = strdup(new_atom_name);
        }

        for (int i = 0; i < n->arity; i++) {
            rename_in_term(n->terms[i], map);
        }
    } 
    else {
        transform_alpha_rename(n->left, map);
        transform_alpha_rename(n->right, map);
    }
}

ASTNode* transform_remove_implications(ASTNode* n) {
    if (!n) return NULL;

    n->left = transform_remove_implications(n->left);
    n->right = transform_remove_implications(n->right);

    if (n->type == NODE_BINARY && n->op == TOKEN_IMPLIES) {
        ASTNode* sigma = n->left;
        n->left = create_unary_node(TOKEN_NOT, sigma);
        n->op = TOKEN_OR;
    }
    return n;
}

ASTNode* transform_to_nnf(ASTNode* n) {
    if (!n) return NULL;

    if (n->type == NODE_UNARY && n->op == TOKEN_NOT) {
        ASTNode* child = n->left;

        if (child->type == NODE_UNARY && child->op == TOKEN_NOT) {
            ASTNode* res = transform_to_nnf(child->left);
            child->left = NULL; 
            free_ast(n); 
            return res;
        }

        if (child->type == NODE_BINARY && child->op == TOKEN_AND) {
            ASTNode* new_node = create_node(NODE_BINARY);
            new_node->op = TOKEN_OR;
            new_node->left = transform_to_nnf(create_unary_node(TOKEN_NOT, child->left));
            new_node->right = transform_to_nnf(create_unary_node(TOKEN_NOT, child->right));
            child->left = child->right = NULL;
            free_ast(n);
            return new_node;
        }

        if (child->type == NODE_BINARY && child->op == TOKEN_OR) {
            ASTNode* new_node = create_node(NODE_BINARY);
            new_node->op = TOKEN_AND;
            new_node->left = transform_to_nnf(create_unary_node(TOKEN_NOT, child->left));
            new_node->right = transform_to_nnf(create_unary_node(TOKEN_NOT, child->right));
            child->left = child->right = NULL;
            free_ast(n);
            return new_node;
        }

        if (child->type == NODE_QUANTIFIER) {
            child->op = (child->op == TOKEN_FORALL) ? TOKEN_EXISTS : TOKEN_FORALL;
            child->left = transform_to_nnf(create_unary_node(TOKEN_NOT, child->left));
            ASTNode* res = child;
            n->left = NULL; free_ast(n);
            return res;
        }
    }

    n->left = transform_to_nnf(n->left);
    n->right = transform_to_nnf(n->right);
    return n;
}

ASTNode* transform_to_pnf(ASTNode* n) {
    if (!n) return NULL;

    if (n->type == NODE_QUANTIFIER) {
        n->left = transform_to_pnf(n->left);
        return n;
    }

    if (n->type == NODE_BINARY) {
        n->left = transform_to_pnf(n->left);
        n->right = transform_to_pnf(n->right);

        if (n->left && n->left->type == NODE_QUANTIFIER) {
            ASTNode* q = n->left;
            n->left = q->left;
            q->left = transform_to_pnf(n);
            return q;
        }
        if (n->right && n->right->type == NODE_QUANTIFIER) {
            ASTNode* q = n->right;      
            n->right = q->left;         
            q->left = transform_to_pnf(n);     
            return q;                   
        }
    }

    if (n->type == NODE_UNARY) n->left = transform_to_pnf(n->left);
    return n;
}