from .parser import AST, VarDecl, FunctionDecl, ExpressionStatement, PrintStmt, Block, IfStmt, WhileStmt, ReturnStmt, BinaryOp, UnaryOp, Literal, Identifier, Assign, Call
from .lexer import Token

# --- Return Value Exception ---
class Return(Exception):
    """An exception-based way to handle return values, unwinding the stack."""
    def __init__(self, value):
        super().__init__()
        self.value = value

# --- Callable Objects ---
class SuperCallable:
    """Base class for anything that can be called (like functions)."""
    def arity(self):
        """Returns the number of arguments the callable expects."""
        raise NotImplementedError

    def call(self, interpreter, arguments):
        """Executes the callable."""
        raise NotImplementedError

class SuperFunction(SuperCallable):
    """Represents a user-defined function in Super."""
    def __init__(self, declaration, closure):
        self.declaration = declaration
        self.closure = closure

    def arity(self):
        return len(self.declaration.params)

    def call(self, interpreter, arguments):
        # Create a new environment for the function's body.
        # It's enclosed by the function's original environment (the closure).
        environment = Environment(self.closure)
        for i, param in enumerate(self.declaration.params):
            environment.define(param.value, arguments[i])

        try:
            # Execute the function body in the new environment.
            interpreter.execute_block(self.declaration.body, environment)
        except Return as return_value:
            # If a return statement was hit, catch it and return its value.
            return return_value.value

        # If the function doesn't explicitly return, it returns nil.
        return None

    def __str__(self):
        return f"<fn {self.declaration.name.value}>"


class Environment:
    """Manages variable scopes and their values."""
    def __init__(self, enclosing=None):
        self.values = {}
        self.enclosing = enclosing

    def define(self, name, value):
        """Define a new variable in the current scope."""
        self.values[name] = value

    def get(self, name_token):
        """Get a variable's value, searching up through enclosing scopes."""
        name = name_token.value
        if name in self.values:
            return self.values[name]
        if self.enclosing is not None:
            return self.enclosing.get(name_token)
        raise RuntimeError(f"Runtime Error: Undefined variable '{name}' at line {name_token.line}.")

    def assign(self, name_token, value):
        """Assign a new value to an existing variable."""
        name = name_token.value
        if name in self.values:
            self.values[name] = value
            return
        if self.enclosing is not None:
            self.enclosing.assign(name_token, value)
            return
        raise RuntimeError(f"Runtime Error: Undefined variable '{name}' at line {name_token.line}.")


