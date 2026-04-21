#include <emscripten.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/core/lexer.h"
#include "../include/core/ast.h"
#include "../include/core/parser.h"
#include "../include/transform.h"
#include "../include/mgu.h"
#include "../include/core/cnf.h"
#include "../include/wasm_plugin.h"

typedef ASTNode* (*TransformStep)(ASTNode*);
typedef void (*OutputFormat)(ASTNode*, char*);

static int32_t run_pipeline(size_t data_len, TransformStep step, OutputFormat format) {
    if (data_len == 0) return 0;

    uint8_t *input_buf = malloc(data_len + 1);
    if (!input_buf) return 1;
    wasm_read_args(input_buf);
    input_buf[data_len] = '\0';

    Lexer l = { .source = (const char*)input_buf, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    Parser p = { .l = &l, .st = st };
    ASTNode* root = parse_formula(&p);

    if (root) {
        if (step) {
            root = step(root);
        }

        char* output_str = calloc(data_len * 8 + 512, sizeof(char));
        if (output_str) {
            format(root, output_str); 
            
            wasm_send_result((uint8_t*)output_str, (size_t)strlen(output_str));
            free(output_str);
        }
        free_ast(root);
    } else {
        const char* error = "Error: Invalid Syntax";
        wasm_send_result((uint8_t*)error, strlen(error));
    }

    free(input_buf);
    free_symbol_table(st);
    return 0;
}

static ASTNode* wrap_alpha(ASTNode* n) {
    transform_reset_alpha_counter();
    transform_alpha_rename(n, NULL);
    return n;
}

static void mgu_output_format(ASTNode* root, char* output_str) {
    if (root && root->type == NODE_BINARY && root->op == TOKEN_AND) {
        Literal* l1 = extract_literal_from_node(root->left);
        Literal* l2 = extract_literal_from_node(root->right);

        if (l1 && l2) {
            calculate_mgu_string(l1, l2, output_str);
        } else {
            strcpy(output_str, "Error: invalid literals.");
        }

        free_literal(l1);
        free_literal(l2);
    } else {
        strcpy(output_str, "Error: expected 'Literal1 ∧ Literal2'");
    }
}


EMSCRIPTEN_KEEPALIVE
int32_t run_remove_implications(size_t data_len) {
    return run_pipeline(data_len, transform_remove_implications, ast_to_formula);
}


EMSCRIPTEN_KEEPALIVE
int32_t run_nnf(size_t data_len) {
    return run_pipeline(data_len, transform_to_nnf, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_pnf(size_t data_len) {
    return run_pipeline(data_len, transform_to_pnf, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_alpha(size_t data_len) {
    return run_pipeline(data_len, wrap_alpha, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_skolem(size_t data_len) {
    return run_pipeline(data_len, transform_skolemize, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_push_universals(size_t data_len) {
    return run_pipeline(data_len, transform_push_universals, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_distribute(size_t data_len) {
    return run_pipeline(data_len, transform_distribute, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_to_sets(size_t data_len) {
    return run_pipeline(data_len, NULL, ast_to_cnf_sets);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_calculate_mgu(size_t data_len) {
    return run_pipeline(data_len, NULL, mgu_output_format);
}

static void resolution_output_format(ASTNode* root, char* output_str) {
    if (root && root->type == NODE_BINARY && root->op == TOKEN_AND) {
        ClauseSet* set = ast_to_clause_set(root);
        if (set->count < 2) {
            strcpy(output_str, "Error: Two clauses are required.");
            free_clause_set(set);
            return;
        }

        Clause* c1 = set->clauses[0];
        Clause* c2 = set->clauses[1];
        bool found = false;

        for (int i = 0; i < c1->count && !found; i++) {
            for (int j = 0; j < c2->count && !found; j++) {
                Literal* l1 = c1->literals[i];
                Literal* l2 = c2->literals[j];

                if (l1->is_negative != l2->is_negative) {
                    bool mgu_success;
                    Substitution* sigma = calculate_mgu(l1, l2, &mgu_success);

                    if (mgu_success) {
                        Clause* res = create_resolvent(c1, i, c2, j, sigma);
                        clause_to_formula(res, output_str);
                        free_clause(res);
                        free_substitution(sigma);
                        found = true;
                    }
                }
            }
        }

        if (!found) strcpy(output_str, "No solution has been found.");
        free_clause_set(set);
    }
}

EMSCRIPTEN_KEEPALIVE
int32_t run_resolve(size_t data_len) {
    return run_pipeline(data_len, NULL, resolution_output_format);
}