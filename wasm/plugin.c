#include <emscripten.h>
#include <string.h>
#include <stdlib.h>
#include "../include/lexer.h"
#include "../include/ast.h"
#include "../include/parser.h"
#include "../include/wasm_plugin.h"

#define TYPST_ENV __attribute__((import_module("typst_env")))

extern TYPST_ENV void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
extern TYPST_ENV void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

EMSCRIPTEN_KEEPALIVE
int32_t run_cnf_transform(size_t data_len) {
    if (data_len == 0) return 0;

    // Typst -> C
    uint8_t *input_buf = malloc(data_len + 1);
    if (!input_buf) return 1;
    wasm_read_args(input_buf);
    input_buf[data_len] = '\0';

    // Lexer and Parser
    Lexer l = { .source = (const char*)input_buf, .cursor = 0 };
    SymbolTable* st = create_symbol_table();
    ASTNode* root = parse_formula(&l, st); 

    if (root) {
        // Out buffer x4 input
        char* output_str = calloc(data_len * 4 + 128, sizeof(char));
        if (output_str) {
            output_str[0] = '\0';
            
            // AST -> S-Expression
            ast_to_sexpr(root, output_str);
            
            // C -> Typst
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