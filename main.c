#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "csv.h"
#include "lexer.h"
#include "parser.h"
#include "exec.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s \"SELECT ... FROM file.csv ...\"\n", argv[0]);
        return 1;
    }

    TokenStream *ts = tokenize(argv[1]);
    if (!ts)
        return 1;

    QueryPlan *qp = parse(ts);
    if (!qp)
    {
        free_tokens(ts);
        return 1;
    }

    Table *t = load_csv(qp->filename);
    if (!t)
    {
        free_query_plan(qp);
        free_tokens(ts);
        return 1;
    }

    execute(t, qp);

    free_table(t);
    free_query_plan(qp);
    free_tokens(ts);
    return 0;
}