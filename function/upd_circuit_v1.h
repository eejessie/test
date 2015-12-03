#ifndef _UPD_CIRCUIT_V1_H
#define _UPD_CIRCUIT_V1_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

void upd_circuit_v1(map<int, CircuitNode> &orNodeList, map<int, int> &ns_set, map<int, int> &nd_set, int maxLineNumber);

#endif
