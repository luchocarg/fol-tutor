#include "transform.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>

ASTNode* remove_implications(ASTNode* n) {
    if (!n) return NULL;

    n->left = remove_implications(n->left);
    n->right = remove_implications(n->right);

    if (n->type == NODE_BINARY && n->op == TOKEN_IMPLIES) {
        ASTNode* sigma = n->left;

        ASTNode* not_sigma = create_node(NODE_UNARY);
        not_sigma->op = TOKEN_NOT;
        not_sigma->left = sigma;

        n->op = TOKEN_OR;
        n->left = not_sigma;
    }

    return n;
}

ASTNode* apply_nnf(ASTNode* n) {
    if (!n) return NULL;

    if (n->type == NODE_UNARY && n->op == TOKEN_NOT) {
        ASTNode* child = n->left;

        // not not A => A
        if (child->type == NODE_UNARY && child->op == TOKEN_NOT) {
            ASTNode* res = apply_nnf(child->left);
            child->left = NULL; 
            free_ast(n); 
            return res;
        }

        // not (A and B) => not A or not B
        if (child->type == NODE_BINARY && child->op == TOKEN_AND) {
            ASTNode* new_node = create_node(NODE_BINARY);
            new_node->op = TOKEN_OR;
            new_node->left = apply_nnf(create_unary_node(TOKEN_NOT, child->left));
            new_node->right = apply_nnf(create_unary_node(TOKEN_NOT, child->right));
            child->left = NULL; child->right = NULL;
            free_ast(n);
            return new_node;
        }

        // not (A or B) => not A and not B
        if (child->type == NODE_BINARY && child->op == TOKEN_OR) {
            ASTNode* new_node = create_node(NODE_BINARY);
            new_node->op = TOKEN_AND;
            new_node->left = apply_nnf(create_unary_node(TOKEN_NOT, child->left));
            new_node->right = apply_nnf(create_unary_node(TOKEN_NOT, child->right));
            child->left = NULL; child->right = NULL;
            free_ast(n);
            return new_node;
        }

        // not ∀X.P => ∃X. not P
        if (child->type == NODE_QUANTIFIER && child->op == TOKEN_FORALL) {
            child->op = TOKEN_EXISTS;
            child->left = apply_nnf(create_unary_node(TOKEN_NOT, child->left));
            ASTNode* res = child;
            n->left = NULL; free_ast(n);
            return res;
        }
        
        // not ∃X.P => ∀X. not P
        if (child->type == NODE_QUANTIFIER && child->op == TOKEN_EXISTS) {
            child->op = TOKEN_FORALL;
            child->left = apply_nnf(create_unary_node(TOKEN_NOT, child->left));
            ASTNode* res = child;
            n->left = NULL; free_ast(n);
            return res;
        }
    }

    n->left = apply_nnf(n->left);
    n->right = apply_nnf(n->right);
    return n;
}

static char* dup_str(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* d = (char*)malloc(len);
    if (d) memcpy(d, s, len);
    return d;
}

static char* make_fresh_name(int id) {
    char buf[16];
    int pos = 15;
    buf[pos--] = '\0';
    
    do {
        buf[pos--] = (char)((id % 10) + '0');
        id /= 10;
    } while (id > 0);
    
    buf[pos--] = '_';
    buf[pos] = 'X';
    
    return dup_str(&buf[pos]);
}

// Búsqueda en el scope actual
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
            if (t->name) free(t->name);
            t->name = dup_str(new_name);
        }
    }

    for (int i = 0; i < t->arity; i++) {
        rename_in_term(t->args[i], map);
    }
}

static int alpha_counter = 0;

void reset_alpha_counter(void) {
    alpha_counter = 0;
}

void apply_alpha_renaming(ASTNode* n, VarMap* map) {
    if (!n) return;

    if (n->type == NODE_QUANTIFIER) {
        char* fresh = make_fresh_name(++alpha_counter);
        char* old_name = n->name;
        
        VarMap new_scope = { old_name, fresh, map };
        
        n->name = fresh;
        
        apply_alpha_renaming(n->left, &new_scope);

        if (old_name) free(old_name);
    } 
    else if (n->type == NODE_PREDICATE) {
        for (int i = 0; i < (int)n->term_count; i++) {
            rename_in_term(n->terms[i], map);
        }
    } 
    else {
        apply_alpha_renaming(n->left, map);
        apply_alpha_renaming(n->right, map);
    }
}

ASTNode* apply_pnf(ASTNode* n) {
    if (!n) return NULL;

    if (n->type == NODE_QUANTIFIER) {
        n->left = apply_pnf(n->left);
        return n;
    }

    if (n->type == NODE_BINARY) {
        n->left = apply_pnf(n->left);
        n->right = apply_pnf(n->right);

        if (n->left && n->left->type == NODE_QUANTIFIER) {
            ASTNode* q = n->left;
            n->left = q->left;
            q->left = apply_pnf(n);
            return q;
        }

        if (n->right && n->right->type == NODE_QUANTIFIER) {
            ASTNode* q = n->right;      
            n->right = q->left;         
            q->left = apply_pnf(n);     
            return q;                   
        }
    }

   if (n->type == NODE_UNARY) {
        n->left = apply_pnf(n->left);
    }

    return n;
}