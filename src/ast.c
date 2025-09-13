#include "ast.h"
#include <stdlib.h>

// --- AST Management Functions ---
AstNode* create_node(AstNodeType type) {
    AstNode* node = (AstNode*)malloc(sizeof(AstNode));
    node->type = type;
    return node;
}

AstNode* create_binary_op(AstNode* left, TokenType op, AstNode* right) {
    AstNode* node = create_node(NODE_BINARY_OP);
    node->as.binary_op.left = left;
    node->as.binary_op.op = op;
    node->as.binary_op.right = right;
    return node;
}

void free_ast(AstNode* node) {
    if (node == NULL) return;
    // This is a stub. A real implementation would recursively free the tree.
    free(node);
}
