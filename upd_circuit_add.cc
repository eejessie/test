/*
 * =====================================================================================
 *
 *       Filename:  upd_circuit_add.cc
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
#include "class/FaultList.h"
#include "function/print_circuit.h"
#include "function/helper.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0;

int upd_circuit_add(map<int, CircuitNode> &comNodeList, struct ConnFault cf, int destType, FaultList &fault, int &maxLineNumber)
{
	map<int,CircuitNode>::iterator itrm, itrm1, itrm2, itrm3;
	map<int,CircuitNode>::iterator itrm_n1, itrm_n2, itrm_n3, itrm_n4, itrm_n5, itrm_n6;
	set<int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv, itrv1;
	//itrm = comNodeList.end();
	//itrm--;
	
	//cout<<"Before this update, there are "<<comNodeList.size()<<" nodes in comNodeList"<<endl;
	//cout<<"S = "<<cf.src<<", D = "<<cf.dest<<", P = "<<cf.pola<<endl;
	//addedNode1 is the node to extend the 2 inputs to 3 inputs;
	//addedNode2 is the node to transform the connection fault to stuck-at fault;
	//addedNode3 is the input to addedNode2.  
	CircuitNode *addedNode1, *addedNode2, *addedNode3, *addedNode4, *addedNode5, *addedNode6;
	addedNode1 = new CircuitNode(GT);
	addedNode2 = new CircuitNode(GT);
	addedNode3 = new CircuitNode(PI);
	addedNode4 = new CircuitNode(GT);
	addedNode5 = new CircuitNode(FB);
	addedNode6 = new CircuitNode(FB);

	int newID1 = maxLineNumber + 1;
	int newID2 = newID1 + 1;
	int newID3 = newID2 + 1;
	int newID4 = newID3 + 1;
	int newID5 = newID4 + 1;
	int newID6 = newID5 + 1;
	fault.lineNumber =  newID3;
	
	maxLineNumber += 6;

	addedNode1->lineNumber = newID1;
	addedNode2->lineNumber = newID2;
	addedNode3->lineNumber = newID3;
	addedNode4->lineNumber = newID4;
	addedNode5->lineNumber = newID5;
	addedNode6->lineNumber = newID6;

	addedNode1->internal = true;
	addedNode2->internal = false;
	addedNode3->internal = false;
	addedNode4->internal = false;
	addedNode5->internal = false;
	addedNode6->internal = false;

	addedNode1->circuitIndex = 1;
	addedNode2->circuitIndex = 1;
	addedNode3->circuitIndex = 1;	
	addedNode4->circuitIndex = 1;
	addedNode5->circuitIndex = 1;
	addedNode6->circuitIndex = 1;


	//Insert these nodes into initial circuit.
	comNodeList.insert(pair<int, CircuitNode>(newID1, *addedNode1));
	comNodeList.insert(pair<int, CircuitNode>(newID2, *addedNode2));
	comNodeList.insert(pair<int, CircuitNode>(newID3, *addedNode3));
	comNodeList.insert(pair<int, CircuitNode>(newID5, *addedNode5));
	comNodeList.insert(pair<int, CircuitNode>(newID6, *addedNode6));
	if(cf.pola)
		comNodeList.insert(pair<int, CircuitNode>(newID4, *addedNode4));	

	//Define special iterators for added nodes.
	itrm_n1 = comNodeList.find(newID1);     //itrm_n1 : addedNode1
	itrm_n2 = comNodeList.find(newID2);     //itrm_n2 : addedNode2
	itrm_n3 = comNodeList.find(newID3);     //itrm_n3 : addedNode3
	itrm_n4 = comNodeList.find(newID4);     //itrm_n4 : addedNode4
	itrm_n5 = comNodeList.find(newID5);     //itrm_n5 : addedNode5
	itrm_n6 = comNodeList.find(newID6);     //itrm_n6 : addedNode6

	//update addedNode1
	itrm = comNodeList.find(cf.dest);          //itrm: cf.dest
	itrm_n1->second.pointFanOut.push_back(&itrm->second);
	itrm_n1->second.numberFanIn = 2;
	itrm_n1->second.numberFanOut = 1;
	for(itrs=itrm->second.listFanIn.begin(); itrs!=itrm->second.listFanIn.end(); itrs++)
	{
		itrm_n1->second.listFanIn.insert(*itrs);
		itrm1 = comNodeList.find(*itrs);
		itrm_n1->second.pointFanIn.push_back(&itrm1->second);
		itrm1->second.pointFanOut.clear();
		itrm1->second.pointFanOut.push_back(&itrm_n1->second);
	}

	//update cf.dest
	itrm->second.listFanIn.clear();
	itrm->second.listFanIn.insert(newID1);
	itrm->second.listFanIn.insert(newID2);
	itrm->second.pointFanIn.clear();
	itrm->second.pointFanIn.push_back(&itrm_n1->second);
	itrm->second.pointFanIn.push_back(&itrm_n2->second);

	//update addedNode2
	itrm_n2->second.pointFanOut.push_back(&itrm->second);
	itrm_n2->second.listFanIn.insert(newID3);
	itrm_n2->second.pointFanIn.push_back(&itrm_n3->second);
	itrm_n2->second.numberFanIn = 2;
	itrm_n2->second.numberFanOut = 1;
	if(!cf.pola)
	{
		itrm_n2->second.listFanIn.insert(newID5);
		itrm_n2->second.pointFanIn.push_back(&itrm_n5->second);
	}
	else
	{
		itrm_n2->second.listFanIn.insert(newID4);
		itrm_n2->second.pointFanIn.push_back(&itrm_n4->second);
	}		
		
	//update addedNode3
	itrm_n3->second.gateType = 0;
	itrm_n3->second.pointFanOut.push_back(&itrm_n2->second);
	itrm_n3->second.numberFanIn = 0;
	itrm_n3->second.numberFanOut = 1;

	//update addedNode4
	if(cf.pola)
	{
		itrm_n4->second.gateType = G_NOT;		
		itrm_n4->second.listFanIn.insert(newID5);
		itrm_n4->second.pointFanIn.push_back(&itrm_n5->second);
		itrm_n4->second.pointFanOut.push_back(&itrm_n2->second);
		itrm_n4->second.numberFanIn = 1;
		itrm_n4->second.numberFanOut = 1;
	}

	//update addedNode5
	itrm2 = comNodeList.find(cf.src);             //itrm2: cf.src
	itrm_n5->second.gateType = G_BRNCH;
	itrm_n5->second.listFanIn.insert(cf.src);	
	itrm_n5->second.pointFanIn.push_back(&itrm2->second);
	itrm_n5->second.numberFanIn = 1;
	itrm_n5->second.numberFanOut = 1;
	if(cf.pola)
	{
		itrm_n5->second.pointFanOut.push_back(&itrm_n4->second);
	}
	else
	{
		itrm_n5->second.pointFanOut.push_back(&itrm_n2->second);
	}

	//update addedNode6
	itrm_n6->second.gateType = G_BRNCH;
	itrm_n6->second.listFanIn.insert(cf.src);
	itrm_n6->second.pointFanIn.push_back(&itrm2->second);
	itrm_n6->second.numberFanIn = 1;
	itrm_n6->second.numberFanOut = 1;
	itrm_n6->second.pointFanOut = itrm2->second.pointFanOut;

	//Updated the original fanout nodes of cf.src
	for(itrv = itrm2->second.pointFanOut.begin(); itrv !=itrm2->second.pointFanOut.end(); itrv++)
	{		
		itrm3 = comNodeList.find((*itrv)->lineNumber);
		itrm3->second.listFanIn.erase(cf.src);
		itrm3->second.listFanIn.insert(newID6);
		vector_erase(itrm3->second, cf.src, 1);
		itrm3->second.pointFanIn.push_back(&itrm_n6->second);
	}
	
    //Update cf.src
	itrm2->second.pointFanOut.clear();
	itrm2->second.pointFanOut.push_back(&itrm_n5->second);
	itrm2->second.pointFanOut.push_back(&itrm_n6->second);

	switch(destType){
		//The destination gate is an OR gate
		case(G_OR):
			itrm_n1->second.gateType = G_OR;		
			itrm_n2->second.gateType = G_AND;
			fault.stuckAtValue = 1;			
			break;
			
		//The destination gate is a NOR gate
		case(G_NOR):
			itrm_n1->second.gateType = G_OR;
			itrm->second.gateType = G_NOR;
			itrm_n2->second.gateType = G_AND;	
			fault.stuckAtValue = 1;			
			break;

		//The destination gate is an AND gate
		case(G_AND):
			itrm_n1->second.gateType = G_AND;
			itrm_n2->second.gateType = G_OR;
			fault.stuckAtValue = 0;
			break;

		//The destination gate is a NAND gate
		case(G_NAND):
			itrm_n1->second.gateType = G_AND;		
			itrm->second.gateType = G_NAND;
			itrm_n2->second.gateType = G_OR;	
			fault.stuckAtValue = 0;
			break;
	}

	
	delete addedNode1;
	delete addedNode2;
	delete addedNode3;
	delete addedNode4;
	delete addedNode5;
	delete addedNode6;

	//print_circuit(comNodeList);

	return newID1;

}

