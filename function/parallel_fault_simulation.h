/*
 * =====================================================================================
 *
 *       Filename:  parallel_fault_simulation.h
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  16/07/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yi Wu
 *
 * =====================================================================================
 */
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <cassert>

#include "../class/CircuitNode.h"
#include "../class/FaultList.h"
#include "../class/ConnFault.h"


#ifndef PARALLEL_FAULT_SIMULATION_H
#define PARALLEL_FAULT_SIMULATION_H


void parallel_fault_simulation(map<int, CircuitNode> &masterNodeList, vector<FaultList> &sf, string testVector, vector<int> &testRecord);


#endif 

