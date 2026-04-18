#include <emscripten.h>
#include <stdint.h>
#include <stdlib.h>

#define TYPST_ENV __attribute__((import_module("typst_env")))

extern TYPST_ENV void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
extern TYPST_ENV void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

EMSCRIPTEN_KEEPALIVE
int32_t run_cnf_transform(size_t data_len) {
    if (data_len == 0) return 0;

    uint8_t *buffer = (uint8_t *)malloc(data_len);
    if (!buffer) return 1;

    wasm_minimal_protocol_write_args_to_buffer(buffer);
    
    wasm_minimal_protocol_send_result_to_host(buffer, data_len);
    
    free(buffer);
    
    return 0;
}