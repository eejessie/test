/*
 * =====================================================================================
 *
 *       Filename:  red_removal.h
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

#ifndef RED_REMOVAL_H
#define RED_REMOVAL_H


	void red_removal(map<int, CircuitNode> &buOrNodeList_m, map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m, vector<string> &masterInputVector, vector<int> &inputList, int ini_size);


#endif 

