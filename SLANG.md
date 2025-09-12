# Super Language: "Gen Alpha Slang" Mode

This document describes the "Gen Alpha Slang" mode for the Super programming language. This mode provides an alternative set of keywords and CLI commands for a more fun and "online" coding experience.

## Activation

Activate slang mode by passing the `--slang` flag as the *first* argument to the `super` CLI. For example:

```bash
super --slang my_slang_file.super
super --slang cook my_lib.super
```

## Keyword Mapping

When slang mode is active, the lexer recognizes the following keywords:

| Standard | Slang |
|---|---|
| `fn` | `vibe_check` |
| `let` | `bet` |
| `if` | `sus` |
| `else` | `nah` |
| `while` | `on_repeat` |
| `print` | `spill` |
| `return`| `yeet` |
| `true` | `fax` |
| `false` | `cap` |

## CLI Command Mapping

The following CLI commands are also aliased in slang mode:

| Standard | Slang |
|---|---|
| `init` | `rizz` |
| `build-lib`| `cook` |

## Example

Here is a full example of a program written in Super's slang mode:

```super
# To run this file: super --slang my_file.super

vibe_check main() -> int {
    bet is_based: bool = fax;

    sus is_based {
        spill("This program is based.");
    } nah {
        spill("This program is cringe.");
    }

    yeet 0;
}
```
