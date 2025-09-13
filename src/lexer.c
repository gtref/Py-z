#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// --- Lexer Implementation ---
typedef struct { const char *start; const char *current; int line; } Lexer;
static Lexer lexer;
int g_slang_mode = 0;

void set_slang_mode(int enabled) {
    g_slang_mode = enabled;
}

void init_lexer(const char *source) { lexer.start = source; lexer.current = source; lexer.line = 1; }
static int is_at_end() { return *lexer.current == '\0'; }
static char advance() { lexer.current++; return lexer.current[-1]; }
static char peek() { return *lexer.current; }

static void skip_whitespace() { for (;;) { char c = peek(); switch (c) { case ' ': case '\r': case '\t': advance(); break; case '\n': lexer.line++; advance(); break; case '/': if (lexer.current[1] == '/') { while (peek() != '\n' && !is_at_end()) advance(); } else if (lexer.current[1] == '*') { advance(); advance(); while (!(peek() == '*' && lexer.current[1] == '/') && !is_at_end()) { if (peek() == '\n') lexer.line++; advance(); } if (!is_at_end()) advance(); if (!is_at_end()) advance(); } else { return; } break; default: return; } } }
static Token make_token(TokenType type) { Token token; token.type = type; token.start = lexer.start; token.length = (int)(lexer.current - lexer.start); token.line = lexer.line; return token; }
static Token error_token(const char* message) { Token token; token.type = TOKEN_UNKNOWN; token.start = message; token.length = (int)strlen(message); token.line = lexer.line; return token; }
static TokenType check_keyword(int start, int length, const char* rest, TokenType type) { if (lexer.current - lexer.start == start + length && memcmp(lexer.start + start, rest, length) == 0) { return type; } return TOKEN_IDENTIFIER; }

