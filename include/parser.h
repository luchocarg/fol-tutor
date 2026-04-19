#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "symbol_table.h"
#include "ast.h"

extern int parser_silent_mode;

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

ASTNode* parse_formula(Lexer* l, SymbolTable* st);
ASTNode* parse_implication(Lexer* l, SymbolTable* st);
ASTNode* parse_disjunction(Lexer* l, SymbolTable* st);
ASTNode* parse_conjunction(Lexer* l, SymbolTable* st);
ASTNode* parse_unary(Lexer* l, SymbolTable* st);
ASTNode* parse_primary(Lexer* l, SymbolTable* st);

Term* parse_term(Lexer* l, SymbolTable* st);
Term** parse_term_list(Lexer* l, int* count, SymbolTable* st);
Term* create_term(TermType type);
void free_term(Term* t);

#endif