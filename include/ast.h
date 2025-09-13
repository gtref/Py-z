#ifndef AST_H
#define AST_H

#include "symtab.h"
#include "lexer.h"

// --- Abstract Syntax Tree Node Definitions ---

typedef enum {
    NODE_LITERAL_FLOAT,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_LET_DECL,
    NODE_PRINT_STMT,
    NODE_BLOCK,
    NODE_FN_DEF,
    NODE_RETURN_STMT,
    NODE_STRUCT_DEF,
    NODE_MEMBER_ACCESS,
    NODE_ARRAY_LITERAL,
    NODE_ARRAY_ACCESS
} AstNodeType;

struct AstNode;
struct StmtListNode;
struct ExprListNode;

typedef struct {
    double float_val;
} LiteralNode;

typedef struct {
    char* name;
} IdentifierNode;

typedef struct {
    TokenType op;
    struct AstNode* left;
    struct AstNode* right;
} BinaryOpNode;

typedef struct {
    TokenType op;
    struct AstNode* right;
} UnaryOpNode;

typedef struct {
    char* name;
    VarType type;
    struct AstNode* initializer;
} LetDeclNode;

typedef struct {
    struct AstNode* expression;
} PrintStmtNode;

typedef struct StmtListNode {
    struct AstNode* statement;
    struct StmtListNode* next;
} StmtListNode;

typedef struct {
    char* name;
    StmtListNode* block_stmts;
} FnDefNode;

typedef struct {
    struct AstNode* expression;
} ReturnStmtNode;

typedef struct {
    char* name;
    StmtListNode* fields; // Using StmtListNode for fields for now
} StructDefNode;

typedef struct {
    struct AstNode* object;
    char* field_name;
} MemberAccessNode;

typedef struct {
    struct ExprListNode* elements;
} ArrayLiteralNode;

typedef struct {
    struct AstNode* array;
    struct AstNode* index;
} ArrayAccessNode;

typedef struct ExprListNode {
    struct AstNode* expression;
    struct ExprListNode* next;
} ExprListNode;


typedef struct AstNode {
    AstNodeType type;
    union {
        LiteralNode      literal;
        IdentifierNode   identifier;
        BinaryOpNode     binary_op;
        UnaryOpNode      unary_op;
        LetDeclNode      let_decl;
        PrintStmtNode    print_stmt;
        StmtListNode*    block_stmts;
        FnDefNode        fn_def;
        ReturnStmtNode   return_stmt;
        StructDefNode    struct_def;
        MemberAccessNode member_access;
        ArrayLiteralNode array_literal;
        ArrayAccessNode  array_access;
    } as;
} AstNode;


// --- AST Management Functions ---
AstNode* create_node(AstNodeType type);
AstNode* create_binary_op(AstNode* left, TokenType op, AstNode* right);
void free_ast(AstNode* node);

#endif // AST_H
