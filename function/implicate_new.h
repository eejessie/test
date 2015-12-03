#ifndef _IMPLICATE_NEW_H
#define _IMPLICATE_NEW_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

int path_propagation_new(map<int, CircuitNode> &masterNodeList, FaultList wt_error, int current_node, vector<int> &remaining_unjust_gates, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced, int flag_ao);

#endif
