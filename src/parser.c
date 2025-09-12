#include "parser.h"
#include "lexer.h"
#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Parser/Transpiler Implementation ---
typedef struct { Token current; Token previous; FILE* output_file; int had_error; int panic_mode; } Parser;
static Parser parser;

static void error_at(Token* token, const char* message);
static void synchronize();
static void advance_parser() { parser.previous = parser.current; for (;;) { parser.current = get_next_token(); if (parser.current.type != TOKEN_UNKNOWN) break; error_at(&parser.current, parser.current.start); } }
static void consume(TokenType type, const char* message) { if (parser.current.type == type) { advance_parser(); return; } error_at(&parser.current, message); }
static int match(TokenType type) { if (parser.current.type == type) { advance_parser(); return 1; } return 0; }

static const char* type_to_c_str(VarType type) { if (type == TYPE_INT) return "int"; if (type == TYPE_FLOAT) return "double"; if (type == TYPE_BOOL) return "int"; if (type == TYPE_STRING) return "const char*"; return "void"; }
static VarType parse_type() {
    if (match(TOKEN_LSQUARE)) {
        parse_type();
        consume(TOKEN_RSQUARE, "Expect ']' after array type.");
        return TYPE_ARRAY;
    }
    if (match(TOKEN_INT)) return TYPE_INT;
    if (match(TOKEN_STRING_TYPE)) return TYPE_STRING;
    if (match(TOKEN_BOOL_TYPE)) return TYPE_BOOL;
    if (match(TOKEN_FLOAT_TYPE)) return TYPE_FLOAT;
    if (match(TOKEN_IDENTIFIER)) { Symbol* s = get_symbol(parser.previous.start, parser.previous.length); if (s && s->type == TYPE_STRUCT) return TYPE_STRUCT; }
    error_at(&parser.current, "Expect type annotation.");
    return TYPE_UNKNOWN;
}

static void parse_expression();
static void parse_primary() {
    if (match(TOKEN_NUMBER)) { fprintf(parser.output_file, "%.*s", parser.previous.length, parser.previous.start); }
    else if (match(TOKEN_STRING)) { fprintf(parser.output_file, "%.*s", parser.previous.length, parser.previous.start); }
    else if (match(TOKEN_TRUE)) { fprintf(parser.output_file, "1"); }
    else if (match(TOKEN_FALSE)) { fprintf(parser.output_file, "0"); }
    else if (match(TOKEN_LSQUARE)) {
        fprintf(parser.output_file, "{");
        if (!match(TOKEN_RSQUARE)) {
            do { parse_expression(); } while (match(TOKEN_COMMA));
            consume(TOKEN_RSQUARE, "Expect ']' after array elements.");
        }
        fprintf(parser.output_file, "}");
    }
    else if (match(TOKEN_IDENTIFIER)) {
        // ...
    }
    else if (match(TOKEN_LPAREN)) { fprintf(parser.output_file, "("); parse_expression(); fprintf(parser.output_file, ")"); consume(TOKEN_RPAREN, "Expect ')' after expression."); }
    else { error_at(&parser.current, "Expect expression."); }
}
static void parse_call() { parse_primary(); while (match(TOKEN_DOT) || match(TOKEN_LSQUARE)) { if (parser.previous.type == TOKEN_DOT) { fprintf(parser.output_file, "."); consume(TOKEN_IDENTIFIER, "Expect property name after '.'."); fprintf(parser.output_file, "%.*s", parser.previous.length, parser.previous.start); } else { fprintf(parser.output_file, "["); parse_expression(); consume(TOKEN_RSQUARE, "Expect ']' after index."); fprintf(parser.output_file, "]"); } } }
static void parse_factor() { parse_call(); while (match(TOKEN_STAR) || match(TOKEN_SLASH)) { Token op = parser.previous; fprintf(parser.output_file, " %.*s ", op.length, op.start); parse_call(); } }
static void parse_term() { parse_factor(); while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) { Token op = parser.previous; fprintf(parser.output_file, " %.*s ", op.length, op.start); parse_factor(); } }
static void parse_comparison() { parse_term(); while (match(TOKEN_GT) || match(TOKEN_LT)) { Token op = parser.previous; fprintf(parser.output_file, " %.*s ", op.length, op.start); parse_term(); } }
static void parse_equality() { parse_comparison(); while (match(TOKEN_BANG_EQ) || match(TOKEN_EQ_EQ)) { Token op = parser.previous; fprintf(parser.output_file, " %.*s ", op.length, op.start); parse_comparison(); } }
static void parse_logic_and() { parse_equality(); while (match(TOKEN_AND)) { fprintf(parser.output_file, " && "); parse_equality(); } }
static void parse_logic_or() { parse_logic_and(); while (match(TOKEN_OR)) { fprintf(parser.output_file, " || "); parse_logic_and(); } }
static void parse_expression() { parse_logic_or(); }

