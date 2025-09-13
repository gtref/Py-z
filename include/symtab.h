#ifndef SYMTAB_H
#define SYMTAB_H

// --- Symbol Table ---
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_STRUCT,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_UNKNOWN
} VarType;

struct Symbol;
struct Field;

typedef struct {
    VarType element_type;
    int size; // -1 for unknown
} ArrayType;

typedef struct StructDef {
    char* name;
    struct Field* fields;
} StructDef;

typedef struct Symbol {
    char* name;
    VarType type;
    union {
        ArrayType array_type;
        StructDef* struct_def;
        // ... func sig
    } as;
    struct Symbol* next;
} Symbol;

typedef struct Field {
    char* name;
    VarType type;
    struct Field* next;
} Field;


void add_symbol(const char* name, int len, VarType type);
Symbol* get_symbol(const char* name, int len);
void free_symbol_table();

#endif // SYMTAB_H
