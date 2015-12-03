/*
 * =====================================================================================
 *
 *       Filename:  del_gate.h
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

#ifndef DEL_GATE_H
#define DEL_GATE_H


	void del_gate(map<int, CircuitNode> &comNodeList, int node);

#endif 

