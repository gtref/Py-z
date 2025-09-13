#include "parser.h"
#include "lexer.h"
#include "symtab.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Parser Implementation ---
typedef struct { Token current; Token previous; int had_error; } Parser;
static Parser parser;

static void error_at(Token* token, const char* message);
static void advance_parser() { parser.previous = parser.current; for (;;) { parser.current = get_next_token(); if (parser.current.type != TOKEN_UNKNOWN) break; } }
static void consume(TokenType type, const char* message) { if (parser.current.type == type) { advance_parser(); return; } error_at(&parser.current, message); }
static int match(TokenType type) { if (parser.current.type == type) { advance_parser(); return 1; } return 0; }
static int check(TokenType type) { return parser.current.type == type; }

static AstNode* parse_expression();
static AstNode* parse_statement();
static StmtListNode* parse_block();
static VarType parse_type();
static AstNode* parse_struct_definition();

static AstNode* parse_primary() {
    if (match(TOKEN_NUMBER)) {
        AstNode* node = create_node(NODE_LITERAL_FLOAT);
        node->as.literal.float_val = strtod(parser.previous.start, NULL);
        return node;
    }
    if (match(TOKEN_IDENTIFIER)) {
        AstNode* node = create_node(NODE_IDENTIFIER);
        node->as.identifier.name = (char*)malloc(parser.previous.length + 1);
        strncpy(node->as.identifier.name, parser.previous.start, parser.previous.length);
        node->as.identifier.name[parser.previous.length] = '\0';
        return node;
    }
    error_at(&parser.current, "Expect expression.");
    return NULL;
}
static AstNode* parse_call() { AstNode* expr = parse_primary(); while(match(TOKEN_DOT) || match(TOKEN_LSQUARE)) { if (parser.previous.type == TOKEN_DOT) { AstNode* node = create_node(NODE_MEMBER_ACCESS); node->as.member_access.object = expr; consume(TOKEN_IDENTIFIER, "Expect property name after '.'."); node->as.member_access.field_name = (char*)malloc(parser.previous.length + 1); strncpy(node->as.member_access.field_name, parser.previous.start, parser.previous.length); node->as.member_access.field_name[parser.previous.length] = '\0'; expr = node;} else { AstNode* node = create_node(NODE_ARRAY_ACCESS); node->as.array_access.array = expr; node->as.array_access.index = parse_expression(); consume(TOKEN_RSQUARE, "Expect ']' after index."); expr = node; } } return expr; }
static AstNode* parse_unary() { if (match(TOKEN_MINUS)) { Token op = parser.previous; AstNode* right = parse_unary(); AstNode* node = create_node(NODE_UNARY_OP); node->as.unary_op.right = right; return node; } return parse_call(); }
static AstNode* parse_factor() { AstNode* expr = parse_unary(); while (match(TOKEN_STAR) || match(TOKEN_SLASH)) { TokenType op = parser.previous.type; AstNode* right = parse_unary(); expr = create_binary_op(expr, op, right); } return expr; }
static AstNode* parse_term() { AstNode* expr = parse_factor(); while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) { TokenType op = parser.previous.type; AstNode* right = parse_factor(); expr = create_binary_op(expr, op, right); } return expr; }
static AstNode* parse_expression() { return parse_term(); }

static AstNode* parse_statement() {
    if (match(TOKEN_PRINT)) { AstNode* node = create_node(NODE_PRINT_STMT); consume(TOKEN_LPAREN, "Expect '('."); node->as.print_stmt.expression = parse_expression(); consume(TOKEN_RPAREN, "Expect ')'."); consume(TOKEN_SEMICOLON, "Expect ';'."); return node; }
    if (match(TOKEN_LET)) { AstNode* node = create_node(NODE_LET_DECL); consume(TOKEN_IDENTIFIER, "Expect variable name."); Token name = parser.previous; node->as.let_decl.name = (char*)malloc(name.length + 1); strncpy(node->as.let_decl.name, name.start, name.length); node->as.let_decl.name[name.length] = '\0'; consume(TOKEN_COLON, "Expect ':'."); node->as.let_decl.type = parse_type(); consume(TOKEN_EQ, "Expect '='."); node->as.let_decl.initializer = parse_expression(); consume(TOKEN_SEMICOLON, "Expect ';'."); return node; }
    if (match(TOKEN_RETURN)) { AstNode* node = create_node(NODE_RETURN_STMT); node->as.return_stmt.expression = parse_expression(); consume(TOKEN_SEMICOLON, "Expect ';'."); return node; }
    return parse_expression();
}

