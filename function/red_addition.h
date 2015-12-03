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
#include "../class/ConnFault.h"

#ifndef READ_ADDITION_H
#define READ_ADDITION_H


void red_addition(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, vector<ConnFault> &cf, int threshold, vector<string> &testPool, map<int, int> &truthTable, int maxLineNumber);


#endif 

