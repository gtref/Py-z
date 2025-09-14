import re

class Token:
    """A class to represent a token, containing its type, value (lexeme), line, and column."""
    def __init__(self, type, value, line, column):
        self.type = type
        self.value = value
        self.line = line
        self.column = column

    def __repr__(self):
        return f"Token({self.type}, {repr(self.value)}, L{self.line}:C{self.column})"

# Regex patterns for each token type.
# The order is important, as `re` will match the first pattern it can.
# Keywords are listed before the general ID.
TOKEN_REGEX_PATTERNS = [
    ('COMMENT',   r'//.*|#.*'), # Support for both // and # comments
    ('FLOAT',     r'\d+\.\d+'),
    ('INTEGER',   r'\d+'),
    ('STRING',    r'"(?:\\.|[^"\\])*"'),

    # Keywords
    ('LET',       r'let'),
    ('FN',        r'fn'),
    ('IF',        r'if'),
    ('ELSE',      r'else'),
    ('WHILE',     r'while'),
    ('RETURN',    r'return'),
    ('TRUE',      r'true'),
    ('FALSE',     r'false'),
    ('NIL',       r'nil'),
    ('PRINT',     r'print'), # Added for built-in function

    # Identifier
    ('ID',        r'[a-zA-Z_][a-zA-Z0-9_]*'),

    # Operators and Delimiters
    ('LPAREN',    r'\('),
    ('RPAREN',    r'\)'),
    ('LBRACE',    r'\{'),
    ('RBRACE',    r'\}'),
    ('LBRACKET',  r'\['),
    ('RBRACKET',  r'\]'),
    ('COMMA',     r','),
    ('DOT',       r'\.'),
    ('PLUS',      r'\+'),
    ('MINUS',     r'-'),
    ('MULTIPLY',  r'\*'),
    ('DIVIDE',    r'/'),
    ('ASSIGN',    r'='),
    ('EQ',        r'=='),
    ('NEQ',       r'!='),
    ('LT',        r'<'),
    ('GT',        r'>'),
    ('LTE',       r'<='),
    ('GTE',       r'>='),
    ('AND',       r'&&'),
    ('OR',        r'\|\|'),
    ('NOT',       r'!'),
    ('COLON',     r':'),
    ('SEMICOLON', r';'),

    # Special tokens
    ('NEWLINE',   r'\n'),
    ('SKIP',      r'[ \t\r]+'),
    ('MISMATCH',  r'.'),
]

# The master regex that combines all token patterns.
MASTER_REGEX = '|'.join(f'(?P<{name}>{pattern})' for name, pattern in TOKEN_REGEX_PATTERNS)

class Lexer:
    """The lexer, responsible for turning source code into a stream of tokens."""
    def __init__(self, source_code):
        self.source_code = source_code
        self.tokens = []
        self.line = 1
        self.column = 1

    def tokenize(self):
        """Performs the tokenization of the source code."""
        for mo in re.finditer(MASTER_REGEX, self.source_code):
            kind = mo.lastgroup
            value = mo.group()

            if kind == 'NEWLINE':
                self.line += 1
                self.column = 1
                continue # Don't add newline tokens for now
            elif kind in ('SKIP', 'COMMENT'):
                self.column += len(value)
                continue
            elif kind == 'MISMATCH':
                # This should be a syntax error in the parser, but we can catch it here.
                raise RuntimeError(f"Unexpected character: '{value}' at line {self.line}, column {self.column}")

            # Create a token, handling special cases like stripping quotes from strings.
            if kind == 'STRING':
                value = value[1:-1]

            token = Token(kind, value, self.line, self.column)
            self.tokens.append(token)

            self.column += len(value)

        # Add a final 'End Of File' token to signal the end of the input.
        self.tokens.append(Token('EOF', None, self.line, self.column))
        return self.tokens

# Example usage for testing the lexer directly.
if __name__ == '__main__':
    code = """
let name = "Jules";
let age = 1; // Almost
if (age >= 1) {
    print("Hello, " + name);
}
"""
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    for token in tokens:
        print(token)
