#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "ast.h"

// Traverses the AST and checks for type errors.
// Returns 0 on success, 1 on failure.
int typecheck(AstNode* ast);

#endif // TYPECHECKER_H
