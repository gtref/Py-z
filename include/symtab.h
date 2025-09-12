#ifndef SYMTAB_H
#define SYMTAB_H

// --- Symbol Table ---
typedef enum {
    TYPE_STRING,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRUCT,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_UNKNOWN
} VarType;

// Forward declarations
struct Symbol;
struct StructDef;

typedef struct Field {
    char* name;
    VarType type;
    struct Field* next;
} Field;

typedef struct StructDef {
    char* name;
    Field* fields;
} StructDef;

typedef struct FuncParam {
    char* name;
    VarType type;
    struct FuncParam* next;
} FuncParam;

typedef struct {
    FuncParam* params;
    VarType return_type;
    struct Symbol* parent_symbol; // Back-pointer
} FuncSignature;

typedef struct Symbol {
    char* name;
    VarType type;
    union {
        FuncSignature* func_sig;
        StructDef* struct_def;
    } as;
    struct Symbol* next;
} Symbol;


Symbol* add_symbol(const char* name, int len, VarType type);
Symbol* get_symbol(const char* name, int len);
void free_symbol_table();

#endif // SYMTAB_H
