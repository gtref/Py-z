# Super

**Super** is an educational, statically-typed, compiled programming language that transpiles to C. Its syntax is inspired by modern languages like Rust and Go, with a special "Gen Alpha Slang" mode for fun.

## Philosophy

*   **Simplicity & Power:** Aims for a clean, modern syntax without sacrificing expressive power.
*   **Education:** Serves as a clear example of how a compiler, build system, and CLI tool are built.

## Language Syntax

The `super` CLI transpiles a `.super` file into C code, which is then compiled with `gcc`.

### Types

Super has five primitive types: `int`, `string`, `bool`, user-defined `struct`s, and `array`s.

### Comments

Super supports single-line comments (`//`) and multi-line block comments (`/* ... */`).

### `let` (Variable Declaration)

Variables are declared with `let` and must be explicitly typed. Statements end with a semicolon.

```super
let x: int = 10;
let message: string = "Hello!";
```

### `fn` (Function Definition)

Functions are declared with `fn` and require full type annotations.

```super
fn add(a: int, b: int) -> int {
    return a + b;
}
```

### `struct` (Struct Definition)

User-defined types can be created with `struct`.

```super
struct Point {
    x: int;
    y: int;
}
```

### Arrays

Arrays are ordered lists of elements of the same type.

```super
let my_list: [int] = [10, 20, 30];
let first: int = my_list[0];
print(first); // Prints 10
```

### Control Flow (`if`/`else`, `while`)

Super supports `if/else` and `while` loops.

### Operators

Super supports standard arithmetic, comparison, equality, and logical operators.

### `import`

The `import` statement includes a `superlib` library.

## "Gen Alpha Slang" Mode

Activate slang mode with the `--slang` flag. This changes the language's keywords and CLI commands.
(See previous documentation for keyword mappings)

## CLI Commands

*   `super <file>`: Transpile and run a program.
*   `super init <project>` or `super rizz <project>`: Create a new project.
*   `super build-lib <file>` or `super cook <file>`: Build a library.
*   `super check <file>`: Check the syntax of a file without running.
*   `super repl`: Start the interactive Read-Eval-Print Loop.
*   `super --help`: Show help.
*   `super --show-tokens <file>`: Display tokens from a file.
*   `super --slang ...`: Use slang mode for any command.
