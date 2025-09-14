from .lexer import Token

# --- Abstract Syntax Tree (AST) Node Definitions ---
# Every node in the tree will inherit from this base class.
class AST:
    pass

class Program(AST):
    """The root of the AST, representing the entire program."""
    def __init__(self, statements):
        self.statements = statements

class VarDecl(AST):
    """A variable declaration, e.g., `let x = 10;`"""
    def __init__(self, identifier_token, expression):
        self.identifier = identifier_token
        self.expression = expression

class ExpressionStatement(AST):
    """A statement that is just an expression, e.g., `x + 1;`"""
    def __init__(self, expression):
        self.expression = expression

class PrintStmt(AST):
    """A print statement, e.g., `print("Hello");`"""
    def __init__(self, expression):
        self.expression = expression

class Block(AST):
    """A block of statements, e.g., `{ ... }`"""
    def __init__(self, statements):
        self.statements = statements

class IfStmt(AST):
    """An if statement, e.g., `if (condition) { ... } else { ... }`"""
    def __init__(self, condition, then_branch, else_branch):
        self.condition = condition
        self.then_branch = then_branch
        self.else_branch = else_branch

class WhileStmt(AST):
    """A while loop, e.g., `while (condition) { ... }`"""
    def __init__(self, condition, body):
        self.condition = condition
        self.body = body

class BinaryOp(AST):
    """A binary operation, e.g., `a + b` or `x > 5`."""
    def __init__(self, left, op_token, right):
        self.left = left
        self.op = op_token
        self.right = right

class UnaryOp(AST):
    """A unary operation, e.g., `-5` or `!true`."""
    def __init__(self, op_token, right):
        self.op = op_token
        self.right = right

class Literal(AST):
    """A literal value, e.g., `123`, `"hello"`, `true`."""
    def __init__(self, token):
        self.token = token
        self.value = token.value

class Identifier(AST):
    """An identifier, e.g., a variable name."""
    def __init__(self, token):
        self.token = token
        self.name = token.value

class Assign(AST):
    """An assignment expression, e.g., `x = 10`."""
    def __init__(self, identifier_token, expression):
        self.identifier = identifier_token
        self.expression = expression

class FunctionDecl(AST):
    """A function declaration, e.g., `fn name(p1, p2) { ... }`."""
    def __init__(self, name_token, params, body):
        self.name = name_token
        self.params = params
        self.body = body

class Call(AST):
    """A function call expression, e.g., `name(arg1)`."""
    def __init__(self, callee, paren_token, arguments):
        self.callee = callee
        self.paren = paren_token # The closing parenthesis, for error reporting
        self.arguments = arguments

class ReturnStmt(AST):
    """A return statement, e.g., `return 42;`."""
    def __init__(self, keyword_token, value):
        self.keyword = keyword_token
        self.value = value

# --- Parser ---

