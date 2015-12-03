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


#ifndef GEN_CF_H
#define GEN_CF_H

int gen_cf(map<int, CircuitNode> &orNodeList_m, vector<ConnFault> &cf, int totalInputs);

#endif 

