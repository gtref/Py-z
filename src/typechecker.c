#include "typechecker.h"
#include "ast.h"
#include "symtab.h"
#include <stdio.h>
#include <string.h>

// --- Type Checker Implementation ---
static VarType visit(AstNode* node);

static VarType visit_binary_op(AstNode* node) {
    VarType left_type = visit(node->as.binary_op.left);
    VarType right_type = visit(node->as.binary_op.right);
    if (left_type != right_type) {
        printf("Type error: type mismatch in binary operation.\n");
        return TYPE_UNKNOWN;
    }
    if (left_type != TYPE_INT && left_type != TYPE_FLOAT) {
        printf("Type error: binary operations only supported for numbers.\n");
        return TYPE_UNKNOWN;
    }
    return left_type;
}

static VarType visit_let_decl(AstNode* node) {
    VarType initializer_type = visit(node->as.let_decl.initializer);
    if (node->as.let_decl.type != initializer_type) {
        printf("Type error: initializer type does not match declared type for '%s'.\n", node->as.let_decl.name);
        return TYPE_UNKNOWN;
    }
    add_symbol(node->as.let_decl.name, strlen(node->as.let_decl.name), node->as.let_decl.type);
    return node->as.let_decl.type;
}

static VarType visit_identifier(AstNode* node) {
    Symbol* symbol = get_symbol(node->as.identifier.name, strlen(node->as.identifier.name));
    if (symbol == NULL) {
        printf("Type error: undefined variable '%s'.\n", node->as.identifier.name);
        return TYPE_UNKNOWN;
    }
    return symbol->type;
}

static VarType visit_print_stmt(AstNode* node) {
    visit(node->as.print_stmt.expression);
    return TYPE_INT;
}

static VarType visit_return_stmt(AstNode* node) {
    return visit(node->as.return_stmt.expression);
}

static VarType visit_block(StmtListNode* list) {
    for (StmtListNode* current = list; current != NULL; current = current->next) {
        visit(current->statement);
    }
    return TYPE_INT;
}

static VarType visit_fn_def(AstNode* node) {
    add_symbol(node->as.fn_def.name, strlen(node->as.fn_def.name), TYPE_FUNCTION);
    visit_block(node->as.fn_def.block_stmts);
    return TYPE_FUNCTION;
}

static VarType visit(AstNode* node) {
    if (node == NULL) return TYPE_UNKNOWN;
    switch (node->type) {
        case NODE_LITERAL_FLOAT: return TYPE_FLOAT;
        case NODE_BINARY_OP:     return visit_binary_op(node);
        case NODE_LET_DECL:      return visit_let_decl(node);
        case NODE_IDENTIFIER:    return visit_identifier(node);
        case NODE_PRINT_STMT:    return visit_print_stmt(node);
        case NODE_FN_DEF:        return visit_fn_def(node);
        case NODE_BLOCK:         return visit_block(node->as.block_stmts);
        case NODE_RETURN_STMT:   return visit_return_stmt(node);
        default: return TYPE_UNKNOWN;
    }
}

int typecheck(AstNode* ast) {
    if (ast == NULL) return 1;
    VarType result_type = visit(ast);
    free_symbol_table();
    return (result_type == TYPE_UNKNOWN) ? 1 : 0;
}