class Parser:
    """
    The parser, which takes a list of tokens and builds an AST.
    It uses a recursive descent strategy.
    """
    def __init__(self, tokens):
        self.tokens = [t for t in tokens if t.type != 'COMMENT'] # Filter out comments
        self.pos = 0

    def parse(self):
        """The main entry point for parsing."""
        statements = []
        while not self.is_at_end():
            statements.append(self.declaration())
        return Program(statements)

    # --- Statement Parsers ---

    def declaration(self):
        try:
            if self.match('FN'):
                return self.function_declaration('function')
            if self.match('LET'):
                return self.var_declaration()
            return self.statement()
        except RuntimeError as error:
            # In the future, we can synchronize here to continue parsing.
            # For now, we just propagate the error.
            raise error

    def function_declaration(self, kind):
        name = self.consume('ID', f"Expect {kind} name.")
        self.consume('LPAREN', f"Expect '(' after {kind} name.")
        parameters = []
        if not self.check('RPAREN'):
            while True:
                if len(parameters) >= 255:
                    self.error(self.peek(), "Can't have more than 255 parameters.")
                parameters.append(self.consume('ID', "Expect parameter name."))
                if not self.match('COMMA'):
                    break
        self.consume('RPAREN', "Expect ')' after parameters.")

        self.consume('LBRACE', f"Expect '{{' before {kind} body.")
        body = self.block()
        return FunctionDecl(name, parameters, body)

    def var_declaration(self):
        name = self.consume('ID', "Expect variable name.")
        initializer = None
        if self.match('ASSIGN'):
            initializer = self.expression()

        self.consume('SEMICOLON', "Expect ';' after variable declaration.")
        return VarDecl(name, initializer)

    def statement(self):
        if self.match('PRINT'):
            return self.print_statement()
        if self.match('IF'):
            return self.if_statement()
        if self.match('WHILE'):
            return self.while_statement()
        if self.match('RETURN'):
            return self.return_statement()
        if self.match('LBRACE'):
            return Block(self.block())

        # Default to an expression statement.
        expr = self.expression()
        self.consume('SEMICOLON', "Expect ';' after expression.")
        return ExpressionStatement(expr)

    def print_statement(self):
        # Assumes `print` is followed by an expression, not necessarily in parens.
        # e.g., `print "hello";`
        # For `print(expr)`, the primary() method needs to handle call expressions.
        # Let's keep it simple for now: print(expression);
        self.consume('LPAREN', "Expect '(' after 'print'.")
        expr = self.expression()
        self.consume('RPAREN', "Expect ')' after expression.")
        self.consume('SEMICOLON', "Expect ';' after print statement.")
        return PrintStmt(expr)

    def if_statement(self):
        self.consume('LPAREN', "Expect '(' after 'if'.")
        condition = self.expression()
        self.consume('RPAREN', "Expect ')' after if condition.")

        then_branch = self.statement()
        else_branch = None
        if self.match('ELSE'):
            else_branch = self.statement()

        return IfStmt(condition, then_branch, else_branch)

    def while_statement(self):
        self.consume('LPAREN', "Expect '(' after 'while'.")
        condition = self.expression()
        self.consume('RPAREN', "Expect ')' after while condition.")
        body = self.statement()
        return WhileStmt(condition, body)

    def return_statement(self):
        keyword = self.previous()
        value = None
        if not self.check('SEMICOLON'):
            value = self.expression()
        self.consume('SEMICOLON', "Expect ';' after return value.")
        return ReturnStmt(keyword, value)

    def block(self):
        statements = []
        while not self.check('RBRACE') and not self.is_at_end():
            statements.append(self.declaration())

        self.consume('RBRACE', "Expect '}' after block.")
        return statements

    # --- Expression Parsers (by precedence) ---

    def expression(self):
        return self.assignment()

    def assignment(self):
        expr = self.equality()

        if self.match('ASSIGN'):
            equals = self.previous()
            value = self.assignment() # Allow chained assignment like a = b = 5

            if isinstance(expr, Identifier):
                name = expr.token
                return Assign(name, value)

            # This would be a syntax error like `(a + b) = 5`.
            self.error(equals, "Invalid assignment target.")

        return expr

    def equality(self):
        expr = self.comparison()
        while self.match('NEQ', 'EQ'):
            operator = self.previous()
            right = self.comparison()
            expr = BinaryOp(expr, operator, right)
        return expr

    def comparison(self):
        expr = self.term()
        while self.match('GT', 'GTE', 'LT', 'LTE'):
            operator = self.previous()
            right = self.term()
            expr = BinaryOp(expr, operator, right)
        return expr

    def term(self):
        expr = self.factor()
        while self.match('MINUS', 'PLUS'):
            operator = self.previous()
            right = self.factor()
            expr = BinaryOp(expr, operator, right)
        return expr

    def factor(self):
        expr = self.unary()
        while self.match('DIVIDE', 'MULTIPLY'):
            operator = self.previous()
            right = self.unary()
            expr = BinaryOp(expr, operator, right)
        return expr

    def unary(self):
        if self.match('NOT', 'MINUS'):
            operator = self.previous()
            right = self.unary()
            return UnaryOp(operator, right)
        return self.call()

    def call(self):
        expr = self.primary()

        while True:
            if self.match('LPAREN'):
                expr = self.finish_call(expr)
            else:
                break

        return expr

    def finish_call(self, callee):
        arguments = []
        if not self.check('RPAREN'):
            while True:
                if len(arguments) >= 255:
                    self.error(self.peek(), "Can't have more than 255 arguments.")
                arguments.append(self.expression())
                if not self.match('COMMA'):
                    break

        paren = self.consume('RPAREN', "Expect ')' after arguments.")
        return Call(callee, paren, arguments)

    def primary(self):
        if self.match('TRUE', 'FALSE', 'NIL', 'INTEGER', 'FLOAT', 'STRING'):
            return Literal(self.previous())

        if self.match('ID'):
            return Identifier(self.previous())

        if self.match('LPAREN'):
            expr = self.expression()
            self.consume('RPAREN', "Expect ')' after expression.")
            return expr

        # If no expression matches, it's an error.
        raise self.error(self.peek(), "Expect expression.")

    # --- Helper Methods ---

    def match(self, *types):
        """Checks if the current token is one of the given types. If so, consumes it."""
        for t in types:
            if self.check(t):
                self.advance()
                return True
        return False

    def consume(self, type, message):
        """Checks for a specific token type and consumes it, or raises an error."""
        if self.check(type):
            return self.advance()
        raise self.error(self.peek(), message)

    def check(self, type):
        """Checks the type of the current token without consuming it."""
        if self.is_at_end():
            return False
        return self.peek().type == type

    def advance(self):
        """Consumes the current token and returns it."""
        if not self.is_at_end():
            self.pos += 1
        return self.previous()

    def is_at_end(self):
        return self.peek().type == 'EOF'

    def peek(self):
        return self.tokens[self.pos]

    def previous(self):
        return self.tokens[self.pos - 1]

    def error(self, token, message):
        """Creates a standardized runtime error."""
        if token.type == 'EOF':
            return RuntimeError(f"Parse Error at end: {message}")
        else:
            return RuntimeError(f"Parse Error at '{token.value}' (Line {token.line}): {message}")