static StmtListNode* parse_block() { consume(TOKEN_LBRACE, "Expect '{'."); StmtListNode* head = NULL; StmtListNode** tail = &head; while (!check(TOKEN_RBRACE) && !check(TOKEN_EOF)) { StmtListNode* new_node = (StmtListNode*)malloc(sizeof(StmtListNode)); new_node->statement = parse_statement(); new_node->next = NULL; if (new_node->statement != NULL) { *tail = new_node; tail = &new_node->next; } else { free(new_node); } } consume(TOKEN_RBRACE, "Expect '}'."); return head; }
static AstNode* parse_function_definition() { consume(TOKEN_FN, "Expect 'fn'."); consume(TOKEN_IDENTIFIER, "Expect function name."); Token name = parser.previous; consume(TOKEN_LPAREN, "Expect '('."); consume(TOKEN_RPAREN, "Expect ')'."); consume(TOKEN_ARROW, "Expect '->'."); VarType return_type = parse_type(); AstNode* fn_node = create_node(NODE_FN_DEF); fn_node->as.fn_def.name = (char*)malloc(name.length + 1); strncpy(fn_node->as.fn_def.name, name.start, name.length); fn_node->as.fn_def.name[name.length] = '\0'; fn_node->as.fn_def.block_stmts = parse_block(); return fn_node; }

static AstNode* parse_struct_definition() {
    consume(TOKEN_STRUCT, "Expect 'struct'.");
    consume(TOKEN_IDENTIFIER, "Expect struct name.");
    Token name = parser.previous;
    AstNode* node = create_node(NODE_STRUCT_DEF);
    node->as.struct_def.name = (char*)malloc(name.length + 1);
    strncpy(node->as.struct_def.name, name.start, name.length);
    node->as.struct_def.name[name.length] = '\0';
    node->as.struct_def.fields = parse_block(); // Re-use block parsing for fields
    return node;
}

AstNode* parse(const char* source) {
    init_lexer(source);
    parser.had_error = 0;
    advance_parser();
    StmtListNode* head = NULL;
    StmtListNode** tail = &head;
    while(!check(TOKEN_EOF)) {
        StmtListNode* new_node = (StmtListNode*)malloc(sizeof(StmtListNode));
        if (check(TOKEN_STRUCT)) {
            new_node->statement = parse_struct_definition();
        } else {
            new_node->statement = parse_function_definition();
        }
        new_node->next = NULL;
        if (new_node->statement != NULL) { *tail = new_node; tail = &new_node->next; }
        else { free(new_node); }
    }
    if (parser.had_error) { return NULL; }
    AstNode* program_node = create_node(NODE_BLOCK);
    program_node->as.block_stmts = head;
    return program_node;
}

static VarType parse_type() {
    if (match(TOKEN_LSQUARE)) {
        parse_type();
        consume(TOKEN_RSQUARE, "Expect ']' after array type.");
        return TYPE_ARRAY;
    }
    if (match(TOKEN_INT)) return TYPE_INT;
    if (match(TOKEN_FLOAT_TYPE)) return TYPE_FLOAT;
    if (match(TOKEN_BOOL_TYPE)) return TYPE_BOOL;
    if (match(TOKEN_STRING_TYPE)) return TYPE_STRING;
    if (match(TOKEN_IDENTIFIER)) return TYPE_STRUCT;
    error_at(&parser.current, "Expect type.");
    return TYPE_UNKNOWN;
}

static void error_at(Token* token, const char* message) { if (parser.had_error) return; parser.had_error = 1; fprintf(stderr, "[line %d] Error: %s\n", token->line, message); }
