#include "transform.h"
#include "core/ast.h"

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