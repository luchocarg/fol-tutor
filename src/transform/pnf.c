#include "transform.h"
#include "core/ast.h"

ASTNode* transform_to_pnf(ASTNode* n) {
    if (!n) return NULL;

    if (n->type == NODE_BINARY) {
        n->left = transform_to_pnf(n->left);
        n->right = transform_to_pnf(n->right);
    } else if (n->type == NODE_UNARY || n->type == NODE_QUANTIFIER) {
        n->left = transform_to_pnf(n->left);
    }

    if (n->type == NODE_UNARY && n->left->type == NODE_QUANTIFIER) {
        ASTNode* q = n->left;
        q->op = (q->op == TOKEN_FORALL) ? TOKEN_EXISTS : TOKEN_FORALL;
        n->left = q->left;
        q->left = transform_to_pnf(n); 
        return q;
    }

    if (n->type == NODE_BINARY) {
        if ((n->left && n->left->type == NODE_QUANTIFIER && n->left->op == TOKEN_EXISTS) ||
            (n->right && n->right->type == NODE_QUANTIFIER && n->right->op == TOKEN_EXISTS)) {
            
            ASTNode* q = (n->left && n->left->op == TOKEN_EXISTS) ? n->left : n->right;
            if (q == n->left) n->left = q->left; else n->right = q->left;
            
            q->left = transform_to_pnf(n);
            return q;
        }

        if ((n->left && n->left->type == NODE_QUANTIFIER) || 
            (n->right && n->right->type == NODE_QUANTIFIER)) {
            
            ASTNode* q = (n->left && n->left->type == NODE_QUANTIFIER) ? n->left : n->right;
            if (q == n->left) n->left = q->left; else n->right = q->left;
            
            q->left = transform_to_pnf(n);
            return q;
        }
    }

    return n;
}