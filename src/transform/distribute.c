#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include "transform.h"
#include "core/ast.h"

ASTNode* transform_distribute(ASTNode* n) {
    if (!n) return NULL;

    n->left = transform_distribute(n->left);
    n->right = transform_distribute(n->right);

    if (n->type == NODE_BINARY && n->op == TOKEN_OR) {
        if (n->right && n->right->type == NODE_BINARY && n->right->op == TOKEN_AND) {
            ASTNode* A = n->left;
            ASTNode* B = n->right->left;
            ASTNode* C = n->right->right;
            ASTNode* and_node = n->right;

            ASTNode* new_or_left = create_node(NODE_BINARY);
            new_or_left->op = TOKEN_OR;
            new_or_left->left = copy_ast(A);
            new_or_left->right = B;

            ASTNode* new_or_right = create_node(NODE_BINARY);
            new_or_right->op = TOKEN_OR;
            new_or_right->left = A;
            new_or_right->right = C;

            and_node->left = transform_distribute(new_or_left);
            and_node->right = transform_distribute(new_or_right);

            free(n);
            return and_node;
        }
        if (n->left && n->left->type == NODE_BINARY && n->left->op == TOKEN_AND) {
            ASTNode* B = n->left->left;
            ASTNode* C = n->left->right;
            ASTNode* A = n->right;
            ASTNode* and_node = n->left;

            ASTNode* new_or_left = create_node(NODE_BINARY);
            new_or_left->op = TOKEN_OR;
            new_or_left->left = B;
            new_or_left->right = copy_ast(A);

            ASTNode* new_or_right = create_node(NODE_BINARY);
            new_or_right->op = TOKEN_OR;
            new_or_right->left = C;
            new_or_right->right = A;

            and_node->left = transform_distribute(new_or_left);
            and_node->right = transform_distribute(new_or_right);

            free(n);
            return and_node;
        }
    }
    return n;
}