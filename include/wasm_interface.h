#ifndef WASM_INTERFACE_H
#define WASM_INTERFACE_H

#include <stdint.h>

#define TYPST_PLUGIN __attribute__((visibility("default")))

TYPST_PLUGIN int32_t run_cnf_transform(const uint8_t* data, uint32_t len);

#endif