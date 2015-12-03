/*
 * =====================================================================================
 *
 *       Filename:  copy_point_vector.h
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

#ifndef COPY_POINT_VECTOR_H
#define COPY_POINT_VECTOR_H

	int locate_in_vector(vector<CircuitNode> &masterNodeList, int line);
	void copy_point_vector1(map<int, CircuitNode>&tempOrNodeList, map<int, CircuitNode>&orNodeList_m);
	void copy_point_vector2(vector<CircuitNode> &tempOrNodeList, map<int, CircuitNode> &orNodeList_m);
	void copy_point_vector3(map<int, CircuitNode> &tempOrNodeList, vector<CircuitNode> &orNodeList);
	void copy_point_vector4(vector<CircuitNode> &tempOrNodeList, vector<CircuitNode> &orNodeList);

#endif 

