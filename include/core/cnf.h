#ifndef CNF_H
#define CNF_H

#include "core/ast.h"
#include <stdbool.h>

typedef struct {
    bool is_negative;     
    char* predicate_name; 
    Term** args;          
    int arity;            
} Literal;

typedef struct {
    Literal** literals;   
    int count;            
    int capacity;         
} Clause;

typedef struct {
    Clause** clauses;     
    int count;
    int capacity;
} ClauseSet;

Literal* extract_literal_from_node(ASTNode* node);
void free_literal(Literal* l);

ClauseSet* create_clause_set(void);
void free_clause_set(ClauseSet* set);

ClauseSet* ast_to_clause_set(ASTNode* matrix_root);

Clause* get_clause(ClauseSet* set, int index);
Literal* get_literal(Clause* clause, int index);

Literal* copy_literal(Literal* src);

void free_clause(Clause* c);

void clause_to_formula(Clause* c, char* buf);

#endif