class Interpreter:
    """
    Walks the AST and executes the code.
    This class uses the Visitor pattern (`visit_*` methods).
    """
    def __init__(self):
        self.environment = Environment()

    def interpret(self, program):
        """The main entry point for the interpreter."""
        try:
            for statement in program.statements:
                self.execute(statement)
        except RuntimeError as e:
            # Re-raise for the main runner to catch and display.
            raise e

    def execute(self, stmt):
        """Dispatches to the correct visit method for a statement."""
        method_name = f'visit_{stmt.__class__.__name__.lower()}'
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(stmt)

    def evaluate(self, expr):
        """Dispatches to the correct visit method for an expression."""
        method_name = f'visit_{expr.__class__.__name__.lower()}'
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(expr)

    def generic_visit(self, node):
        raise NotImplementedError(f"No visit method for node type: {node.__class__.__name__}")

    # --- Statement Visitors ---

    def visit_expressionstatement(self, stmt):
        self.evaluate(stmt.expression)
        return None

    def visit_vardecl(self, stmt):
        value = None
        if stmt.expression is not None:
            value = self.evaluate(stmt.expression)
        self.environment.define(stmt.identifier.value, value)
        return None

    def visit_printstmt(self, stmt):
        value = self.evaluate(stmt.expression)
        # Simple Python print. We can format it nicely.
        if value is None:
            print("nil")
        elif isinstance(value, bool):
            print(str(value).lower())
        else:
            print(value)
        return None

    def visit_block(self, stmt):
        self.execute_block(stmt.statements, Environment(self.environment))
        return None

    def execute_block(self, statements, environment):
        previous_env = self.environment
        try:
            self.environment = environment
            for statement in statements:
                self.execute(statement)
        finally:
            self.environment = previous_env

    def visit_ifstmt(self, stmt):
        if bool(self.evaluate(stmt.condition)):
            self.execute(stmt.then_branch)
        elif stmt.else_branch is not None:
            self.execute(stmt.else_branch)
        return None

    def visit_whilestmt(self, stmt):
        while bool(self.evaluate(stmt.condition)):
            self.execute(stmt.body)
        return None

    def visit_functiondecl(self, stmt):
        function = SuperFunction(stmt, self.environment)
        self.environment.define(stmt.name.value, function)
        return None

    def visit_returnstmt(self, stmt):
        value = None
        if stmt.value is not None:
            value = self.evaluate(stmt.value)
        raise Return(value)


    # --- Expression Visitors ---

    def visit_call(self, expr):
        callee = self.evaluate(expr.callee)

        arguments = []
        for argument in expr.arguments:
            arguments.append(self.evaluate(argument))

        if not isinstance(callee, SuperCallable):
            raise RuntimeError(f"Can only call functions and classes at line {expr.paren.line}.")

        if len(arguments) != callee.arity():
            raise RuntimeError(f"Expected {callee.arity()} arguments but got {len(arguments)} at line {expr.paren.line}.")

        return callee.call(self, arguments)

    def visit_assign(self, expr):
        value = self.evaluate(expr.expression)
        self.environment.assign(expr.identifier, value)
        return value

    def visit_literal(self, expr):
        val_str = expr.value
        if expr.token.type == 'INTEGER':
            return int(val_str)
        if expr.token.type == 'FLOAT':
            return float(val_str)
        if expr.token.type == 'STRING':
            return str(val_str)
        if expr.token.type == 'TRUE':
            return True
        if expr.token.type == 'FALSE':
            return False
        if expr.token.type == 'NIL':
            return None
        return val_str

    def visit_identifier(self, expr):
        return self.environment.get(expr.token)

    def visit_unaryop(self, expr):
        right = self.evaluate(expr.right)
        op_type = expr.op.type

        if op_type == 'MINUS':
            if not isinstance(right, (int, float)):
                raise RuntimeError(f"Operand for '-' must be a number, got {type(right)} at line {expr.op.line}.")
            return -right
        if op_type == 'NOT':
            return not bool(right)

        return None # Should be unreachable

    def visit_binaryop(self, expr):
        left = self.evaluate(expr.left)
        right = self.evaluate(expr.right)
        op_type = expr.op.type

        # Arithmetic operations
        if op_type in ('MINUS', 'MULTIPLY', 'DIVIDE'):
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise RuntimeError(f"Operands for '{expr.op.value}' must be numbers at line {expr.op.line}.")
            if op_type == 'MINUS': return left - right
            if op_type == 'MULTIPLY': return left * right
            if op_type == 'DIVIDE':
                if right == 0:
                    raise RuntimeError(f"Division by zero at line {expr.op.line}.")
                return left / right

        # Addition can be for numbers or string concatenation
        if op_type == 'PLUS':
            if isinstance(left, (int, float)) and isinstance(right, (int, float)):
                return left + right
            if isinstance(left, str) and isinstance(right, str):
                return left + right
            raise RuntimeError(f"Operands for '+' must be two numbers or two strings at line {expr.op.line}.")

        # Comparison operators
        if op_type == 'GT': return left > right
        if op_type == 'GTE': return left >= right
        if op_type == 'LT': return left < right
        if op_type == 'LTE': return left <= right
        if op_type == 'EQ': return left == right
        if op_type == 'NEQ': return left != right

        return None # Should be unreachable
