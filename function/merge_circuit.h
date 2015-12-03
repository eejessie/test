/*
 * =====================================================================================
 *
 *       Filename:  merge_circuit.h
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

#ifndef MERGE_CIRCUIT_H
#define MERGE_CIRCUIT_H


	int merge_circuit(map<int, CircuitNode> &masterNodeList_m, map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m);


#endif 

