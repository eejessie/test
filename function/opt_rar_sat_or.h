#ifndef _OPT_RAR_SAT_OR_H
#define _OPT_RAR_SAT_OR_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cudd.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cuddInt.h"

void opt_rar_sat_or(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &orFaultList, DdManager *dd, DdNode *dnode, vector<string> &br_lisp_log);

#endif
