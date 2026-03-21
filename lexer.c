#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer.h"

static int is_keyword(const char *word, TokenType *out)
{
    struct
    {
        const char *kw;
        TokenType tt;
    } kws[] = {
        {"SELECT", TOK_SELECT},
        {"FROM", TOK_FROM},
        {"WHERE", TOK_WHERE},
        {"GROUP", TOK_GROUP},
        {"BY", TOK_BY},
        {"AND", TOK_AND},
        {"OR", TOK_OR},
        {"COUNT", TOK_COUNT},
        {"SUM", TOK_SUM},
        {"AVG", TOK_AVG},
    };
    int n = sizeof(kws) / sizeof(kws[0]);
    for (int i = 0; i < n; i++)
        if (strcasecmp(word, kws[i].kw) == 0)
        {
            *out = kws[i].tt;
            return 1;
        }
    return 0;
}

TokenStream *tokenize(const char *input)
{
    TokenStream *ts = calloc(1, sizeof(TokenStream));
    if (!ts)
        return NULL;

    const char *p = input;

    while (*p && ts->count < MAX_TOKENS - 1)
    {
        while (isspace((unsigned char)*p))
            p++;
        if (!*p)
            break;

        Token *tok = &ts->tokens[ts->count];

        if (p[0] == '<' && p[1] == '=')
        {
            tok->type = TOK_LTE;
            strcpy(tok->text, "<=");
            p += 2;
        }
        else if (p[0] == '>' && p[1] == '=')
        {
            tok->type = TOK_GTE;
            strcpy(tok->text, ">=");
            p += 2;
        }
        else if (p[0] == '!' && p[1] == '=')
        {
            tok->type = TOK_NEQ;
            strcpy(tok->text, "!=");
            p += 2;
        }
        else if (p[0] == '=' && p[1] == '=')
        {
            tok->type = TOK_EQ;
            strcpy(tok->text, "==");
            p += 2;
        }
        else if (*p == '<')
        {
            tok->type = TOK_LT;
            strcpy(tok->text, "<");
            p++;
        }
        else if (*p == '>')
        {
            tok->type = TOK_GT;
            strcpy(tok->text, ">");
            p++;
        }
        else if (*p == '=')
        {
            tok->type = TOK_EQ;
            strcpy(tok->text, "=");
            p++;
        }
        else if (*p == '*')
        {
            tok->type = TOK_STAR;
            strcpy(tok->text, "*");
            p++;
        }
        else if (*p == '(')
        {
            tok->type = TOK_LPAREN;
            strcpy(tok->text, "(");
            p++;
        }
        else if (*p == ')')
        {
            tok->type = TOK_RPAREN;
            strcpy(tok->text, ")");
            p++;
        }
        else if (*p == ',')
        {
            tok->type = TOK_COMMA;
            strcpy(tok->text, ",");
            p++;
        }
        else if (*p == '\'' || *p == '"')
        {
            char q = *p++;
            int i = 0;
            while (*p && *p != q && i < 255)
                tok->text[i++] = *p++;
            tok->text[i] = '\0';
            if (*p == q)
                p++;
            tok->type = TOK_STRING;
        }
        else if (isdigit((unsigned char)*p) || (*p == '-' && isdigit((unsigned char)p[1])))
        {
            int i = 0;
            if (*p == '-')
                tok->text[i++] = *p++;
            while ((isdigit((unsigned char)*p) || *p == '.') && i < 255)
                tok->text[i++] = *p++;
            tok->text[i] = '\0';
            tok->type = TOK_NUMBER;
        }
        else if (isalpha((unsigned char)*p) || *p == '_')
        {
            int i = 0;
            while ((isalnum((unsigned char)*p) || *p == '_' || *p == '.') && i < 255)
                tok->text[i++] = *p++;
            tok->text[i] = '\0';
            TokenType kw;
            tok->type = is_keyword(tok->text, &kw) ? kw : TOK_IDENT;
        }
        else
        {
            fprintf(stderr, "lexer error: unexpected char '%c'\n", *p);
            tok->type = TOK_ERROR;
            tok->text[0] = *p;
            tok->text[1] = '\0';
            p++;
        }

        ts->count++;
    }

    ts->tokens[ts->count].type = TOK_EOF;
    strcpy(ts->tokens[ts->count].text, "EOF");
    ts->count++;

    return ts;
}

void free_tokens(TokenStream *ts)
{
    free(ts);
}