#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "compiler.h"
#include <stdio.h>

// The main entry point for the parser/transpiler.
// Returns the root of the AST (a list of statements/definitions),
// or NULL on failure.
AstNode* parse(const char* source);

#endif // PARSER_H
