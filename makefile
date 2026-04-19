CC           := gcc
STD          := -std=c11
CFLAGS       := $(STD) -Iinclude -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
                -Wstrict-prototypes -fstack-protector-all -g3 -O0

SRC_DIR      := src
TEST_DIR     := tests
OBJ_DIR      := obj
BIN          := testing

SRCS         := $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS    := $(wildcard $(TEST_DIR)/*.c)
OBJS         := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_OBJS    := $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

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
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

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