static void parse_print_statement() {
    consume(TOKEN_LPAREN, "Expect '(' after 'print'.");
    if (parser.current.type == TOKEN_IDENTIFIER) {
        Symbol* symbol = get_symbol(parser.current.start, parser.current.length);
        if (symbol == NULL) { error_at(&parser.current, "Undefined variable."); }
        else {
            VarType type = symbol->type;
            if (type == TYPE_STRING) { fprintf(parser.output_file, "    printf(\"%%s\\n\", %.*s);\n", parser.current.length, parser.current.start); }
            else if (type == TYPE_INT) { fprintf(parser.output_file, "    printf(\"%%d\\n\", %.*s);\n", parser.current.length, parser.current.start); }
            else if (type == TYPE_FLOAT) { fprintf(parser.output_file, "    printf(\"%%f\\n\", %.*s);\n", parser.current.length, parser.current.start); }
            else if (type == TYPE_BOOL) { fprintf(parser.output_file, "    printf(\"%%s\\n\", %.*s ? \"true\" : \"false\");\n", parser.current.length, parser.current.start); }
            else { error_at(&parser.current, "Cannot print value of this type."); }
            advance_parser();
        }
    } else if (parser.current.type == TOKEN_STRING) {
        fprintf(parser.output_file, "    printf(\"%%s\\n\", %.*s);\n", parser.current.length, parser.current.start);
        advance_parser();
    } else {
        error_at(&parser.current, "Can only print an identifier or a string literal.");
    }
    consume(TOKEN_RPAREN, "Expect ')' after print argument.");
    consume(TOKEN_SEMICOLON, "Expect ';' after statement.");
}

static void parse_let_statement(CompilerMode mode) {
    consume(TOKEN_IDENTIFIER, "Expect variable name.");
    Token name = parser.previous;
    consume(TOKEN_COLON, "Expect ':' after variable name.");
    VarType type = parse_type();
    add_symbol(name.start, name.length, type);
    consume(TOKEN_EQ, "Expect '=' after type annotation.");

    if (type == TYPE_ARRAY) {
        fprintf(parser.output_file, "int %.*s[] = ", name.length, name.start);
    } else {
        const char* type_str = type_to_c_str(type);
        if (mode == MODE_RUN) fprintf(parser.output_file, "    ");
        fprintf(parser.output_file, "%s %.*s = ", type_str, name.length, name.start);
    }
    parse_expression();
    fprintf(parser.output_file, ";\n");
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
}

static void parse_if_statement(CompilerMode mode);
static void parse_while_statement(CompilerMode mode);
static void parse_statement(CompilerMode mode) {
    if (match(TOKEN_PRINT)) { parse_print_statement(); }
    else if (match(TOKEN_LET)) { parse_let_statement(mode); }
    else if (match(TOKEN_IF)) { parse_if_statement(mode); }
    else if (match(TOKEN_WHILE)) { parse_while_statement(mode); }
    else { error_at(&parser.current, "Expect statement."); advance_parser(); }
}
static void parse_block(CompilerMode mode) { consume(TOKEN_LBRACE, "Expect '{' before block."); fprintf(parser.output_file, " {\n"); while (parser.current.type != TOKEN_RBRACE && parser.current.type != TOKEN_EOF) { parse_statement(mode); } consume(TOKEN_RBRACE, "Expect '}' after block."); fprintf(parser.output_file, "}\n"); }
static void parse_if_statement(CompilerMode mode) { fprintf(parser.output_file, "if ("); parse_expression(); fprintf(parser.output_file, ")"); parse_block(mode); if (match(TOKEN_ELSE)) { fprintf(parser.output_file, " else "); if (match(TOKEN_IF)) { parse_if_statement(mode); } else { parse_block(mode); } } }
static void parse_while_statement(CompilerMode mode) { fprintf(parser.output_file, "while ("); parse_expression(); fprintf(parser.output_file, ")"); parse_block(mode); }
static void parse_function_definition(CompilerMode mode);
static void parse_struct_definition();

int transpile(const char* source, FILE* output_file, CompilerMode mode) {
    init_lexer(source);
    parser.output_file = output_file;
    parser.had_error = 0;
    parser.panic_mode = 0;
    if (mode == MODE_RUN) { fprintf(parser.output_file, "#include <stdio.h>\n\n"); }
    advance_parser();
    while (parser.current.type != TOKEN_EOF) {
        if (parser.panic_mode) synchronize();
        if (match(TOKEN_FN)) { parse_function_definition(mode); }
        else if (match(TOKEN_STRUCT)) { parse_struct_definition(); }
        else { error_at(&parser.current, "Only function or struct definitions are allowed at the top level."); }
    }
    return parser.had_error ? 1 : 0;
}

