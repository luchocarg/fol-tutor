#include <emscripten.h>
#include <string.h>
#include <stdlib.h>
#include "../include/core/lexer.h"
#include "../include/core/ast.h"
#include "../include/core/parser.h"
#include "../include/transform.h"
#include "../include/wasm_plugin.h"

typedef ASTNode* (*TransformStep)(ASTNode*);

static int32_t run_pipeline(size_t data_len, TransformStep step) {
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

        char* output_str = calloc(data_len * 4 + 128, sizeof(char));
        if (output_str) {
            ast_to_formula(root, output_str);
            wasm_send_result((uint8_t*)output_str, strlen(output_str));
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

EMSCRIPTEN_KEEPALIVE
int32_t run_nnf(size_t data_len) {
    return run_pipeline(data_len, transform_to_nnf);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_pnf(size_t data_len) {
    return run_pipeline(data_len, transform_to_pnf);
}

static ASTNode* wrap_alpha(ASTNode* n) {
    transform_reset_alpha_counter();
    transform_alpha_rename(n, NULL);
    return n;
}

EMSCRIPTEN_KEEPALIVE
int32_t run_alpha(size_t data_len) {
    return run_pipeline(data_len, wrap_alpha);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_skolem(size_t data_len) {
    return run_pipeline(data_len, transform_skolemize);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_push_universals(size_t data_len) {
    return run_pipeline(data_len, transform_push_universals);
}

EMSCRIPTEN_KEEPALIVE
int32_t run_distribute(size_t data_len) {
    return run_pipeline(data_len, transform_distribute);
}