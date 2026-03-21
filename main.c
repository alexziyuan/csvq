#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "csv.h"
#include "lexer.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s <file.csv> <query>\n", argv[0]);
        return 1;
    }

    Table *t = load_csv(argv[1]);
    if (!t)
        return 1;

    printf("=== TABLE ===\n");
    for (int i = 0; i < t->ncols; i++)
        printf("%-16s", t->headers[i]);
    printf("\n");
    for (int r = 0; r < t->nrows; r++)
    {
        for (int c = 0; c < t->ncols; c++)
            printf("%-16s", t->rows[r].values[c]);
        printf("\n");
    }

    printf("\n=== TOKENS ===\n");
    TokenStream *ts = tokenize(argv[2]);
    for (int i = 0; i < ts->count; i++)
        printf("[%2d]  %s\n", ts->tokens[i].type, ts->tokens[i].text);

    free_table(t);
    free_tokens(ts);
    return 0;
}