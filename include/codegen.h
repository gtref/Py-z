#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

// Traverses the AST and generates C code for a runnable program.
void codegen_run(AstNode* ast, FILE* outfile);

// Traverses the AST and generates a C header file for a library.
void codegen_header(AstNode* ast, FILE* outfile);

// Traverses the AST and generates a C source file for a library.
void codegen_source(AstNode* ast, FILE* outfile, const char* header_name);

#endif // CODEGEN_H
