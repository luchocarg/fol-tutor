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

typedef struct Clause {
    Literal** literals;   
    int count;            
    int capacity;         
    int id;               
    struct Clause* parent1;
    struct Clause* parent2;
} Clause;

typedef struct {
    Clause** clauses;     
    int count;
    int capacity;
} ClauseSet;

Literal* extract_literal_from_node(ASTNode* node);
void free_literal(Literal* l);

void reset_clause_id_counter(void);
int get_next_clause_id(void);
Clause* create_empty_clause(void);

ClauseSet* create_clause_set(void);
void free_clause_set(ClauseSet* set);
ClauseSet* ast_to_clause_set(ASTNode* matrix_root);

Clause* get_clause(ClauseSet* set, int index);
Literal* get_literal(Clause* clause, int index);
Literal* copy_literal(Literal* src);
void free_clause(Clause* c);

void clause_to_formula(Clause* c, char* buf);
void clause_to_formula_sep(Clause* c, const char* sep, char* buf);
void clause_set_to_formula(ClauseSet* set, char* buf);
void ast_to_cnf_sets(ASTNode* n, char* buf);

#endif