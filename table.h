#ifndef TABLE_H
#define TABLE_H

#define MAX_COLS 64
#define MAX_ROWS 8192
#define MAX_FIELD 256

typedef struct
{
    char values[MAX_COLS][MAX_FIELD];
    int ncols;
} Row;

typedef struct
{
    char headers[MAX_COLS][64];
    Row rows[MAX_ROWS];
    int ncols;
    int nrows;
} Table;

#endif