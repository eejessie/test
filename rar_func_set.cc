/*
 * =====================================================================================
 *
 *       Filename:  adjust_ln.cc
 *
 *    Description:  This function adjusts the line number of nodes in BR circuit so that the interface corresponds to each other and there are no    
 *                  duplicate line numbers in both circuits. 
 *
 *        License:  
 *
 *        Version:  1.0
 *        Created:  11/08/2014
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

#include "class/CircuitNode.h"         // This class stores information about each node in the circuit.
#include "class/ConnFault.h"
#include "function/copy_point_vector.h"
#include "function/print_circuit.h"
#include "function/helper.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0;


void adjust_ln(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, vector<int> &inputList, vector<int> &outputList)
{
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3;
	set<int>::iterator itrs;
	vector<int>::iterator itrv0;
	vector<CircuitNode*>::iterator itrv;

	vector<int> tempPI = inputList;
	//for(int i = 0; i < inputList.size(); i++)
	//	tempPI.push_back(inputList[i]);
		
	redFILE0 << "tempPI:"<<endl;
	for(itrv0 = tempPI.begin(); itrv0 != tempPI.end(); itrv0++)
		redFILE0 << *itrv0 << " ";
	redFILE0<<endl;
		
	//Step1 Find out the right order of the the POs in original circuit.
	vector<int> tempPO = outputList;
	/*for(int i = 0; i < outputList.size(); i++)
	{
		itrm = orNodeList.find(outputList[i]);
		tempPO.push_back(itrm->second.lineNumber);			
	}*/
	
	insertion_sort_asc(tempPO, outputList.size());
	int maxLineNumber = tempPO[tempPO.size()-1];
	
	redFILE0 << "tempPO:"<<endl;
	for(itrv0 = tempPO.begin(); itrv0 != tempPO.end(); itrv0++)
		redFILE0 << *itrv0 << " ";
	redFILE0<<endl;
	
	vector<int> interPI;
	interPI.insert(interPI.begin(), tempPO.begin(), tempPO.end());
	interPI.insert(interPI.begin(), tempPI.begin(), tempPI.end());
	
	redFILE0 << "After merging, interPI:"<<endl;
	for(itrv0 = interPI.begin(); itrv0 != interPI.end(); itrv0++)
		redFILE0 << *itrv0 << " ";
	redFILE0<<endl;
	
	//Step2 Adjust the line number of PIs in BR circuit and other duplicate line numbers.
	
	//Plus all line numbers in brNodeList with maxLineNumber + 1, where maxLineNumber is the maximum line number in orNodeList.
	for(itrm = brNodeList.begin(); itrm != brNodeList.end(); itrm++)
		itrm->second.lineNumber = itrm->second.lineNumber + maxLineNumber + 1;
	
	//Make the line number of orginal circuit's POs in BR circuit the same as those in original circuit.
	int numPI = inputList.size();
	int numPO = outputList.size();
	for(itrm = brNodeList.begin(); itrm != brNodeList.end(); itrm++)
		if(itrm->second.nodeType == 1)
		{			
			int node = itrm->second.lineNumber;
			if(((node-maxLineNumber -1) >= numPI) && ((node-maxLineNumber -1) < numPI + numPO))
			{
				//redFILE0 << "Current node is "<< itrm->second.lineNumber<<endl;
				int input_ln = itrm->second.lineNumber - maxLineNumber - 1;	
				itrm->second.lineNumber = interPI[input_ln];
			}
		}
	
	
	//Update the listFanIn of lines in BR circuit.
	for(itrm = brNodeList.begin(); itrm != brNodeList.end(); itrm++)
	{
		itrm->second.listFanIn.clear();
		for(itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++)
			itrm->second.listFanIn.insert((*itrv)->lineNumber);
		
	}
	
	
	
	//Update the key value of lines in BR circuit.
	map<int, CircuitNode> newBrNodeList;
	for(itrm = brNodeList.begin(); itrm != brNodeList.end(); itrm++)
	{
		int number = itrm->second.lineNumber;
		newBrNodeList.insert(pair<int, CircuitNode>(number, itrm->second));
	}
	
	for(itrm = newBrNodeList.begin(); itrm != newBrNodeList.end(); itrm++)
	{
		//redFILE0 << "This node is "<<itrm->second.lineNumber<<endl;
	
		itrm->second.pointFanIn.clear();
		for(itrs = itrm->second.listFanIn.begin(); itrs != itrm->second.listFanIn.end(); itrs++)
		{
			itrm1 = newBrNodeList.find(*itrs);
			itrm->second.pointFanIn.push_back(&itrm1->second);
		}

		itrm->second.pointFanOut.clear();
		if(itrm->second.nodeType == 1)   //corresponding node in brNodeList.
		{	
			int index = search_vector_int(tempPO, itrm->second.lineNumber);
			if(index != -1)
			{
				int node = index + numPI;
				itrm2 = brNodeList.find(node);
			}
			else
				itrm2 = brNodeList.find(itrm->second.lineNumber - maxLineNumber - 1); 
		}
		else
			itrm2 = brNodeList.find(itrm->second.lineNumber - maxLineNumber - 1);   //corresponding node in brNodeList.
			
		//redFILE0 << "This node is "<<itrm2->second.lineNumber<<endl;
		for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++)
		{
			//redFILE0 << "Input node is "<<(*itrv)->lineNumber<<endl<<endl;
			itrm3 = newBrNodeList.find((*itrv)->lineNumber);
			itrm->second.pointFanOut.push_back(&itrm3->second);
		}
	}
	
	copy_point_vector1(newBrNodeList, brNodeList);
			
}






