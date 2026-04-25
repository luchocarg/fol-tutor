#include "core/checkers.h"
#include <stdlib.h>
#include <string.h>

bool is_formula_impl_free(ASTNode *n) {
  if (!n)
    return true;
  if (n->type == NODE_BINARY && n->op == TOKEN_IMPLIES)
    return false;
  return is_formula_impl_free(n->left) && is_formula_impl_free(n->right);
}

static bool check_nnf_recursive(ASTNode *n, bool under_negation) {
  if (!n)
    return true;
  if (n->type == NODE_UNARY && n->op == TOKEN_NOT) {
    if (under_negation)
      return false;
    if (n->left->type != NODE_ATOM)
      return false;
    return true;
  }
  if (n->type == NODE_QUANTIFIER)
    return check_nnf_recursive(n->left, false);
  if (n->type == NODE_BINARY)
    return check_nnf_recursive(n->left, false) &&
           check_nnf_recursive(n->right, false);
  return true;
}

bool is_formula_nnf(ASTNode *n) { return check_nnf_recursive(n, false); }

bool is_formula_skolemized(ASTNode *n) {
  if (!n)
    return true;
  if (n->type == NODE_QUANTIFIER && n->op == TOKEN_EXISTS)
    return false;
  return is_formula_skolemized(n->left) && is_formula_skolemized(n->right);
}

static bool check_pnf_recursive(ASTNode *n, bool seen_non_quantifier) {
  if (!n)
    return true;
  if (n->type == NODE_QUANTIFIER) {
    if (seen_non_quantifier)
      return false;
    return check_pnf_recursive(n->left, false);
  }
  return check_pnf_recursive(n->left, true) &&
         check_pnf_recursive(n->right, true);
}

bool is_formula_pnf(ASTNode *n) { return check_pnf_recursive(n, false); }

static bool check_rectified_recursive(ASTNode *n, const char **seen_vars,
                                      int *count) {
  if (!n)
    return true;
  if (n->type == NODE_QUANTIFIER) {
    for (int i = 0; i < *count; i++) {
      if (n->name && strcmp(seen_vars[i], n->name) == 0)
        return false;
    }
    if (n->name)
      seen_vars[(*count)++] = n->name;
  }
  return check_rectified_recursive(n->left, seen_vars, count) &&
         check_rectified_recursive(n->right, seen_vars, count);
}

bool is_formula_rectified(ASTNode *n) {
  const char *seen[1024];
  int count = 0;
  return check_rectified_recursive(n, seen, &count);
}
