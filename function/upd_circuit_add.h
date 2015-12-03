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
#include "../class/FaultList.h"
#include "../class/ConnFault.h"


#ifndef UPD_CIRCUIT_ADD_H
#define UPD_CIRCUIT_ADD_H


	int upd_circuit_add(map<int, CircuitNode> &comNodeList, struct ConnFault cf, int destType, FaultList &fault, int &maxLineNumber);


#endif 

