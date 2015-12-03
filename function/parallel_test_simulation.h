/*
 * =====================================================================================
 *
 *       Filename:  parallel_test_simulation.h
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


#ifndef PARALLEL_TEST_SIMULATION_H
#define PARALLEL_TEST_SIMULATION_H


map<int, vector<int> > parallel_test_simulation(map<int, CircuitNode> &orNodeList, FaultList &sf, vector<string> &testVector, int outputLevel, vector<int> *levelSet);


#endif 

