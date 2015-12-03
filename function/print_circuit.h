/*
 * =====================================================================================
 *
 *       Filename:  print_circuit.h
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

#ifndef PRINT_CIRCUIT_H
#define PRINT_CIRCUIT_H


	void print_circuit(map <int, CircuitNode> &masterNodeList_m, ofstream &redFILE);
	void print_circuit(vector<CircuitNode> &masterNodeList, ofstream &redFILE);


#endif 

