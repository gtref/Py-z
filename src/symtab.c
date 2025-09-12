#include "symtab.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h> // For NULL

static Symbol* symbol_table = NULL;

Symbol* add_symbol(const char* name, int len, VarType type) {
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->name = (char*)malloc(len + 1);
    strncpy(new_symbol->name, name, len);
    new_symbol->name[len] = '\0';
    new_symbol->type = type;
    new_symbol->as.func_sig = NULL;
    new_symbol->as.struct_def = NULL;
    new_symbol->next = symbol_table;
    symbol_table = new_symbol;
    return new_symbol;
}

Symbol* get_symbol(const char* name, int len) {
    for (Symbol* s = symbol_table; s != NULL; s = s->next) {
        if (strlen(s->name) == (size_t)len && strncmp(s->name, name, len) == 0) {
            return s;
        }
    }
    return NULL;
}

void free_func_params(FuncParam* param) {
    while(param != NULL) {
        FuncParam* next = param->next;
        free(param->name);
        free(param);
        param = next;
    }
}

void free_struct_fields(Field* field) {
    while(field != NULL) {
        Field* next = field->next;
        free(field->name);
        free(field);
        field = next;
    }
}

void free_symbol_table() {
    Symbol* current = symbol_table;
    while (current != NULL) {
        Symbol* next = current->next;
        if (current->type == TYPE_FUNCTION && current->as.func_sig != NULL) {
            free_func_params(current->as.func_sig->params);
            free(current->as.func_sig);
        } else if (current->type == TYPE_STRUCT && current->as.struct_def != NULL) {
            free_struct_fields(current->as.struct_def->fields);
            free(current->as.struct_def->name);
            free(current->as.struct_def);
        }
        free(current->name);
        free(current);
        current = next;
    }
    symbol_table = NULL;
}
