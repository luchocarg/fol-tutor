#include <emscripten.h>
#include <string.h>
#include <stdlib.h>
#include "../include/core/lexer.h"
#include "../include/core/ast.h"
#include "../include/core/parser.h"
#include "../include/transform.h"
#include "../include/wasm_plugin.h"

EMSCRIPTEN_KEEPALIVE
int32_t run_cnf_transform(size_t data_len) {
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
        root = transform_remove_implications(root);
        root = transform_to_nnf(root);
 
        transform_reset_alpha_counter();
        transform_alpha_rename(root, NULL);
        root = transform_to_pnf(root);

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