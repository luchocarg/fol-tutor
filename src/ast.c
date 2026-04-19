#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"

ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type =  type;
    node->op =    TOKEN_ERROR;
    node->left =  NULL;
    node->right = NULL;
    node->name =  NULL;
    node->terms = NULL;
    node->term_count = 0;
    
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;

    free_ast(node->left);
    free_ast(node->right);

    if (node->terms) {
        for (int i = 0; i < node->term_count; i++) {
            free_term(node->terms[i]);
        }
        free(node->terms);
    }

    if (node->name) free(node->name);
    free(node);
}