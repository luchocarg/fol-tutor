#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "core/ast.h"

typedef struct VarMap {
    const char* original;
    const char* replacement;
    struct VarMap* next;
} VarMap;

ASTNode* transform_remove_implications(ASTNode* n);
ASTNode* transform_to_nnf(ASTNode* n);
void transform_reset_alpha_counter(void);
void transform_alpha_rename(ASTNode* n, VarMap* map);
ASTNode* transform_to_pnf(ASTNode* n);
ASTNode* transform_skolemize(ASTNode* n);
ASTNode* transform_skolemize_recursive(ASTNode* n, const char** univ_stack, int depth);
ASTNode* transform_distribute(ASTNode* n);
ASTNode* transform_push_universals(ASTNode* n);

#endif