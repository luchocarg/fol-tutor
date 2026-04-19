#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "symbol_table.h"
#include "parser.h"
#include "ast.h"

typedef struct VarMap {
    const char* original;
    const char* replacement;
    struct VarMap* next;
} VarMap;

ASTNode* remove_implications(ASTNode* n);
ASTNode* apply_nnf(ASTNode* n);
void reset_alpha_counter(void);
void apply_alpha_renaming(ASTNode* n, VarMap* map);
ASTNode* apply_pnf(ASTNode* n);

#endif