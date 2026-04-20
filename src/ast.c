#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

Term* create_term(TermType type) {
    Term* t = (Term*)malloc(sizeof(Term));
    if (!t) return NULL;
    t->type = type;
    t->name = NULL;
    t->args = NULL;
    t->arity = 0;
    return t;
}

void free_term(Term* t) {
    if (!t) return;
    if (t->args) {
        for (int i = 0; i < t->arity; i++) free_term(t->args[i]);
        free(t->args);
    }
    if (t->name) free(t->name);
    free(t);
}

void term_to_sexpr(Term* t, char* buf) {
    if (!t) return;
    if (t->type == TERM_FUNCTION && t->arity > 0) {
        strcat(buf, "(");
        strcat(buf, t->name);
        for (int i = 0; i < t->arity; i++) {
            strcat(buf, " ");
            term_to_sexpr(t->args[i], buf);
        }
        strcat(buf, ")");
    } else {
        strcat(buf, t->name);
    }
}

ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = type;
    node->op = TOKEN_ERROR;
    node->left = node->right = NULL;
    node->name = NULL;
    node->terms = NULL;
    node->arity = 0;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;

    free_ast(node->left);
    free_ast(node->right);

    if (node->terms) {
        for (int i = 0; i < node->arity; i++) {
            free_term(node->terms[i]);
        }
        free(node->terms);
    }

    if (node->name) free(node->name);
    free(node);
}

void ast_to_sexpr(ASTNode* n, char* buf) {
    if (!n) return;
    switch (n->type) {
        case NODE_ATOM:
            if (n->arity > 0) {
                strcat(buf, "(");
            }
            
            if (n->name) strcat(buf, n->name);
            
            for (int i = 0; i < n->arity; i++) {
                strcat(buf, " ");
                term_to_sexpr(n->terms[i], buf);
            }
            
            if (n->arity > 0) {
                strcat(buf, ")");
            }
            break;
            
        case NODE_QUANTIFIER:
            strcat(buf, "(");
            strcat(buf, n->op == TOKEN_FORALL ? "∀" : "∃");
            strcat(buf, " ");
            if (n->name) strcat(buf, n->name);
            strcat(buf, " ");
            ast_to_sexpr(n->left, buf);
            strcat(buf, ")");
            break;
            
        case NODE_UNARY:
            strcat(buf, "(¬ ");
            ast_to_sexpr(n->left, buf);
            strcat(buf, ")");
            break;
            
        case NODE_BINARY:
            strcat(buf, "(");
            const char* op_str = (n->op == TOKEN_AND) ? "∧" : 
                                 (n->op == TOKEN_OR ? "∨" : "⇒");
            strcat(buf, op_str);
            strcat(buf, " ");
            ast_to_sexpr(n->left, buf);
            strcat(buf, " ");
            ast_to_sexpr(n->right, buf);
            strcat(buf, ")");
            break;
            
        case NODE_FALSUM:
            strcat(buf, "⊥");
            break;
            
        default: break;
    }
}

ASTNode* create_unary_node(TokenType op, ASTNode* child) {
    ASTNode* node = create_node(NODE_UNARY);
    if (!node) return NULL;
    node->op = op;
    node->left = child;
    node->right = NULL;
    return node;
}