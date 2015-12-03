#ifndef _RECURSIVE_LEARNING_UN_H
#define _RECURSIVE_LEARNING_UN_H


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"



void recursive_learning_un(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &orNodeList, int current_node, int r, int rmax, map<int, int> &MA_dir, multimap<int, FaultList> &untSet, int &consist_flag);



#endif
