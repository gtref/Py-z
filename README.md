# Super (v1.0-stable)

**Super** is an educational, statically-typed, compiled programming language that transpiles to C. This version represents a stable, working core of the language.

## Core Language Features

This version of Super supports the following features:

*   **Functions:** Only a `fn main() -> int` function is supported as the entry point.
*   **Variables:** `let` declarations for `int` and `float` types.
*   **Printing:** A `print()` function that can print an integer or a float.
*   **Arithmetic:** Basic arithmetic operators (`+`, `-`, `*`, `/`).
*   **Comments:** Single-line (`//`) and multi-line (`/* ... */`) comments.

### Example

```super
fn main() -> int {
    let x: float = 10.5;
    let y: int = 20;

    // Note: Type checking is not yet fully implemented,
    // so mixed-type arithmetic may not work as expected.
    let z: float = x + 2.0;

    print(z);
    print(y);

    return 0;
}
```

## CLI Usage

To compile and run a Super program:
```bash
./super <filename.super>
```