static void parse_struct_definition() {
    consume(TOKEN_IDENTIFIER, "Expect struct name.");
    Token name = parser.previous;
    StructDef* def = (StructDef*)malloc(sizeof(StructDef));
    def->name = (char*)malloc(name.length + 1);
    strncpy(def->name, name.start, name.length);
    def->name[name.length] = '\0';
    def->fields = NULL;
    Field** field_tail = &def->fields;
    Symbol* struct_symbol = add_symbol(name.start, name.length, TYPE_STRUCT);
    struct_symbol->as.struct_def = def;
    fprintf(parser.output_file, "typedef struct %.*s {\n", name.length, name.start);
    consume(TOKEN_LBRACE, "Expect '{' before struct body.");
    while(!match(TOKEN_RBRACE)) {
        consume(TOKEN_IDENTIFIER, "Expect field name.");
        Token field_name = parser.previous;
        consume(TOKEN_COLON, "Expect ':' after field name.");
        VarType type = parse_type();
        fprintf(parser.output_file, "    %s %.*s;\n", type_to_c_str(type), field_name.length, field_name.start);
        consume(TOKEN_SEMICOLON, "Expect ';' after field.");
        Field* field = (Field*)malloc(sizeof(Field));
        field->name = (char*)malloc(field_name.length + 1);
        strncpy(field->name, field_name.start, field_name.length);
        field->name[field_name.length] = '\0';
        field->type = type;
        field->next = NULL;
        *field_tail = field;
        field_tail = &field->next;
    }
    fprintf(parser.output_file, "} %.*s;\n\n", name.length, name.start);
}

static void parse_function_definition(CompilerMode mode) {
    consume(TOKEN_IDENTIFIER, "Expect function name.");
    Token name = parser.previous;
    consume(TOKEN_LPAREN, "Expect '('.");
    FuncSignature* sig = (FuncSignature*)malloc(sizeof(FuncSignature));
    sig->params = NULL;
    FuncParam** param_tail = &sig->params;
    if (!match(TOKEN_RPAREN)) {
        do {
            FuncParam* param = (FuncParam*)malloc(sizeof(FuncParam));
            consume(TOKEN_IDENTIFIER, "Expect parameter name.");
            param->name = (char*)malloc(parser.previous.length + 1);
            strncpy(param->name, parser.previous.start, parser.previous.length);
            param->name[parser.previous.length] = '\0';
            consume(TOKEN_COLON, "Expect ':' after parameter name.");
            param->type = parse_type();
            param->next = NULL;
            *param_tail = param;
            param_tail = &param->next;
        } while (match(TOKEN_COMMA));
        consume(TOKEN_RPAREN, "Expect ')' after parameters.");
    }
    consume(TOKEN_ARROW, "Expect '->' for return type.");
    sig->return_type = parse_type();
    Symbol* func_symbol = add_symbol(name.start, name.length, TYPE_FUNCTION);
    func_symbol->as.func_sig = sig;
    char func_header[1024];
    char temp_buffer[1024] = "";
    int offset = 0;
    offset += snprintf(temp_buffer, sizeof(temp_buffer), "%s %.*s(", type_to_c_str(sig->return_type), name.length, name.start);
    for (FuncParam* p = sig->params; p != NULL; p = p->next) {
        offset += snprintf(temp_buffer + offset, sizeof(temp_buffer) - offset, "%s %s", type_to_c_str(p->type), p->name);
        if (p->next != NULL) {
            offset += snprintf(temp_buffer + offset, sizeof(temp_buffer) - offset, ", ");
        }
    }
    snprintf(func_header, sizeof(func_header), "%s)", temp_buffer);
    if (mode == MODE_BUILD_LIB_HEADER) {
        fprintf(parser.output_file, "extern %s;\n", func_header);
        consume(TOKEN_LBRACE, "Expect '{'.");
        while(parser.current.type != TOKEN_RBRACE && parser.current.type != TOKEN_EOF) advance_parser();
        consume(TOKEN_RBRACE, "Expect '}'.");
    } else {
        fprintf(parser.output_file, "%s", func_header);
        parse_block(mode);
    }
}

static void synchronize() { parser.panic_mode = 0; while (parser.current.type != TOKEN_EOF) { if (parser.previous.type == TOKEN_SEMICOLON) return; switch (parser.current.type) { case TOKEN_FN: case TOKEN_LET: case TOKEN_IF: case TOKEN_WHILE: case TOKEN_PRINT: case TOKEN_RETURN: case TOKEN_STRUCT: return; default:; } advance_parser(); } }
static void error_at(Token* token, const char* message) { if (parser.panic_mode) return; parser.panic_mode = 1; fprintf(stderr, "[line %d] Error", token->line); if (token->type == TOKEN_EOF) { fprintf(stderr, " at end"); } else if (token->type != TOKEN_UNKNOWN) { fprintf(stderr, " at '%.*s'", token->length, token->start); } fprintf(stderr, ": %s\n", message); parser.had_error = 1; }

int transpile_repl(const char* source) {
    init_lexer(source);
    parser.output_file = fopen("/dev/null", "w");
    parser.had_error = 0;
    parser.panic_mode = 0;
    advance_parser();
    if (parser.current.type == TOKEN_EOF) return 0;
    parse_expression();
    if (!parser.had_error) { printf("Valid Expression\n"); }
    fclose(parser.output_file);
    return parser.had_error;
}
