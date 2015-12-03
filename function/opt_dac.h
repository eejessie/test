#ifndef _OPT_DAC_H
#define _OPT_DAC_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

void opt_dac(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m);

#endif
