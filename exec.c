#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exec.h"
#include "eval.h"

/* ---- helpers ---- */

static int grp_col_idx; /* used by qsort comparator */
static int cmp_rows(const void *a, const void *b)
{
    const Row *ra = (const Row *)a;
    const Row *rb = (const Row *)b;
    return strcmp(ra->values[grp_col_idx], rb->values[grp_col_idx]);
}

void execute(Table *t, QueryPlan *qp)
{
    /* 1. filter */
    Row *filtered = malloc(sizeof(Row) * t->nrows);
    if (!filtered)
    {
        fprintf(stderr, "error: out of memory\n");
        return;
    }
    int nfiltered = 0;

    for (int r = 0; r < t->nrows; r++)
    {
        if (eval_expr(t, &t->rows[r], qp->where))
            filtered[nfiltered++] = t->rows[r];
    }

    /* 2. resolve SELECT col indices */
    /* ncols==0 means SELECT * — expand to all columns */
    SelectCol cols[MAX_COLS];
    int ncols = qp->ncols;

    if (ncols == 0)
    {
        ncols = t->ncols;
        for (int i = 0; i < ncols; i++)
        {
            strncpy(cols[i].name, t->headers[i], 63);
            cols[i].agg = AGG_NONE;
        }
    }
    else
    {
        for (int i = 0; i < ncols; i++)
            cols[i] = qp->cols[i];
    }

    /* 3. no GROUP BY - project and print */
    if (qp->groupby[0] == '\0')
    {
        /* print header */
        for (int i = 0; i < ncols; i++)
            printf("%-16s", cols[i].name);
        printf("\n");
        for (int r = 0; r < nfiltered; r++)
        {
            for (int i = 0; i < ncols; i++)
            {
                int ci = find_col(t, cols[i].name);
                printf("%-16s", (ci >= 0) ? filtered[r].values[ci] : "?");
            }
            printf("\n");
        }
        free(filtered);
        return;
    }

    /* 4. GROUP BY */
    int grp_col = find_col(t, qp->groupby);
    if (grp_col < 0)
    {
        fprintf(stderr, "error: GROUP BY column '%s' not found\n", qp->groupby);
        free(filtered);
        return;
    }

    /* sort by group column */
    grp_col_idx = grp_col;
    qsort(filtered, nfiltered, sizeof(Row), cmp_rows);

    /* print header */
    for (int i = 0; i < ncols; i++)
    {
        char label[80];
        if (cols[i].agg == AGG_COUNT)
            snprintf(label, sizeof(label), "COUNT(%s)", cols[i].name);
        else if (cols[i].agg == AGG_SUM)
            snprintf(label, sizeof(label), "SUM(%s)", cols[i].name);
        else if (cols[i].agg == AGG_AVG)
            snprintf(label, sizeof(label), "AVG(%s)", cols[i].name);
        else if (cols[i].agg == AGG_MIN)
            snprintf(label, sizeof(label), "MIN(%s)", cols[i].name);
        else if (cols[i].agg == AGG_MAX)
            snprintf(label, sizeof(label), "MAX(%s)", cols[i].name);
        else
            snprintf(label, sizeof(label), "%s", cols[i].name);
        printf("%-16s", label);
    }
    printf("\n");

    /* iterate groups */
    int start = 0;
    while (start < nfiltered)
    {
        int end = start + 1;
        while (end < nfiltered &&
               strcmp(filtered[end].values[grp_col],
                      filtered[start].values[grp_col]) == 0)
            end++;

        for (int i = 0; i < ncols; i++)
        {
            int ci = find_col(t, cols[i].name);
            char cell[64];

            if (cols[i].agg == AGG_NONE)
            {
                snprintf(cell, sizeof(cell), "%s",
                         ci >= 0 ? filtered[start].values[ci] : "?");
            }
            else if (cols[i].agg == AGG_COUNT)
            {
                snprintf(cell, sizeof(cell), "%d", end - start);
            }
            else if (cols[i].agg == AGG_SUM || cols[i].agg == AGG_AVG)
            {
                double sum = 0.0;
                int cnt = 0;
                for (int r = start; r < end; r++)
                {
                    if (ci < 0)
                        continue;
                    char *endp;
                    double v = strtod(filtered[r].values[ci], &endp);
                    if (endp != filtered[r].values[ci])
                    {
                        sum += v;
                        cnt++;
                    }
                }
                if (cols[i].agg == AGG_SUM)
                    snprintf(cell, sizeof(cell), "%.4g", sum);
                else
                    snprintf(cell, sizeof(cell), "%.4g", cnt ? sum / cnt : 0.0);
            }

            else if (cols[i].agg == AGG_MIN || cols[i].agg == AGG_MAX)
            {
                double result = 0.0;
                int first = 1;
                for (int r = start; r < end; r++)
                {
                    if (ci < 0)
                        continue;
                    char *endp;
                    double v = strtod(filtered[r].values[ci], &endp);
                    if (endp != filtered[r].values[ci])
                    {
                        if (first || (cols[i].agg == AGG_MIN && v < result) || (cols[i].agg == AGG_MAX && v > result))
                            result = v;
                        first = 0;
                    }
                }
                snprintf(cell, sizeof(cell), "%.4g", result);
            }
            printf("%-16s", cell);
        }
        printf("\n");
        start = end;
    }
    free(filtered);
}