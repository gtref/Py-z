#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

typedef enum {
    MODE_RUN,
    MODE_BUILD_LIB_HEADER,
    MODE_BUILD_LIB_SOURCE,
    MODE_CHECK,
    MODE_REPL
} CompilerMode;

// The main entry point for the parser/transpiler.
// Returns 0 on success, 1 on failure.
int transpile(const char* source, FILE* output_file, CompilerMode mode);
int transpile_repl(const char* source);

#endif // PARSER_H
