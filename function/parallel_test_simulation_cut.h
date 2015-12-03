/*
 * =====================================================================================
 *
 *       Filename:  parallel_test_simulation_cut.h
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
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cudd.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cuddInt.h"


#ifndef PARALLEL_TEST_SIMULATION_CUT_H
#define PARALLEL_TEST_SIMULATION_CUT_H


vector<int> parallel_test_simulation_cut(map<int, CircuitNode> &orNodeList, FaultList &sf, vector<string> &testVector, map<int, int> &truthTable, int outputLevel, vector<int> *levelSet);

#endif 

