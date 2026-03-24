#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

typedef struct
{
    TokenStream *ts;
    int pos;
} Parser;

static Token *peek(Parser *p)
{
    return &p->ts->tokens[p->pos];
}

static Token *consume(Parser *p)
{
    Token *t = &p->ts->tokens[p->pos];
    if (t->type != TOK_EOF)
        p->pos++;
    return t;
}

static int expect(Parser *p, TokenType type)
{
    if (peek(p)->type != type)
    {
        fprintf(stderr, "parse error: expected token type %u, got %s\n",
                type, peek(p)->text);
        return 0;
    }
    consume(p);
    return 1;
}

/* ---- WHERE expression parser ---- */

static Expr *new_cmp(const char *col, CmpOp op, const char *val)
{
    Expr *e = calloc(1, sizeof(Expr));
    e->type = EXPR_CMP;
    strncpy(e->col, col, 63);
    e->op = op;
    strncpy(e->val, val, 63);
    return e;
}

static Expr *new_logic(ExprType type, Expr *left, Expr *right)
{
    Expr *e = calloc(1, sizeof(Expr));
    e->type = type;
    e->left = left;
    e->right = right;
    return e;
}

static CmpOp token_to_cmpop(TokenType t)
{
    switch (t)
    {
    case TOK_EQ:
        return OP_EQ;
    case TOK_NEQ:
        return OP_NEQ;
    case TOK_LT:
        return OP_LT;
    case TOK_GT:
        return OP_GT;
    case TOK_LTE:
        return OP_LTE;
    case TOK_GTE:
        return OP_GTE;
    default:
        return OP_EQ; // default case, should not happen
    }
}

static int is_cmpop(TokenType t)
{
    return t == TOK_EQ || t == TOK_NEQ || t == TOK_LT || t == TOK_GT || t == TOK_LTE || t == TOK_GTE;
}

/* term: IDENT OP (NUMBER | STRING | IDENT) */
static Expr *parse_term(Parser *p)
{
    if (peek(p)->type != TOK_IDENT)
    {
        fprintf(stderr, "parse error: expected column name, got '%s'\n", peek(p)->text);
        return NULL;
    }
    char col[64];
    strncpy(col, consume(p)->text, 63);

    if (!is_cmpop(peek(p)->type))
    {
        fprintf(stderr, "parse error: expected comparison operator, got '%s'\n", peek(p)->text);
        return NULL;
    }
    CmpOp op = token_to_cmpop(consume(p)->type);

    TokenType vt = peek(p)->type;
    if (vt != TOK_NUMBER && vt != TOK_STRING && vt != TOK_IDENT)
    {
        fprintf(stderr, "parse error: expected value, got '%s'\n", peek(p)->text);
        return NULL;
    }
    char val[64];
    strncpy(val, consume(p)->text, 63);

    return new_cmp(col, op, val);
}

/* expr: term ((AND|OR) term)* */
static Expr *parse_expr(Parser *p)
{
    Expr *left = parse_term(p);
    if (!left)
        return NULL;

    while (peek(p)->type == TOK_AND || peek(p)->type == TOK_OR)
    {
        ExprType logic = (consume(p)->type == TOK_AND) ? EXPR_AND : EXPR_OR;
        Expr *right = parse_term(p);
        if (!right)
        {
            free(left);
            return NULL;
        }
        left = new_logic(logic, left, right);
    }
    return left;
}

/* ---- SELECT column list ---- */
static int parse_select_cols(Parser *p, QueryPlan *qp)
{
    if (peek(p)->type == TOK_STAR)
    {
        consume(p);
        qp->ncols = 0;
        return 1;
    }

    while (1)
    {
        AggType agg = AGG_NONE;
        char name[64];

        if (peek(p)->type == TOK_COUNT || peek(p)->type == TOK_SUM ||
            peek(p)->type == TOK_AVG || peek(p)->type == TOK_MIN ||
            peek(p)->type == TOK_MAX)
        {
            TokenType tt = consume(p)->type;
            agg = (tt == TOK_COUNT) ? AGG_COUNT : (tt == TOK_SUM) ? AGG_SUM
                                              : (tt == TOK_AVG)   ? AGG_AVG
                                              : (tt == TOK_MIN)   ? AGG_MIN
                                                                  : AGG_MAX;
            if (!expect(p, TOK_LPAREN))
                return 0;
            if (peek(p)->type != TOK_IDENT && peek(p)->type != TOK_STAR)
            {
                fprintf(stderr, "parse error: expected column in aggregate\n");
                return 0;
            }
            strncpy(name, consume(p)->text, 63);
            if (!expect(p, TOK_RPAREN))
                return 0;
        }
        else if (peek(p)->type == TOK_IDENT)
        {
            strncpy(name, consume(p)->text, 63);
        }
        else
        {
            fprintf(stderr, "debug: token type=%d text='%s'\n",
                    peek(p)->type, peek(p)->text);
            fprintf(stderr, "parse error: expected column name or aggregation function, got '%s'\n",
                    peek(p)->text);
            return 0;
        }

        if (qp->ncols >= MAX_SELECT_COLS)
        {
            fprintf(stderr, "parse error: too many SELECT columns\n");
            return 0;
        }
        strncpy(qp->cols[qp->ncols].name, name, 63);
        qp->cols[qp->ncols].agg = agg;
        qp->ncols++;

        if (peek(p)->type != TOK_COMMA)
            break;
        consume(p);
    }
    return 1;
}

/* ---- Top-level parse ---- */

QueryPlan *parse(TokenStream *ts)
{
    Parser p = {ts, 0};
    QueryPlan *qp = calloc(1, sizeof(QueryPlan));
    if (!qp)
        return NULL;

    if (!expect(&p, TOK_SELECT))
        goto fail;
    if (!parse_select_cols(&p, qp))
        goto fail;
    if (!expect(&p, TOK_FROM))
        goto fail;

    if (peek(&p)->type != TOK_IDENT)
    {
        fprintf(stderr, "parse error: expected filename after FROM\n");
        goto fail;
    }
    strncpy(qp->filename, consume(&p)->text, 255);

    if (peek(&p)->type == TOK_WHERE)
    {
        consume(&p);
        qp->where = parse_expr(&p);
        if (!qp->where)
            goto fail;
    }

    if (peek(&p)->type == TOK_GROUP)
    {
        consume(&p);
        if (!expect(&p, TOK_BY))
            goto fail;
        if (peek(&p)->type != TOK_IDENT)
        {
            fprintf(stderr, "parse error: expected column name after GROUP BY\n");
            goto fail;
        }
        strncpy(qp->groupby, consume(&p)->text, 63);
    }

    return qp;

fail:
    free_query_plan(qp);
    return NULL;
}

void free_expr(Expr *e)
{
    if (!e)
        return;
    free_expr(e->left);
    free_expr(e->right);
    free(e);
}

void free_query_plan(QueryPlan *qp)
{
    if (!qp)
        return;
    free_expr(qp->where);
    free(qp);
}