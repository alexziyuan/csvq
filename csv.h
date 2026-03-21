#ifndef CSV_H
#define CSV_H

#include "table.h"

Table *load_csv(const char *filename);
void free_table(Table *t);

#endif