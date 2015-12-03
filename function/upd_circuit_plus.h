#ifndef _UPD_CIRCUIT_PLUS_H
#define _UPD_CIRCUIT_PLUS_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

void upd_circuit_plus(map<int, CircuitNode> &orNodeList, map<int, int> &wa_set, map<int, int> &ndp_set, int maxLineNumber);

#endif
