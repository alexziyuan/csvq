#ifndef EVAL_H
#define EVAL_H

#include "table.h"
#include "query.h"

int eval_expr(Table *t, Row *row, Expr *e);
int find_col(Table *t, const char *name);

#endif