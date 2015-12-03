/*
 * =====================================================================================
 *
 *       Filename:  atpg.h
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
 
#ifndef ATPG_H
#define ATPG_H


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


#include "../class/CircuitNode.h" 
#include "../class/CircuitLine.h" 
#include "../class/FaultList.h"   
#include "../class/queue.h"       



	int SetLineLevel (map <int, CircuitNode> &masterNodeList, vector<int> &inputList);
	int SetLineLevel_new(map <int, CircuitNode> &masterNodeList, vector<int> &inputList);

	int SimpleLogicSimulation (map <int, CircuitNode> &masterNodeList, vector<int> &inputList, vector<int> &inVector, int outputLevel, FaultList &objFault, vector<int> *levelSet, map<int, int> &MA_value);
	
	void find_fanout_gates(map<int, CircuitNode> &masterNodeList_m, int line, queue &Q);
	
	void logic_simulation_helper(map<int, CircuitNode> &masterNodeList_m, queue &Q, string newVector, FaultList &objFault);
	
	int EdSimpleLogicSimulation (map <int, CircuitNode> &masterNodeList, vector<int> &inputList, string inVector, int outputLevel, FaultList &objFault);
	
	void LogicSimulation (map <int, CircuitNode> &masterNodeList, int totalInputs, string inVector, int outputLevel, vector<int> *levelSet, map<int, int> &outputValue);
//    void LogicSimulation (map <int, CircuitNode> &masterNodeList, int totalInputs, string inVector, int outputLevel, vector<int> *levelSet, int &outValue);
	
	void CreateFaultObjects (map <int, CircuitLine> &masterLineList, map<int, CircuitNode> &masterNodeList);
	
	void CollapseFaults (map <int, CircuitLine> &masterLineList, map<int, CircuitNode> &masterNodeList);
	
	void CreateFaultList(map<int, CircuitLine> &masterLineList, vector<FaultList> &inFaultList, map<int, CircuitNode> &masterNodeList); 



#endif 

