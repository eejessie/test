/*
 * =====================================================================================
 *
 *       Filename:  copy_point_vector.cc
 *
 *    Description:  
 *
 *        License:  
 *
 *        Version:  1.0
 *        Created:  3/07/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yi Wu <eejessie@sjtu.edu.cn>
 *         
 * =====================================================================================
 */

// #####   HEADER FILE INCLUDES   ################################################### 

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
#include <utility>

#include "class/CircuitNode.h"
#include "class/ConnFault.h"


#include "function/print_circuit.h" 
#include "function/run_logic_simulation.h"
#include "function/helper.h"


// Global constant definitions.
#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0, redFILE1;


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SimpleLogicSimulation
 *  Description:  Simple logic simulation. The number of inputs are provided to it.
 *                It generates a random input vector, simulates the circuit and then 
 *                generates the output value.
 * =====================================================================================
 */


void copy_point_vector1(map<int, CircuitNode> &tempOrNodeList, map<int, CircuitNode> &orNodeList_m)
{
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3;
	vector<CircuitNode*>::iterator itrv, itrv1;
	set <int>::iterator itrs;

	
	orNodeList_m.clear();
	for(itrm = tempOrNodeList.begin(); itrm != tempOrNodeList.end(); itrm++)
		orNodeList_m.insert(pair<int, CircuitNode>(itrm->first, itrm->second));

	for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
	{
		//redFILE1 << "current node: "<<itrm->second.lineNumber<<endl;
		itrm->second.pointFanIn.clear();
		for(itrs = itrm->second.listFanIn.begin(); itrs != itrm->second.listFanIn.end(); itrs++)
		{
			itrm1 = orNodeList_m.find(*itrs);
			if(itrm1 != orNodeList_m.end())
				itrm->second.pointFanIn.push_back(&itrm1->second);
		}

		itrm->second.pointFanOut.clear();
		itrm2 = tempOrNodeList.find(itrm->second.lineNumber);   //corresponding node in tempOrNodeList.
		for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++)
		{
			//redFILE1 << "current node's fanout: "<<(*itrv)->lineNumber<<endl;
			itrm3 = orNodeList_m.find((*itrv)->lineNumber);
			if(itrm3 != orNodeList_m.end())
				itrm->second.pointFanOut.push_back(&itrm3->second);
		}
	}
	
}

void copy_point_vector2(vector<CircuitNode> &tempOrNodeList, map<int, CircuitNode> &orNodeList_m)
{
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2;
	vector<CircuitNode*>::iterator itrv1;
	vector<CircuitNode>::iterator itrv;
	set <int>::iterator itrs;

	orNodeList_m.clear();

	for(itrv = tempOrNodeList.begin(); itrv != tempOrNodeList.end(); itrv++)
		orNodeList_m.insert(pair<int, CircuitNode>((*itrv).lineNumber, (*itrv)));


	int loc;
	for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
	{
		itrm->second.pointFanIn.clear();
		for(itrs = itrm->second.listFanIn.begin(); itrs != itrm->second.listFanIn.end(); itrs++)
		{
			itrm1 = orNodeList_m.find(*itrs);
			itrm->second.pointFanIn.push_back(&itrm1->second);
		}

		itrm->second.pointFanOut.clear();
		loc = search_vector_node(tempOrNodeList, itrm->second.lineNumber);         //corresponding node in tempOrNodeList.
		for(itrv1 = tempOrNodeList[loc].pointFanOut.begin(); itrv1 != tempOrNodeList[loc].pointFanOut.end(); itrv1++)
		{
			itrm2 = orNodeList_m.find((*itrv1)->lineNumber);
			itrm->second.pointFanOut.push_back(&itrm2->second);
		}
	}
}

void copy_point_vector3(map<int, CircuitNode> &tempOrNodeList, vector<CircuitNode> &orNodeList)
{
	map<int, CircuitNode>::iterator itrm, itrm2, itrm3;
	vector<CircuitNode*>::iterator itrv;
	vector<CircuitNode>::iterator itrv0, itrv1;
	set <int>::iterator itrs;

	orNodeList.clear();

	for(itrm = tempOrNodeList.begin(); itrm != tempOrNodeList.end(); itrm++)
		orNodeList.push_back(itrm->second);


	int loc;
	for(itrv0 = orNodeList.begin(); itrv0 != orNodeList.end(); itrv0++)
	{
		(*itrv0).pointFanIn.clear();
		for(itrs = (*itrv0).listFanIn.begin(); itrs != (*itrv0).listFanIn.end(); itrs++)
		{
			loc = search_vector_node(orNodeList, *itrs);
			(*itrv0).pointFanIn.push_back(&orNodeList[loc]);
		}

		(*itrv0).pointFanOut.clear();
		itrm2 = tempOrNodeList.find((*itrv0).lineNumber);   //corresponding node in tempOrNodeList.
		for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++)
		{
			loc = search_vector_node(orNodeList, (*itrv)->lineNumber);
			(*itrv0).pointFanOut.push_back(&orNodeList[loc]);
		}
	}
}

void copy_point_vector4(vector<CircuitNode> &tempOrNodeList, vector<CircuitNode> &orNodeList)
{
	//map<int, CircuitNode>::iterator itrm, itrm2, itrm3;
	vector<CircuitNode*>::iterator itrv;
	vector<CircuitNode>::iterator itrv0;
	set <int>::iterator itrs;

	orNodeList.clear();

	for(itrv0 = tempOrNodeList.begin(); itrv0 != tempOrNodeList.end(); itrv0++)
		orNodeList.push_back((*itrv0));

	int loc, loc1;
	for(itrv0 = orNodeList.begin(); itrv0 != orNodeList.end(); itrv0++)
	{
		(*itrv0).pointFanIn.clear();
		for(itrs = (*itrv0).listFanIn.begin(); itrs != (*itrv0).listFanIn.end(); itrs++)
		{
			loc = search_vector_node(tempOrNodeList, *itrs);
			(*itrv0).pointFanIn.push_back(&tempOrNodeList[loc]);
			
		}

		(*itrv0).pointFanOut.clear();
		loc = search_vector_node(tempOrNodeList, (*itrv0).lineNumber); //corresponding node in tempOrNodeList.
		for(itrv = tempOrNodeList[loc].pointFanOut.begin(); itrv != tempOrNodeList[loc].pointFanOut.end(); itrv++)
		{
			loc1 = search_vector_node(orNodeList, (*itrv)->lineNumber);
			(*itrv0).pointFanOut.push_back(&orNodeList[loc1]);
		}
	}
}





