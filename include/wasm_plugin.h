#ifndef WASM_PLUGIN_H
#define WASM_PLUGIN_H

#include <stdint.h>
#include <stddef.h>

#define WASM_EXPORT __attribute__((visibility("default")))
#define WASM_IMPORT(name) __attribute__((import_module("typst_env"), import_name(name))) extern

WASM_IMPORT("wasm_minimal_protocol_send_result_to_host")
void wasm_send_result(const uint8_t *ptr, size_t len);

WASM_IMPORT("wasm_minimal_protocol_write_args_to_buffer")
void wasm_read_args(uint8_t *ptr);

WASM_EXPORT int32_t run_cnf_transform(size_t arg_len);
WASM_EXPORT int32_t run_calculate_mgu_trace(size_t arg_len);
WASM_EXPORT int32_t run_user_factor(size_t arg_len);
WASM_EXPORT int32_t run_auto_resolve(size_t arg_len);

#endif