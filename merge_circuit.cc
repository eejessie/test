/*
 * =====================================================================================
 *
 *       Filename:  merge_circuit.cc
 *
 *    Description:  This function merges the original circuit and the boolean relation 
 *					circuit
 *
 *        Version:  1.0
 *        Created:  2/07/2014
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
#include <utility>
#include <ctime>
#include <sys/timeb.h>

#include "class/CircuitNode.h"  
#include "class/HashTable.h"     
#include "function/print_circuit.h" 
#include "function/copy_point_vector.h"
#include "function/helper.h"

// Global constant definitions.
#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern int numPI;
extern int numPO;
extern vector<int> inputList;
extern vector<int> outputList;
extern vector<int> brInputList; 
extern ofstream redFILE0, redFILE1;
extern HashTable opl_hash;


//merge_circuit
int merge_circuit(map<int, CircuitNode> &masterNodeList_m, map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m)
{
	//Iterators
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;
	map<int, CircuitNode*>::iterator itrm_p, itrm_p1, itrm_p2, itrm_p3;
	map<int, int>::iterator itrmi;
	set<int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv, itrv1;
	vector<CircuitNode>::iterator itrv0;
	
	//Variables
	int maxLineNumber=0, maxLineNumber1=0, maxLineNumber2=0;
	map<int, CircuitNode> tempOrNodeList_m;
	map<int, CircuitNode> tempBrNodeList_m;
	

	itrm = orNodeList_m.end();
	itrm--;
	maxLineNumber1=itrm->second.lineNumber;
	
	itrm1 = brNodeList_m.end();
	itrm1--;
	maxLineNumber2=itrm1->second.lineNumber;


	if(maxLineNumber1 > maxLineNumber2)
		maxLineNumber = maxLineNumber1;
	else
		maxLineNumber = maxLineNumber2;


	//Find nodes in brNodeList which have same line number as nodes in orNodeList, and change it to another number.
	//Then update the corresponding fanin and fanout list (change_lineNumber).
/*	int flag = 0;
	for(itrm = brNodeList_m.begin(); itrm != brNodeList_m.end(); itrm++)
	{
	    //If the current node is the primary output of the original circuit, then skip.
		int line = itrm->second.lineNumber;
		int p;
		if(opl_hash.Search_Hash(line, p) == 1)
		    continue;
		
		itrm1 = orNodeList_m.find(line);
		if(itrm1 != orNodeList_m.end())
		{					
			redFILE0 << cout << "In brNodeList_m, node "<<line <<" has same number with node "<<itrm1->second.lineNumber <<"in orNodeList_m."<<endl;
			itrm->second.lineNumber = maxLineNumber + 1;
			maxLineNumber = maxLineNumber + 1;
		}
	}*/
	
	//Get the input list of the BR circuit.
