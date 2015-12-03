#ifndef _ATPG_CHECKER_H
#define _ATPG_CHECKER_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

bool CheckVectorForATPG (map<int, CircuitNode> &masterNodeList, int totalInputs, int inLineNumber, bool inStuckAtValue, string inVector);

bool TestAllVectorsATPG (map<int, CircuitNode> &masterNodeList, int totalInputs, int inLineNumber, bool inStuckAtValue, vector<string> &inVectorList);
#endif
