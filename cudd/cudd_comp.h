#ifndef CUDD_COMP_H
#define CUDD_COMP_H

#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cudd.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cuddInt.h"

int cudd_comp(DdManager *dd, DdNode *node, int num_inputs, int *assign);

#endif
