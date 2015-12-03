/**CFile***********************************************************************

  FileName    [cudd_comp.c]

  SeeAlso     []

  Author      [Fabio Somenzi]

  Copyright   [Copyright (c) 1995-2004, Regents of the University of Colorado

  All rights reserved.

******************************************************************************/

#include "ntr.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cuddInt.h"


#define BUFLENGTH 8192

#ifndef lint
static char rcsid[] UTIL_UNUSED = "$Id: main.c,v 1.38 2004/08/13 18:28:28 fabio Exp fabio $";
#endif

static  char    buffer[BUFLENGTH];
#ifdef DD_DEBUG
extern  st_table *checkMinterms (BnetNetwork *net, DdManager *dd, st_table *previous);
#endif


int Eval_Value(DdManager * dd, DdNode * f, int * inputs)
{
    int comple;
    DdNode *ptr;

    comple = Cudd_IsComplement(f);
    comple = 1- comple;
    ptr = Cudd_Regular(f);
    
    while(Cudd_IsNonConstant(ptr)){
	if (inputs[ptr->index] == 1) {
	    ptr = cuddT(ptr);
	} else {
	    comple ^= Cudd_IsComplement(cuddE(ptr));
	    ptr = Cudd_Regular(cuddE(ptr));
	}
    }
    return comple;

} /* end of Cudd_Eval */

int cudd_comp(DdManager *dd, DdNode *node, int num_inputs, int *assign)
{
 /*   printf("current assign: \n");
    int i;
    for(i = 0; i < num_inputs; i++)
        printf("%d ", assign[i]);*/
    int res = Eval_Value(dd, node, assign);
/*    if(res == 1)
        printf("value = 1\n");
    else
        printf("value = 0\n");  */

    return res;      
}


