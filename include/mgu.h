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

typedef struct ClauseNode {
    Clause* c;
    char*   mgu_str;
    struct ClauseNode* next;
} ClauseNode;


void calculate_mgu_string(Literal* l1, Literal* l2, char* output_buffer);
bool unify_terms(Term* t1, Term* t2, Substitution** s);
void free_substitution(Substitution* s);
Term* get_substitution(Substitution* s, const char* var_name);

Substitution* parse_substitution_string(const char* input);
Substitution* calculate_mgu(Literal* l1, Literal* l2, bool* success);
Substitution* calculate_simultaneous_mgu(Literal** L1, int c1, Literal** L2, int c2, bool* success);

void apply_substitution_to_term(Term** t_ptr, Substitution* s);
void apply_substitution_to_literal(Literal* l, Substitution* s);
void apply_substitution_to_clause(Clause* c, Substitution* s);

Clause* create_resolvent(Clause* c1, int index1, Clause* c2, int index2, Substitution* sigma);
Clause* create_general_resolvent(Clause* c1, int* mask1, int mask1_len, Clause* c2, int* mask2, int mask2_len, Substitution* sigma);

void calculate_simultaneous_mgu_string(Substitution* s, char* output_buffer);
void standardize_apart_clause(Clause* target, Clause* context);

Clause* factor_clause(Clause* c, int i1, int i2, Substitution** out_sigma);
void calculate_mgu_trace(Literal* l1, Literal* l2, char* trace_buf);

Clause* run_automated_resolution(ClauseSet* initial_set, ClauseNode** out_all_clauses, int max_steps);

#endif