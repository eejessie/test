#ifndef REM_RED_H
#define REM_RED_H


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>


#include "../class/CircuitNode.h" 
#include "../class/FaultList.h"          

    void rem_max_red(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &original_sf, multimap<int, FaultList> &cost_sf, vector<string> &testPool, int outputLevel, vector<int> *levelSet, int &first_cost);

	void rem_red(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m, vector <FaultList> &orFaultList);



#endif 

