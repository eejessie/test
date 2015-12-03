/*
 * =====================================================================================
 *
 *       Filename:  idtf_red.h
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  11/08/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yi Wu
 *
 * =====================================================================================
 */
#ifndef IDTF_RED_H
#define IDTF_RED_H 
 
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


void idtf_red(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &masterNodeList_m, map<int, int> &truthTable, int outputLevel, vector<int> *levelSet, vector<int> &inputList);

#endif 

