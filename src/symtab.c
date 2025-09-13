#include "symtab.h"
#include <stdlib.h>
#include <string.h>

static Symbol* symbol_table = NULL;

void add_symbol(const char* name, int len, VarType type) {
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->name = (char*)malloc(len + 1);
    strncpy(new_symbol->name, name, len);
    new_symbol->name[len] = '\0';
    new_symbol->type = type;
    new_symbol->next = symbol_table;
    symbol_table = new_symbol;
}

Symbol* get_symbol(const char* name, int len) {
    for (Symbol* s = symbol_table; s != NULL; s = s->next) {
        if (strlen(s->name) == (size_t)len && strncmp(s->name, name, len) == 0) {
            return s;
        }
    }
    return NULL;
}

void free_symbol_table() {
    Symbol* current = symbol_table;
    while (current != NULL) {
        Symbol* next = current->next;
        free(current->name);
        // A real implementation would free the union data here
        free(current);
        current = next;
    }
    symbol_table = NULL;
}
