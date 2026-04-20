CC           := gcc
STD          := -std=c11
CFLAGS       := $(STD) -Iinclude -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
                -Wstrict-prototypes -fstack-protector-all -g3 -O0

SRC_DIR      := src
TEST_DIR     := tests
OBJ_DIR      := obj
BIN          := testing

SRCS         := $(shell find $(SRC_DIR) -name "*.c")
TEST_SRCS    := $(shell find $(TEST_DIR) -name "*.c")

OBJS         := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TEST_OBJS    := $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_SRCS))

EMCC         := emcc
WASM_DIR     := wasm
WASM_SRC     := $(WASM_DIR)/plugin.c
WASM_OUT     := $(WASM_DIR)/plugin.wasm

EMCC_FLAGS   := -O3 -Iinclude \
                --no-entry \
                -s STANDALONE_WASM \
                -s PURE_WASI=0 \
                -s FILESYSTEM=0 \
                -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
                -s EXPORTED_FUNCTIONS='["_run_cnf_transform", "_malloc", "_free"]' \
                -s ALLOW_MEMORY_GROWTH=0 \
                -s INITIAL_MEMORY=6553600 \
                -s TOTAL_STACK=524288

all: $(BIN)

$(BIN): $(OBJS) $(TEST_OBJS)
	@echo "Enlazando $@..."
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

wasm:
	@mkdir -p $(WASM_DIR)
	$(EMCC) $(EMCC_FLAGS) $(WASM_SRC) $(SRCS) -o $(WASM_OUT)
    
test: all
	./$(BIN)

VALGRIND := valgrind
VFLAGS   := --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1

valgrind: all
	$(VALGRIND) $(VFLAGS) ./$(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN) $(WASM_OUT)

.PHONY: all clean test valgrind wasm