#ifndef LEXER_H
#define LEXER_H

// --- Token Definitions ---
typedef enum {
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_LSQUARE, TOKEN_RSQUARE,
    TOKEN_SEMICOLON, TOKEN_EQ, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_COLON, TOKEN_COMMA, TOKEN_DOT, TOKEN_BANG, TOKEN_AMPERSAND, TOKEN_PIPE,
    TOKEN_GT, TOKEN_LT,
    TOKEN_ARROW, TOKEN_EQ_EQ, TOKEN_BANG_EQ, TOKEN_AND, TOKEN_OR,
    TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_STRING,
    TOKEN_STRUCT, TOKEN_LET, TOKEN_PRINT, TOKEN_IMPORT,
    TOKEN_FN, TOKEN_IF, TOKEN_ELSE, TOKEN_RETURN, TOKEN_WHILE,
    TOKEN_TRUE, TOKEN_FALSE,
    TOKEN_INT, TOKEN_STRING_TYPE, TOKEN_BOOL_TYPE, TOKEN_FLOAT_TYPE,
    TOKEN_EOF, TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

// --- Lexer Interface ---
void init_lexer(const char *source);
Token get_next_token();
void set_slang_mode(int enabled);
const char* token_type_to_string(TokenType type);

#endif // LEXER_H
