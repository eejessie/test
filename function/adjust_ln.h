/*
 * =====================================================================================
 *
 *       Filename:  adjust_ln.h
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
#ifndef ADJUST_LN_H
#define ADJUST_LN_H 
 
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


	void adjust_ln(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, vector<int> &inputList, vector<int> &outputList);

#endif 

