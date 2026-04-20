#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "symbol_table.h"
#include "ast.h"

typedef struct {
    Lexer* l;
    SymbolTable* st;
} Parser;

ASTNode* parse_formula(Parser* p);
ASTNode* parse_implication(Parser* p);
ASTNode* parse_disjunction(Parser* p);
ASTNode* parse_conjunction(Parser* p);
ASTNode* parse_unary(Parser* p);
ASTNode* parse_primary(Parser* p);

Term* parse_term(Parser* p);
Term** parse_term_list(Parser* p, int* count);

#endif