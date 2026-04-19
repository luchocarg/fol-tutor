#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef enum {
    SYM_PREDICATE,
    SYM_FUNCTION
} SymbolType;

typedef struct Symbol {
    char* name;
    int arity;
    SymbolType type;
    struct Symbol* next;
} Symbol;

typedef struct {
    Symbol* head;
} SymbolTable;

SymbolTable* create_symbol_table(void); 
int validate_or_register(SymbolTable* table, const char* name, int arity, SymbolType type);
void free_symbol_table(SymbolTable* table);

#endif