/*	vector<int> brInputList;
	redFILE1 << "brInputList:"<<endl;
	for(itrm = brNodeList_m.begin(); itrm != brNodeList_m.end(); itrm++)
	{
		if(itrm->second.nodeType == 1)              //PI
		{
			brInputList.push_back(itrm->second.lineNumber);
			redFILE1 << itrm->second.lineNumber<<" ";
		}
	}
	redFILE1 << endl;*/
	
	copy_point_vector1(orNodeList_m, tempOrNodeList_m);
	copy_point_vector1(brNodeList_m, tempBrNodeList_m);
	
	//Update the fanins and fanouts of current nodes and add some branches.	
	for(int i=0; i<inputList.size(); i++)
	{
		itrm2 = tempOrNodeList_m.find(inputList[i]);                //itrm2: inputList[i]
		
		//1. Add the branch node.
		CircuitNode *addedNode;
		addedNode = new CircuitNode(FB);
		addedNode->lineNumber = maxLineNumber + 1;
		addedNode->rectFlag = 0;
		
		tempOrNodeList_m.insert(pair<int, CircuitNode>(addedNode->lineNumber, *addedNode));
		
		itrm1 = tempOrNodeList_m.find(addedNode->lineNumber);   //itrm1: the added node
		itrm1->second.gateType = 1;           //FB		
		itrm1->second.listFanIn.insert(inputList[i]);        //listFanIn
		
		itrm1->second.pointFanIn.push_back(&itrm2->second);  //pointFanIn
		itrm1->second.numberFanIn = 1;		 //numberFanIn
		itrm1->second.circuitIndex = 1;    //circuitIndex
		
		for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++) //pointFanOut
		{
			itrm3 = tempOrNodeList_m.find((*itrv)->lineNumber);
			itrm1->second.pointFanOut.push_back(&itrm3->second);
		}
		itrm1->second.numberFanOut = itrm1->second.pointFanOut.size();  //numberFanOut
		
		
		//2. Update the list fanin of original fanout nodes of inputList[i]
		for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++)
		{
			itrm3 = tempOrNodeList_m.find((*itrv)->lineNumber);
			itrm3->second.listFanIn.erase(inputList[i]);
			itrm3->second.listFanIn.insert(addedNode->lineNumber);
			vector_erase(itrm3->second, inputList[i], 1);
			itrm3->second.pointFanIn.push_back(&itrm1->second);
			itrm3->second.numberFanIn= itrm3->second.pointFanIn.size();
		}
	
		//3. Change inputList[i]'s correspondent PI node in tempBrNodeList_m.
		//print_circuit(tempBrNodeList_m, redFILE1);
		//redFILE1 << "brInputList[numPO + i] = "<<brInputList[numPO + i]<<endl;
		itrm = tempBrNodeList_m.find(brInputList[numPO + i]);		
		itrm->second.nodeType = FB;
		itrm->second.gateType = 1;
		itrm->second.listFanIn.insert(inputList[i]);
		itrm->second.numberFanIn = 1;
		itrm->second.circuitIndex = 3;
					
		
		//Update inputList[i]'s fanout.
		itrm2->second.pointFanOut.clear();
		itrm2->second.pointFanOut.push_back(&itrm1->second);
		itrm2->second.numberFanOut = itrm2->second.pointFanOut.size();

		maxLineNumber = maxLineNumber + 1;
		delete addedNode;	

	}		
			
	itrm = tempOrNodeList_m.end();
	itrm--;
	maxLineNumber=itrm->second.lineNumber;
	
	
	//Put nodes in "orNodeList" into "masterNodeList".
	copy_point_vector1(tempOrNodeList_m, masterNodeList_m);
	
	//Put nodes in "brNodeList" into "masterNodeList", except for the input nodes first.
	for(itrm = tempBrNodeList_m.begin(); itrm != tempBrNodeList_m.end(); itrm++)
	{
	    int p;
	    if(opl_hash.Search_Hash(itrm->second.lineNumber, p) == 1)
	        continue;
		masterNodeList_m.insert(pair<int, CircuitNode>(itrm->second.lineNumber, itrm->second));
	}

	for(itrm = masterNodeList_m.begin(); itrm != masterNodeList_m.end(); itrm++)
	{
		if((itrm->second.circuitIndex == 2) || (itrm->second.circuitIndex == 3))
		{
            //pointFanIn
			itrm->second.pointFanIn.clear();
			itrm1 = tempBrNodeList_m.find(itrm->second.lineNumber);
			for(itrs = itrm1->second.listFanIn.begin(); itrs != itrm1->second.listFanIn.end(); itrs++)
			{
				itrm2 = masterNodeList_m.find(*itrs);
				itrm->second.pointFanIn.push_back(&itrm2->second);				
			}
			itrm->second.numberFanIn = itrm->second.pointFanIn.size();

            //pointFanOut
			itrm->second.pointFanOut.clear();
			itrm3 = tempBrNodeList_m.find(itrm->second.lineNumber);
			for(itrv = itrm3->second.pointFanOut.begin(); itrv != itrm3->second.pointFanOut.end(); itrv++)
			{
				itrm4 = masterNodeList_m.find((*itrv)->lineNumber);
				itrm->second.pointFanOut.push_back(&itrm4->second);
			}
			itrm->second.numberFanOut = itrm->second.pointFanOut.size();
		}
	}


	//step 1
	for(int i=0; i<inputList.size(); i++)
	{
		itrm = masterNodeList_m.find(inputList[i]);
		itrm1 = masterNodeList_m.find(brInputList[numPO + i]);
		itrm->second.pointFanOut.push_back(&itrm1->second);
		itrm->second.numberFanOut = itrm->second.pointFanOut.size();
		itrm->second.circuitIndex = 3;
	}	
	
	
	//Handle the intermediate output nodes.
	//Add the fanout information and change the node type to GT.
	for(int i = 0; i < outputList.size(); i++)
	{
		itrm = masterNodeList_m.find(outputList[i]);
		if(itrm->second.gateType == 1)
			itrm->second.nodeType = FB;                   //GT
		else
			itrm->second.nodeType = GT;  
				
		itrm1 = tempBrNodeList_m.find(itrm->first);
		for(itrv = itrm1->second.pointFanOut.begin(); itrv != itrm1->second.pointFanOut.end(); itrv++)
		{
			itrm2 = masterNodeList_m.find((*itrv)->lineNumber);
			itrm->second.pointFanOut.push_back(&itrm2->second);
		}
		itrm->second.numberFanOut = itrm->second.pointFanOut.size();			
		itrm->second.circuitIndex = 4;
	}
	
	return maxLineNumber;

}


