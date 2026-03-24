#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eval.h"

int find_col(Table *t, const char *name)
{
    for (int i = 0; i < t->ncols; i++)
    {
        if (strcmp(t->headers[i], name) == 0)
            return i;
    }
    return -1;
}

/* Returns the column value for a given row, or the literal if not a column name */
static const char *resolve(Table *t, Row *row, const char *name)
{
    int idx = find_col(t, name);
    return (idx >= 0) ? row->values[idx] : name;
}

static int eval_cmp(Table *t, Row *row, Expr *e)
{
    const char *lhs = resolve(t, row, e->col);
    const char *rhs = e->val;

    /* try numeric comparison first */
    char *lend, *rend;
    double lv = strtod(lhs, &lend);
    double rv = strtod(rhs, &rend);

    int numeric = (lend != lhs && *lend == '\0' && rend != rhs && *rend == '\0');

    if (numeric)
    {
        switch (e->op)
        {
        case OP_EQ:
            return lv == rv;
        case OP_NEQ:
            return lv != rv;
        case OP_LT:
            return lv < rv;
        case OP_LTE:
            return lv <= rv;
        case OP_GT:
            return lv > rv;
        case OP_GTE:
            return lv >= rv;
        }
    }
    else
    {
        int cmp = strcmp(lhs, rhs);
        switch (e->op)
        {
        case OP_EQ:
            return cmp == 0;
        case OP_NEQ:
            return cmp != 0;
        case OP_LT:
            return cmp < 0;
        case OP_LTE:
            return cmp <= 0;
        case OP_GT:
            return cmp > 0;
        case OP_GTE:
            return cmp >= 0;
        }
    }
    return 0; // should not reach here
}

int eval_expr(Table *t, Row *row, Expr *e)
{
    if (!e)
        return 1; // empty expr is true

    switch (e->type)
    {
    case EXPR_CMP:
        return eval_cmp(t, row, e);
    case EXPR_AND:
        return eval_expr(t, row, e->left) && eval_expr(t, row, e->right);
    case EXPR_OR:
        return eval_expr(t, row, e->left) || eval_expr(t, row, e->right);
    }
    return 0; // should not reach here
}