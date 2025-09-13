#include "codegen.h"
#include "ast.h"
#include "symtab.h"
#include "compiler.h"
#include <stdio.h>
#include <string.h>

// --- Code Generator Implementation ---
static void gen_node(AstNode* node, FILE* outfile, CompilerMode mode);

static void gen_binary_op(AstNode* node, FILE* outfile, CompilerMode mode) {
    fprintf(outfile, "(");
    gen_node(node->as.binary_op.left, outfile, mode);
    switch (node->as.binary_op.op) {
        case TOKEN_PLUS:  fprintf(outfile, " + "); break;
        case TOKEN_MINUS: fprintf(outfile, " - "); break;
        case TOKEN_STAR:  fprintf(outfile, " * "); break;
        case TOKEN_SLASH: fprintf(outfile, " / "); break;
        default: break;
    }
    gen_node(node->as.binary_op.right, outfile, mode);
    fprintf(outfile, ")");
}

static void gen_let_decl(AstNode* node, FILE* outfile, CompilerMode mode) {
    const char* type_str = (node->as.let_decl.type == TYPE_INT) ? "int" : "double";
    fprintf(outfile, "    %s %s = ", type_str, node->as.let_decl.name);
    gen_node(node->as.let_decl.initializer, outfile, mode);
    fprintf(outfile, ";\n");
}

static void gen_print_stmt(AstNode* node, FILE* outfile, CompilerMode mode) {
    fprintf(outfile, "    printf(\"%%f\\n\", (double)");
    gen_node(node->as.print_stmt.expression, outfile, mode);
    fprintf(outfile, ");\n");
}

static void gen_block(StmtListNode* list, FILE* outfile, CompilerMode mode) {
    fprintf(outfile, "{\n");
    for (StmtListNode* current = list; current != NULL; current = current->next) {
        gen_node(current->statement, outfile, mode);
    }
    fprintf(outfile, "}\n");
}

static void gen_fn_def(AstNode* node, FILE* outfile, CompilerMode mode) {
    if (strcmp(node->as.fn_def.name, "main") == 0) {
        fprintf(outfile, "int main() ");
    } else {
        fprintf(outfile, "double %s() ", node->as.fn_def.name);
    }
    gen_block(node->as.fn_def.block_stmts, outfile, mode);
}

static void gen_node(AstNode* node, FILE* outfile, CompilerMode mode) {
    if (node == NULL) return;
    switch (node->type) {
        case NODE_LITERAL_FLOAT:
            fprintf(outfile, "%f", node->as.literal.float_val);
            break;
        case NODE_IDENTIFIER:
            fprintf(outfile, "%s", node->as.identifier.name);
            break;
        case NODE_BINARY_OP:
            gen_binary_op(node, outfile, mode);
            break;
        case NODE_LET_DECL:
            gen_let_decl(node, outfile, mode);
            break;
        case NODE_PRINT_STMT:
            gen_print_stmt(node, outfile, mode);
            break;
        case NODE_FN_DEF:
            gen_fn_def(node, outfile, mode);
            break;
        case NODE_BLOCK:
            for(StmtListNode* s = node->as.block_stmts; s != NULL; s = s->next) {
                gen_node(s->statement, outfile, mode);
            }
            break;
        // ... other nodes
        default:
            fprintf(outfile, "// Codegen not implemented for node type %d\n", node->type);
    }
}

void codegen_run(AstNode* ast, FILE* outfile) {
    if (ast == NULL) return;
    fprintf(outfile, "#include <stdio.h>\n\n");
    gen_node(ast, outfile, MODE_RUN);
}

void codegen_header(AstNode* ast, FILE* outfile) {
    if (ast == NULL) return;
    fprintf(outfile, "#ifndef SUPER_LIB_H\n#define SUPER_LIB_H\n\n");
    gen_node(ast, outfile, MODE_BUILD_LIB_HEADER);
    fprintf(outfile, "\n#endif // SUPER_LIB_H\n");
}

void codegen_source(AstNode* ast, FILE* outfile, const char* header_name) {
    if (ast == NULL) return;
    fprintf(outfile, "#include \"%s\"\n\n", header_name);
    gen_node(ast, outfile, MODE_BUILD_LIB_SOURCE);
}
