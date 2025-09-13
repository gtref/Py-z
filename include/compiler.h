#ifndef COMPILER_H
#define COMPILER_H

typedef enum {
    MODE_RUN,
    MODE_BUILD_LIB_HEADER,
    MODE_BUILD_LIB_SOURCE,
    MODE_CHECK,
    MODE_REPL
} CompilerMode;

#endif // COMPILER_H
