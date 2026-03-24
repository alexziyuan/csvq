#ifndef EXEC_H
#define EXEC_H

#include "table.h"
#include "query.h"

void execute(Table *t, QueryPlan *qp);

#endif