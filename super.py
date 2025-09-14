import sys
import os

# This ensures that we can run the script from the root directory
# and it can find the 'src' module.
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

from src.lexer import Lexer
from src.parser import Parser
from src.interpreter import Interpreter

# Global interpreter instance for the REPL to maintain state.
repl_interpreter = Interpreter()
had_error = False

def run(source_code, is_repl=False):
    """Core function to run a piece of Super code."""
    global had_error
    try:
        lexer = Lexer(source_code)
        tokens = lexer.tokenize()

        parser = Parser(tokens)
        ast = parser.parse()

        # If there was a parse error, the parser would have raised an exception.
        # We can now interpret.
        interpreter = repl_interpreter if is_repl else Interpreter()
        interpreter.interpret(ast)

    except RuntimeError as e:
        print(e, file=sys.stderr)
        had_error = True


def run_file(filepath):
    """Execute code from a file."""
    global had_error
    had_error = False
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source = f.read()
        run(source)
        if had_error:
            sys.exit(65) # Exit with a code indicating data error
    except FileNotFoundError:
        print(f"Error: Cannot find file '{filepath}'", file=sys.stderr)
        sys.exit(1)


def run_prompt():
    """Run an interactive Read-Eval-Print Loop (REPL)."""
    global had_error
    print("Super Language REPL (v0.2.0)")
    print("Enter 'exit()' or press Ctrl+D to exit.")

    while True:
        try:
            line = input("> ")
            if not line:
                continue
            if line.strip().lower() in ['exit', 'exit()']:
                break

            had_error = False # Reset error flag for each line
            run(line, is_repl=True)

        except EOFError:
            print("\nExiting REPL.")
            break
        except KeyboardInterrupt:
            print("\nInterrupted. Use 'exit()' or Ctrl+D to exit.")


if __name__ == "__main__":
    if len(sys.argv) > 2:
        print("Usage: python super.py [script_path]", file=sys.stderr)
        sys.exit(64) # Exit with a code indicating command line usage error
    elif len(sys.argv) == 2:
        run_file(sys.argv[1])
    else:
        run_prompt()
