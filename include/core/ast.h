#ifndef AST_H
#define AST_H

#include <stddef.h>
#include "lexer.h"

typedef struct Term Term;
typedef struct ASTNode ASTNode;

typedef enum {
    TERM_VARIABLE,
    TERM_FUNCTION
} TermType;

struct Term {
    TermType type;
    char* name;
    struct Term** args;
    int arity;
};

typedef enum {
    NODE_QUANTIFIER,
    NODE_UNARY,
    NODE_BINARY,
    NODE_ATOM,
    NODE_FALSUM
} ASTNodeType;

struct ASTNode {
    ASTNodeType type;
    TokenType op;
    struct ASTNode *left;
    struct ASTNode *right; 
    char* name;
    Term** terms;
    int arity;
};


Term* copy_term(Term* src);
ASTNode* copy_ast(ASTNode* src);

void ast_to_formula(ASTNode* n, char* buf);
void term_to_formula(Term* t, char* buf);

ASTNode* create_node(ASTNodeType type);
ASTNode* create_unary_node(TokenType op, ASTNode* child);
void free_ast(ASTNode* node);
void ast_to_sexpr(ASTNode* n, char* buf);

Term* create_term(TermType type);
Term* create_variable_term(const char* name);
void free_term(Term* t);
void term_to_sexpr(Term* t, char* buf);

#endif