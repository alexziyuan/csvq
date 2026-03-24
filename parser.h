#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "query.h"

QueryPlan *parse(TokenStream *ts);
void free_expr(Expr *e);
void free_query_plan(QueryPlan *qp);

#endif