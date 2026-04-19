#include "test_helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void term_to_sexpr(Term* t, char* buf) {
    if (!t) return;

    if (t->type == TERM_FUNCTION && t->arity > 0) {
        strcat(buf, "(");
        strcat(buf, t->name);
        for (int i = 0; i < t->arity; i++) {
            strcat(buf, " ");
            term_to_sexpr(t->args[i], buf);
        }
        strcat(buf, ")");
    } else {
        strcat(buf, t->name);
    }
}

void ast_to_sexpr(ASTNode* n, char* buf) {
    if (!n) return;
    switch (n->type) {
        case NODE_PREDICATE:
            if (n->term_count > 0) {
                strcat(buf, "(");
            }
            
            strcat(buf, n->name);
            
            for (int i = 0; i < n->term_count; i++) {
                strcat(buf, " ");
                term_to_sexpr(n->terms[i], buf);
            }
            
            if (n->term_count > 0) {
                strcat(buf, ")");
            }
            break;
        case NODE_QUANTIFIER:
            sprintf(buf + strlen(buf), "(%s %s ", 
                    n->op == TOKEN_FORALL ? "∀" : "∃", n->name);
            ast_to_sexpr(n->left, buf);
            strcat(buf, ")");
            break;
        case NODE_UNARY:
            strcat(buf, "(¬ ");
            ast_to_sexpr(n->left, buf);
            strcat(buf, ")");
            break;
        case NODE_BINARY:
            strcat(buf, "(");
            const char* op = (n->op == TOKEN_AND) ? "∧" : 
                             (n->op == TOKEN_OR ? "∨" : "⇒");
            strcat(buf, op);
            strcat(buf, " ");
            ast_to_sexpr(n->left, buf);
            strcat(buf, " ");
            ast_to_sexpr(n->right, buf);
            strcat(buf, ")");
            break;
        case NODE_FALSUM:
            strcat(buf, "⊥");
            break;
        default: break;
    }
}

void assert_ast(ASTNode* node, const char* expected_sexpr) {
    if (node == NULL) {
        fprintf(stderr, "\n[FAIL] AST is NULL (Parser returned NULL)\n");
        exit(1);
    }
    char obtained[2048] = "";
    obtained[0] = '\0'; 
    ast_to_sexpr(node, obtained);
        if (strcmp(obtained, expected_sexpr) != 0) {
        fprintf(stderr, "\n[FAIL] AST Mismatch\nExpected: %s\nObtained: %s\n", expected_sexpr, obtained);
        exit(1);
    }
}