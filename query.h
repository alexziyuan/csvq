#ifndef QUERY_H
#define QUERY_H

#define MAX_SELECT_COLS 64

/* ---------- WHERE expression tree ---------- */
typedef enum
{
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE
} CmpOp;
typedef enum
{
    EXPR_CMP,
    EXPR_AND,
    EXPR_OR
} ExprType;

typedef struct Expr
{
    ExprType type;
    struct Expr *left;
    struct Expr *right;
    char col[64];
    CmpOp op;
    char val[64];
} Expr;

/* ---------- SELECT column descriptor ---------- */
typedef enum
{
    AGG_NONE,
    AGG_SUM,
    AGG_COUNT,
    AGG_AVG,
    AGG_MIN,
    AGG_MAX
} AggType;

typedef struct
{
    char name[64];
    AggType agg;
} SelectCol;

/* ---------- Full query plan ---------- */
typedef struct
{
    char filename[256];
    SelectCol cols[MAX_SELECT_COLS];
    int ncols;        /* 0 = SELECT * */
    Expr *where;      /* NULL = no filter */
    char groupby[64]; /* "" = no GROUP BY */
} QueryPlan;

#endif