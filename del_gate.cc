/*
 * =====================================================================================
 *
 *       Filename:  del_gate.cc
 *
 *    Description:  
 *
 *        License:  
 *
 *        Version:  1.0
 *        Created:  5/13/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yi WU <eejessie@sjtu.edu.cn>
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

#include "class/CircuitNode.h"         // This class stores information about each node in the circuit.
#include "class/ConnFault.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif


using namespace std;
//using namespace nameSpace_ATPG;



void del_gate(map<int, CircuitNode> &comNodeList, int node)
{
	int retain_node;
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2;
	set<int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv, itrv1;

	//cout << "The node to be deleted is "<<node<<endl;

	itrm = comNodeList.find(node-1);
	for(itrs=itrm->second.listFanIn.begin(); itrs!=itrm->second.listFanIn.end(); itrs++)
		if(*itrs!=node)
		{	retain_node = *itrs;
			//cout << "The node to be retained is "<<retain_node<<endl;
			break;
		}
	
	itrm = comNodeList.find(node);       //delete node newID3
	comNodeList.erase(itrm);
	
	
	itrm = comNodeList.find(retain_node);  //node: newID5 or newID4
	itrm->second.pointFanOut.clear();
	itrm1 = comNodeList.find(node-1);    //node-1: newID2
	itrv = itrm1->second.pointFanOut.begin();        
	itrm2 = comNodeList.find((*itrv)->lineNumber);   //cf.dest
	itrm->second.pointFanOut.push_back(&itrm2->second);	

	//cout << "cf.dest = "<<itrm2->second.lineNumber<<endl;
	//cout << "retain_node is "<<retain_node<<endl;	
	//cout << "Before erasing one input: "<<endl;
	//itrs = itrm2->second.listFanIn.begin();
	//cout << *itrs << " "<<endl;
	itrm2->second.listFanIn.erase(node-1);
	
	//cout << "After erasing one input: "<<endl;
	//itrs = itrm2->second.listFanIn.begin();
	//cout << *itrs << " "<<endl;

	itrm2->second.listFanIn.insert(retain_node);

	itrm2 = comNodeList.find((*itrv)->lineNumber);   //cf.dest
	for(itrv1 = itrm2->second.pointFanIn.begin(); itrv1 != itrm2->second.pointFanIn.end(); itrv1++)
		if((*itrv1)->lineNumber == node-1)
		{
			itrv1 = itrm2->second.pointFanIn.erase(itrv1);
			break;
		}
	itrm2->second.pointFanIn.push_back(&itrm->second);

	comNodeList.erase(itrm1);                 //delete node newID2

	
}






