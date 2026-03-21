#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    TOK_SELECT,
    TOK_FROM,
    TOK_WHERE,
    TOK_GROUP,
    TOK_BY,
    TOK_AND,
    TOK_OR,
    TOK_COUNT,
    TOK_SUM,
    TOK_AVG,
    TOK_MIN,
    TOK_MAX,
    TOK_STAR,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_EQ,
    TOK_NEQ,
    TOK_LT,
    TOK_GT,
    TOK_LTE,
    TOK_GTE,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_EOF,
    TOK_ERROR
} TokenType;

typedef struct
{
    TokenType type;
    char text[256];
} Token;

#define MAX_TOKENS 256

typedef struct
{
    Token tokens[MAX_TOKENS];
    int count;
} TokenStream;

TokenStream *tokenize(const char *input);
void free_tokens(TokenStream *ts);

#endif