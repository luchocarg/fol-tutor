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
#include "../include/core/checkers.h"
#include "../include/wasm_plugin.h"
#include <stdio.h>


typedef ASTNode* (*TransformStep)(ASTNode*);
typedef void (*OutputFormat)(ASTNode*, char*);

// Helpers

static char* get_wasm_input(size_t data_len) {
    if (data_len == 0) return NULL;
    uint8_t *input_buf = malloc(data_len + 1);
    if (!input_buf) return NULL;
    wasm_read_args(input_buf);
    input_buf[data_len] = '\0';
    return (char*)input_buf;
}

static void send_wasm_result_str(const char* str) {
    if (str) {
        wasm_send_result((uint8_t*)str, strlen(str));
    }
}

// Core

static char* process_formula_to_string(const char* input_str, TransformStep step, OutputFormat format) {
    if (!input_str) return strdup("Error: Empty input");

    Lexer l = { .source = input_str, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    Parser p = { .l = &l, .st = st, .depth = 0 };
    ASTNode* root = parse_formula(&p);

    char* result_str = NULL;

    if (root) {
        if (step) {
            root = step(root);
        }

        result_str = calloc(strlen(input_str) * 8 + 1024, sizeof(char));
        if (result_str) {
            format(root, result_str);
        }
        free_ast(root);
    } else {
        result_str = malloc(strlen(p.error_msg) + 16);
        sprintf(result_str, "Error: %s", p.error_msg);
    }

    free_symbol_table(st);
    return result_str;
}

// Wrappers

static int32_t run_generic_transform(size_t data_len, TransformStep step, OutputFormat format) {
    char* input = get_wasm_input(data_len);
    char* output = process_formula_to_string(input, step, format);
    send_wasm_result_str(output);
    free(input);
    free(output);
    return 0;
}

static ASTNode* wrap_alpha(ASTNode* n) {
    transform_reset_alpha_counter();
    transform_alpha_rename(n, NULL);
    return n;
}

EMSCRIPTEN_KEEPALIVE
int32_t run_remove_implications(size_t data_len) {
    return run_generic_transform(data_len, transform_remove_implications, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_nnf(size_t data_len) {
    return run_generic_transform(data_len, transform_to_nnf, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_pnf(size_t data_len) {
    return run_generic_transform(data_len, transform_to_pnf, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_alpha(size_t data_len) {
    return run_generic_transform(data_len, wrap_alpha, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_skolem(size_t data_len) {
    return run_generic_transform(data_len, transform_skolemize, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_push_universals(size_t data_len) {
    return run_generic_transform(data_len, transform_push_universals, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_distribute(size_t data_len) {
    return run_generic_transform(data_len, transform_distribute, ast_to_formula);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_to_sets(size_t data_len) {
    return run_generic_transform(data_len, NULL, ast_to_cnf_sets);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_to_json(size_t data_len) {
    return run_generic_transform(data_len, NULL, ast_to_json);
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
int32_t run_calculate_mgu(size_t data_len) {
    return run_generic_transform(data_len, NULL, mgu_output_format);
}

static int32_t run_property_check(size_t data_len, bool (*check_fn)(ASTNode*)) {
    char* input = get_wasm_input(data_len);
    if (!input) return 0;
    
    Lexer l = { .source = input, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    Parser p = { .l = &l, .st = st, .depth = 0 };
    ASTNode* root = parse_formula(&p);
    
    if (!root) {
        char err[300];
        sprintf(err, "Error: %s", p.error_msg);
        send_wasm_result_str(err);
        free_symbol_table(st);
        free(input);
        return 0;
    }

    bool result = check_fn(root);
    send_wasm_result_str(result ? "true" : "false");
    
    free_ast(root);
    free_symbol_table(st);
    free(input);
    return 0;
}

EMSCRIPTEN_KEEPALIVE int32_t is_impl_free(size_t data_len) { return run_property_check(data_len, is_formula_impl_free); }
EMSCRIPTEN_KEEPALIVE int32_t is_skolemized(size_t data_len) { return run_property_check(data_len, is_formula_skolemized); }
EMSCRIPTEN_KEEPALIVE int32_t is_nnf(size_t data_len) { return run_property_check(data_len, is_formula_nnf); }
EMSCRIPTEN_KEEPALIVE int32_t is_pnf(size_t data_len) { return run_property_check(data_len, is_formula_pnf); }
EMSCRIPTEN_KEEPALIVE int32_t is_rectified(size_t data_len) { return run_property_check(data_len, is_formula_rectified); }

EMSCRIPTEN_KEEPALIVE
int32_t run_user_resolve(size_t data_len) {
    char* input = get_wasm_input(data_len);
    if (!input) return 0;

    char* pipe = strchr(input, '|');
    char* p2 = pipe ? strchr(pipe + 1, '|') : NULL;
    if (!pipe || !p2) { send_wasm_result_str("Error: Invalid input format"); free(input); return 0; }
    *pipe = '\0'; *p2 = '\0';
    
    char* formula_str = input;
    char* lit_str = pipe + 1;
    char* subst_str = p2 + 1;
    while(*subst_str == ' ') subst_str++;

    int m1[64], m2[64], l1=0, l2=0;
    if (strcmp(lit_str, "none:none") != 0) {
        char* colon = strchr(lit_str, ':');
        if (colon) {
            *colon = '\0';
            char* t = strtok(lit_str, ","); while(t) { m1[l1++] = atoi(t); t = strtok(NULL, ","); }
            t = strtok(colon + 1, ","); while(t) { m2[l2++] = atoi(t); t = strtok(NULL, ","); }
        }
    }

    Lexer l = { .source = formula_str, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    Parser p = { .l = &l, .st = st, .depth = 0 };
    ASTNode* root = parse_formula(&p);

    if (root && root->type == NODE_BINARY && root->op == TOKEN_AND) {
        ClauseSet* set = ast_to_clause_set(root);
        if (set->count >= 2) {
            Clause *c1 = set->clauses[0], *c2 = set->clauses[1];
            bool auto_mode = (strcmp(subst_str, "auto") == 0);
            if (auto_mode) standardize_apart_clause(c2, c1);
            
            Substitution* sigma = auto_mode ? NULL : parse_substitution_string(subst_str);
            char output_str[2048] = ""; bool found = false;

            if (l1 > 0 && l2 > 0) {
                Literal *lits1[64], *lits2[64];
                for(int m=0; m<l1; m++) lits1[m] = c1->literals[m1[m]];
                for(int m=0; m<l2; m++) lits2[m] = c2->literals[m2[m]];
                
                if (auto_mode) {
                    bool ok = false; sigma = calculate_simultaneous_mgu(lits1, l1, lits2, l2, &ok);
                    if (ok) {
                        Clause* res = create_general_resolvent(c1, m1, l1, c2, m2, l2, sigma);
                        char mb[1024], rb[1024] = "", cb[1024] = "";
                        calculate_simultaneous_mgu_string(sigma, mb); 
                        clause_to_formula_sep(res, " ∨ ", rb); 
                        clause_to_formula_sep(c2, " ∨ ", cb);
                        
                        sprintf(output_str, "%s|%s|%s", mb, rb, cb);
                        
                        free_clause(res); found = true;
                    }
                } else if (sigma) {
                    Clause* res = create_general_resolvent(c1, m1, l1, c2, m2, l2, sigma);
                    clause_to_formula_sep(res, " ∨ ", output_str); 
                    free_clause(res); found = true;
                }
            }
            if (sigma) free_substitution(sigma);
            if (!found) strcpy(output_str, "Error: Resolution failed");
            send_wasm_result_str(output_str);
        }
        free_clause_set(set);
    } else if (!root) {
        char err[300];
        sprintf(err, "Error: %s", p.error_msg);
        send_wasm_result_str(err);
    } else {
        send_wasm_result_str("Error: Input must be a conjunction of two clauses.");
    }

    if (root) free_ast(root);
    free_symbol_table(st);
    free(input);
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int32_t run_calculate_mgu_trace(size_t data_len) {
    char* input = get_wasm_input(data_len);
    if (!input) return 0;
    Lexer l = { .source = input, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    Parser p = { .l = &l, .st = st, .depth = 0 };
    ASTNode* root = parse_formula(&p);
    char output_str[4096] = "";
    if (root && root->type == NODE_BINARY && root->op == TOKEN_AND) {
        Literal* l1 = extract_literal_from_node(root->left);
        Literal* l2 = extract_literal_from_node(root->right);
        if (l1 && l2) calculate_mgu_trace(l1, l2, output_str);
        else strcpy(output_str, "Error: invalid literals.");
        free_literal(l1); free_literal(l2);
    } else if (!root) {
        sprintf(output_str, "Error: %s", p.error_msg);
    } else strcpy(output_str, "Error: expected 'Literal1 ∧ Literal2'");
    send_wasm_result_str(output_str);
    if (root) free_ast(root);
    free_symbol_table(st);
    free(input);
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int32_t run_user_factor(size_t data_len) {
    char* input = get_wasm_input(data_len);
    if (!input) return 0;
    char* pipe = strchr(input, '|');
    if (!pipe) { send_wasm_result_str("Error: expected 'Clause|i1,i2'"); free(input); return 0; }
    *pipe = '\0';
    char* clause_str = input;
    char* indices_str = pipe + 1;
    int i1 = atoi(indices_str);
    char* comma = strchr(indices_str, ',');
    int i2 = comma ? atoi(comma + 1) : i1;
    Lexer l = { .source = clause_str, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    Parser p = { .l = &l, .st = st, .depth = 0 };
    ASTNode* root = parse_formula(&p);
    char output_str[2048] = "";
    if (root) {
        ClauseSet* set = ast_to_clause_set(root);
        if (set->count > 0) {
            Substitution* sigma = NULL;
            Clause* factored = factor_clause(set->clauses[0], i1, i2, &sigma);
            if (factored) {
                char mb[1024] = "", fb[1024] = "";
                calculate_simultaneous_mgu_string(sigma, mb);
                clause_to_formula_sep(factored, " ∨ ", fb);
                sprintf(output_str, "%s|%s", mb, fb);
                free_clause(factored); if (sigma) free_substitution(sigma);
            } else strcpy(output_str, "Error: Factoring failed");
        } else strcpy(output_str, "Error: Empty clause");
        free_clause_set(set);
    } else {
        sprintf(output_str, "Error: %s", p.error_msg);
    }
    send_wasm_result_str(output_str);
    if (root) free_ast(root);
    free_symbol_table(st);
    free(input);
    return 0;
}


EMSCRIPTEN_KEEPALIVE
int32_t run_auto_resolve(size_t data_len) {
    char* input = get_wasm_input(data_len);
    if (!input) return 0;
    Lexer l = { .source = input, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    Parser p = { .l = &l, .st = st };
    ASTNode* root = parse_formula(&p);
    char* output_str = malloc(65536); 
    output_str[0] = '\0';
    if (root) {
        reset_clause_id_counter();
        
        root = transform_remove_implications(root);
        transform_alpha_rename(root, NULL);
        root = transform_to_nnf(root);
        root = transform_to_pnf(root);
        root = transform_skolemize(root);
        root = transform_distribute(root);
        root = transform_push_universals(root);
        
        ClauseSet* set = ast_to_clause_set(root);
        ClauseNode* all_nodes = NULL;
        run_automated_resolution(set, &all_nodes, 200);
        ClauseNode* curr = all_nodes;
        char* op = output_str;
        while (curr) {
            op += sprintf(op, "%d:", curr->c->id);
            clause_to_formula_sep(curr->c, " ∨ ", op);
            op += strlen(op);
            op += sprintf(op, "|%d|%d|%s", 
                curr->c->parent1 ? curr->c->parent1->id : -1,
                curr->c->parent2 ? curr->c->parent2->id : -1,
                curr->mgu_str ? curr->mgu_str : "");
            
            if (curr->next) op += sprintf(op, ";");
            curr = curr->next;
        }
        ClauseNode* cn = all_nodes;
        int initial_count = set->count;
        int node_idx = 0;
        while (cn) {
            ClauseNode* nxt = cn->next;
            if (cn->mgu_str) free(cn->mgu_str);
            if (node_idx >= initial_count) free_clause(cn->c);
            free(cn);
            cn = nxt;
            node_idx++;
        }
        free(set->clauses); free(set);
    } else {
        sprintf(output_str, "Error: %s", p.error_msg);
    }

    send_wasm_result_str(output_str);
    if (root) free_ast(root);
    free_symbol_table(st);
    free(input);
    free(output_str);
    return 0;
}