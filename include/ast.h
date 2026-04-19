#ifndef AST_H
#define AST_H

#include <stddef.h>
#include "lexer.h"

typedef struct ASTNode ASTNode;
typedef struct Term Term;

typedef enum {
    NODE_QUANTIFIER,
    NODE_UNARY,
    NODE_BINARY,
    NODE_PREDICATE,
    NODE_FALSUM
} ASTNodeType;

struct ASTNode {
    ASTNodeType type;
    TokenType op;
    struct ASTNode *left;
    struct ASTNode *right; // restricted for binary ops
    char* name;
    Term** terms;
    int term_count;
};

ASTNode* create_node(ASTNodeType type);
void free_ast(ASTNode* node);

#endif