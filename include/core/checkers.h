#ifndef CHECKERS_H
#define CHECKERS_H

#include "ast.h"
#include <stdbool.h>

bool is_formula_impl_free(ASTNode* n);
bool is_formula_nnf(ASTNode* n);
bool is_formula_skolemized(ASTNode* n);
bool is_formula_pnf(ASTNode* n);
bool is_formula_rectified(ASTNode* n);

#endif
