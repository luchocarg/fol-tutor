#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include "transform.h"
#include "core/ast.h"

static ASTNode* wrap_with_forall_prefix(ASTNode* body, const char** vars, int count) {
    if (count == 0 || !body) return body;

    ASTNode* current_root = body;
    for (int i = count - 1; i >= 0; i--) {
        ASTNode* new_node = create_node(NODE_QUANTIFIER);
        new_node->op = TOKEN_FORALL;
        new_node->name = strdup(vars[i]);
        new_node->left = current_root;
        new_node->right = NULL;
        current_root = new_node;
    }
    return current_root;
}

static ASTNode* push_universals_recursive(ASTNode* n, const char** stack, int depth) {
    if (!n) return NULL;

    if (n->type == NODE_QUANTIFIER && n->op == TOKEN_FORALL) {
        stack[depth] = n->name;
        ASTNode* inner_content = n->left;
        
        char* var_name_in_stack = n->name; 
        
        ASTNode* result = push_universals_recursive(inner_content, stack, depth + 1);
        
        free(var_name_in_stack);
        free(n);
        return result;
    }

    if (n->type == NODE_BINARY && n->op == TOKEN_AND) {
        n->left = push_universals_recursive(n->left, stack, depth);
        n->right = push_universals_recursive(n->right, stack, depth);
        return n;
    }

    return wrap_with_forall_prefix(n, stack, depth);
}

ASTNode* transform_push_universals(ASTNode* n) {
    const char* univ_stack[128]; 
    return push_universals_recursive(n, univ_stack, 0);
}