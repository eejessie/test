
#ifndef CUT_ENUM_MAIN_H
#define CUT_ENUM_MAIN_H 
 
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
#include <cassert>

#include "../class/CircuitNode.h"


	void cut_enum_main(map<int, CircuitNode> &orNodeList,  vector<int> &last_ef_node, vector<vector<int> > &cut_set, int th_num);

#endif 

