#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

static int parse_line(char *line, Row *row)
{
    int col = 0;
    char *p = line;
    while (*p && col < MAX_COLS)
    {
        char field[MAX_FIELD];
        int fi = 0;
        int quoted = (*p == '"');
        if (quoted)
            p++; // skip opening quote

        while (*p)
        {
            if (quoted)
            {
                if (*p == '"' && *(p + 1) == '"')
                {
                    /* escaped quote: "" → " */
                    field[fi++] = '"';
                    p += 2;
                    continue;
                }
                if (*p == '"')
                {
                    p++;
                    break;
                }
            }
            else
            {
                if (*p == ',' || *p == '\n' || *p == '\r')
                    break;
            }
            if (fi < MAX_FIELD - 1)
                field[fi++] = *p;
            p++;
        }

        field[fi] = '\0';
        strncpy(row->values[col], field, MAX_FIELD - 1);
        col++;

        if (*p == ',')
            p++;
        else if (!quoted && (*p == ',' || *p == '\r'))
            p++;
        else if (*p == ',')
            p++;
    }
    row->ncols = col;
    return col;
}

Table *load_csv(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr, "error: cannot open '%s'\n", filename);
        return NULL;
    }

    Table *t = calloc(1, sizeof(Table));
    if (!t)
    {
        fclose(fp);
        return NULL;
    }

    char line[MAX_COLS * MAX_FIELD];

    if (!fgets(line, sizeof(line), fp))
    {
        fclose(fp);
        return t;
    }
    Row header_row;
    parse_line(line, &header_row);
    t->ncols = header_row.ncols;
    for (int i = 0; i < t->ncols; i++)
        strncpy(t->headers[i], header_row.values[i], 63);

    while (fgets(line, sizeof(line), fp) && t->nrows < MAX_ROWS)
    {
        if (line[0] == '\n' || line[0] == '\r')
            continue;
        parse_line(line, &t->rows[t->nrows]);
        t->nrows++;
    }

    fclose(fp);
    return t;
}

void free_table(Table *t)
{
    free(t);
}