#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "core/symbol_table.h"

SymbolTable* create_symbol_table(void) {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!table) return NULL;
    table->head = NULL;
    return table;
}

int validate_or_register(SymbolTable* table, const char* name, int arity, SymbolType type) {
    if (!table || !name) return 0;

    Symbol* curr = table->head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return (curr->type == type && curr->arity == arity);
        }
        curr = curr->next;
    }

    Symbol* new_sym = (Symbol*)malloc(sizeof(Symbol));
    if (!new_sym) return 0;

    new_sym->name = strdup(name);
    new_sym->arity = arity;
    new_sym->type = type;
    new_sym->next = table->head;
    table->head = new_sym;
    return 1;
}

void free_symbol_table(SymbolTable* table) {
    if (!table) return;
    Symbol* curr = table->head;
    while (curr) {
        Symbol* next = curr->next;
        free(curr->name);
        free(curr);
        curr = next;
    }
    free(table);
}