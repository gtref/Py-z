# Super

**Super** is an educational, statically-typed, compiled programming language that transpiles to C. Its syntax is inspired by modern languages like Rust and Go.

## Philosophy

*   **Simplicity & Power:** Aims for a clean, modern syntax without sacrificing expressive power.
*   **Education:** Serves as a clear example of how a compiler, build system, and CLI tool are built.

## Installation

To compile the `super` CLI, simply run `make`:

```bash
make
```

To install the `super` executable to a common system path (e.g., `/usr/local/bin`), run:

```bash
sudo make install
```

You can then uninstall it with:
```bash
sudo make uninstall
```

## Language Syntax

The `super` CLI transpiles a `.super` file into C code, which is then compiled with `gcc`.

(See previous documentation for full syntax details on types, comments, variables, functions, structs, arrays, control flow, and operators.)

## "Gen Alpha Slang" Mode

Super includes a special "slang mode" which provides an alternative, more "online" set of keywords and commands.

For full details, see the [Slang Mode Documentation](SLANG.md).

## CLI Commands

*   `super <file>`: Transpile and run a program.
*   `super init <project>`: Create a new project.
*   `super build-lib <file>`: Build a library.
*   `super check <file>`: Check the syntax of a file.
*   `super repl`: Start the interactive REPL.
*   `super --help`: Show help.
*   `super --show-tokens <file>`: Display tokens from a file.
*   `super --slang ...`: Use slang mode for any command.
