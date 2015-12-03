/*
 * =====================================================================================
 *
 *       Filename:  red_forward_implication_v3.h
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

#ifndef RED_FORWARD_IMPLICATION_V3_H
#define RED_FORWARD_IMPLICATION_V3_H 
 
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
#include "../class/HashTable.h"


void red_forward_implication_v3(map <int, CircuitNode> &masterNodeList, HashTable &implicationNodeList, int inLineNumber, bool inStuckAtValue);

#endif 

