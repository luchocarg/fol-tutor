#ifndef MGU_H
#define MGU_H

#include "core/ast.h"
#include "core/cnf.h"
#include <stdbool.h>

typedef struct Substitution {
    char* var_name;
    Term* term;
    struct Substitution* next;
} Substitution;

void calculate_mgu_string(Literal* l1, Literal* l2, char* output_buffer);
bool unify_terms(Term* t1, Term* t2, Substitution** s);
void free_substitution(Substitution* s);

Substitution* calculate_mgu(Literal* l1, Literal* l2, bool* success);

void apply_substitution_to_term(Term** t_ptr, Substitution* s);
void apply_substitution_to_literal(Literal* l, Substitution* s);
void apply_substitution_to_clause(Clause* c, Substitution* s);

Clause* create_resolvent(Clause* c1, int index1, Clause* c2, int index2, Substitution* sigma);

#endif