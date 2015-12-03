#ifndef _COMPUTE_MA_OR_H
#define _COMPUTE_MA_OR_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

int compute_MA_or(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &masterNodeList, FaultList & wt_error, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced);

#endif
