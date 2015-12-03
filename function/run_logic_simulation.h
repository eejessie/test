
#ifndef RUN_LOGIC_SIMULATION_H
#define RUN_LOGIC_SIMULATION_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <set>
#include <vector>
#include <map>

#include "../class/CircuitNode.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cudd.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cuddInt.h"


void run_logic_simulation(map<int, CircuitNode> &orNodeList, int totalInputs, vector<string> & InputVector, DdManager *dd, DdNode *dnode);

#endif 

