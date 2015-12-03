#ifndef _IMPLICATE_UN_H
#define _IMPLICATE_UN_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

int path_propagation_un(map<int, CircuitNode> &masterNodeList, int current_node, vector<int> &remaining_unjust_gates,  map<int, int> &MA);

#endif
