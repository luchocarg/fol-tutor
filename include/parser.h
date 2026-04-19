#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "symbol_table.h"

extern int parser_silent_mode;

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

ASTNode* parse_formula(Lexer* l, SymbolTable* st);
ASTNode* parse_implication(Lexer* l, SymbolTable* st);
ASTNode* parse_disjunction(Lexer* l, SymbolTable* st);
ASTNode* parse_conjunction(Lexer* l, SymbolTable* st);
ASTNode* parse_unary(Lexer* l, SymbolTable* st);
ASTNode* parse_primary(Lexer* l, SymbolTable* st);

Term* parse_term(Lexer* l, SymbolTable* st);
Term** parse_term_list(Lexer* l, int* count, SymbolTable* st);
Term* create_term(TermType type);

ASTNode* create_node(ASTNodeType type);
void free_ast(ASTNode* node);
void free_term(Term* t);

#endif