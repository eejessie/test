#ifndef _UPD_CIRCUIT_V2_H
#define _UPD_CIRCUIT_V2_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

void upd_circuit_v2(map<int, CircuitNode> &orNodeList, map<int, int> &ns_set, map<int, int> &nd_set, int sa_line, int maxLineNumber);

#endif
