#ifndef _OPT_RAR_SAT_H
#define _OPT_RAR_SAT_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

void opt_rar_sat(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &orFaultList, vector<string> &br_lisp_log);

#endif
