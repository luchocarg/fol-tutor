#include "transform.h"
#include "core/ast.h"

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