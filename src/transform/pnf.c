#include "transform.h"
#include "core/ast.h"

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