static TokenType identifier_type() {
    if (g_slang_mode) {
        switch (lexer.start[0]) {
            case 'b': return check_keyword(1, 2, "et", TOKEN_LET);
            case 'c': return check_keyword(1, 2, "ap", TOKEN_FALSE);
            case 'f': return check_keyword(1, 2, "ax", TOKEN_TRUE);
            case 'o': return check_keyword(1, 8, "nrepeat", TOKEN_WHILE);
            case 's':
                if (lexer.current - lexer.start > 1) {
                    switch (lexer.start[1]) {
                        case 'u': return check_keyword(2, 1, "s", TOKEN_IF);
                        case 'p': return check_keyword(2, 3, "ill", TOKEN_PRINT);
                    }
                }
                break;
            case 'n': return check_keyword(1, 2, "ah", TOKEN_ELSE);
            case 'v': return check_keyword(1, 8, "ibecheck", TOKEN_FN);
            case 'y': return check_keyword(1, 3, "eet", TOKEN_RETURN);
        }
    } else {
        switch (lexer.start[0]) {
            case 'b': return check_keyword(1, 3, "ool", TOKEN_BOOL_TYPE);
            case 'e': return check_keyword(1, 3, "lse", TOKEN_ELSE);
            case 'f':
                if (lexer.current - lexer.start > 1) {
                    switch (lexer.start[1]) {
                        case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
                        case 'l': return check_keyword(2, 3, "oat", TOKEN_FLOAT_TYPE);
                        case 'n': return TOKEN_FN;
                    }
                }
                break;
            case 'i':
                if (lexer.current - lexer.start > 1) {
                    switch (lexer.start[1]) {
                        case 'f': return TOKEN_IF;
                        case 'n': return check_keyword(2, 1, "t", TOKEN_INT);
                        case 'm': return check_keyword(2, 4, "port", TOKEN_IMPORT);
                    }
                }
                break;
            case 'l': return check_keyword(1, 2, "et", TOKEN_LET);
            case 'p': return check_keyword(1, 4, "rint", TOKEN_PRINT);
            case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
            case 's':
                if (lexer.current - lexer.start > 1) {
                    switch (lexer.start[1]) {
                        case 't': return check_keyword(2, 4, "ruct", TOKEN_STRUCT);
                        case 'r': return check_keyword(2, 4, "ing", TOKEN_STRING_TYPE);
                    }
                }
                break;
            case 't': return check_keyword(1, 3, "rue", TOKEN_TRUE);
            case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
        }
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier() { while (isalpha(peek()) || isdigit(peek())) advance(); return make_token(identifier_type()); }
static Token string() { while (peek() != '"' && !is_at_end()) { if (peek() == '\n') lexer.line++; advance(); } if (is_at_end()) return error_token("Unterminated string."); advance(); return make_token(TOKEN_STRING); }
static Token number() { while (isdigit(peek())) advance(); if (peek() == '.' && isdigit(lexer.current[1])) { advance(); while (isdigit(peek())) advance(); } return make_token(TOKEN_NUMBER); }

Token get_next_token() {
    skip_whitespace();
    lexer.start = lexer.current;
    if (is_at_end()) return make_token(TOKEN_EOF);
    char c = advance();
    if (isalpha(c)) return identifier();
    if (isdigit(c)) return number();
    if (c == '"') return string();
    switch (c) {
        case '(': return make_token(TOKEN_LPAREN); case ')': return make_token(TOKEN_RPAREN);
        case '{': return make_token(TOKEN_LBRACE); case '}': return make_token(TOKEN_RBRACE);
        case '[': return make_token(TOKEN_LSQUARE); case ']': return make_token(TOKEN_RSQUARE);
        case ';': return make_token(TOKEN_SEMICOLON); case ':': return make_token(TOKEN_COLON);
        case ',': return make_token(TOKEN_COMMA); case '.': return make_token(TOKEN_DOT);
        case '+': return make_token(TOKEN_PLUS); case '*': return make_token(TOKEN_STAR);
        case '/': return make_token(TOKEN_SLASH);
        case '!': if (peek() == '=') { advance(); return make_token(TOKEN_BANG_EQ); } return make_token(TOKEN_BANG);
        case '=': if (peek() == '=') { advance(); return make_token(TOKEN_EQ_EQ); } return make_token(TOKEN_EQ);
        case '<': return make_token(TOKEN_LT); case '>': return make_token(TOKEN_GT);
        case '&': if (peek() == '&') { advance(); return make_token(TOKEN_AND); } return make_token(TOKEN_AMPERSAND);
        case '|': if (peek() == '|') { advance(); return make_token(TOKEN_OR); } return make_token(TOKEN_PIPE);
        case '-': if (peek() == '>') { advance(); return make_token(TOKEN_ARROW); } return make_token(TOKEN_MINUS);
    }
    return error_token("Unexpected character.");
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_LPAREN: return "LPAREN"; case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE"; case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LSQUARE: return "LSQUARE"; case TOKEN_RSQUARE: return "RSQUARE";
        case TOKEN_SEMICOLON: return "SEMICOLON"; case TOKEN_EQ: return "EQ";
        case TOKEN_PLUS: return "PLUS"; case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR"; case TOKEN_SLASH: return "SLASH";
        case TOKEN_COLON: return "COLON"; case TOKEN_GT: return "GT";
        case TOKEN_LT: return "LT"; case TOKEN_COMMA: return "COMMA";
        case TOKEN_DOT: return "DOT"; case TOKEN_BANG: return "BANG";
        case TOKEN_AMPERSAND: return "AMPERSAND"; case TOKEN_PIPE: return "PIPE";
        case TOKEN_ARROW: return "ARROW"; case TOKEN_EQ_EQ: return "EQ_EQ";
        case TOKEN_BANG_EQ: return "BANG_EQ"; case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR"; case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING"; case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRUCT: return "STRUCT"; case TOKEN_LET: return "LET";
        case TOKEN_PRINT: return "PRINT"; case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_FN: return "FN"; case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE"; case TOKEN_RETURN: return "RETURN";
        case TOKEN_WHILE: return "WHILE"; case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE"; case TOKEN_INT: return "INT";
        case TOKEN_STRING_TYPE: return "STRING_TYPE"; case TOKEN_BOOL_TYPE: return "BOOL_TYPE";
        case TOKEN_FLOAT_TYPE: return "FLOAT_TYPE"; case TOKEN_EOF: return "EOF";
        case TOKEN_UNKNOWN: return "UNKNOWN";
    }
    return "INVALID";
}
