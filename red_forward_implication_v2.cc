/*
 * =====================================================================================
 *
 *       Filename:  red_forward_implication_v2.cc
 *
 *    Description:  This function performs forward implication on a 1-3 input gate.
 *                  The inputs and output are integer sets. It reads the values from
 *                  logic tables.
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

#include "class/CircuitNode.h"         // This class stores information about each node in the circuit.
#include "class/HashTable.h" 
#include "lib/forward_implication.h"
#include "function/print_circuit.h"
#include "function/helper.h"

using namespace std;
//using namespace nameSpace_ATPG;


extern ofstream redFILE0, redFILE1;
extern vector<int> outputList;


// #####   HEADER FILE INCLUDES   ################################################### 

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  red_forward_imlpication
 *  Description:  The function takes in an STL set as input and returns a set as output.
 *                The version defined below is a single input version.
 *                Set is used since multiple inputs may give same result.
 * =====================================================================================
 */

void red_forward_implication_helper(map <int, CircuitNode> &masterNodeList, int inLineNumber, vector<int> &stop_fanout) 
{

	map<int, CircuitNode>::iterator itrm, itrm1, itrm2;
	vector<CircuitNode*>::iterator itrv, itrv1;
    set <int> *gateVector;
    gateVector = new set <int> [3];
    set <int>::iterator itrSet, itrs, itrs1, itrs2;
	int i, j;	

    set <int> tempSet;  // To store the result of implication.

    itrm = masterNodeList.find(inLineNumber);
	//redFILE1 << endl <<"Current line = "<<inLineNumber<<endl;

	if(itrm->second.pointFanOut.size() == 0)
	{
	    delete []gateVector;
		return;
    }

    for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++)
	{
		itrm1 = masterNodeList.find((*itrv)->lineNumber);
		itrs = itrm1->second.lineValue.begin();
		if(*itrs != X)
		    continue;
		int num = 0;
		for(itrv1 = itrm1->second.pointFanIn.begin(); itrv1 != itrm1->second.pointFanIn.end(); itrv1++)
		{
			itrm2 = masterNodeList.find((*itrv1)->lineNumber);
			gateVector[num++] = itrm2->second.lineValue;
		}
		
        //Call proper implication function depending on the number of the inputs to the gate.
        // Overloaded function, please refer to logic_implication.cc for details.
        switch (itrm1->second.pointFanIn.size()) {
             case 1:
                 tempSet = forwardImplication (itrm1->second.gateType, gateVector[0]);
                 break;
             case 2:
                 tempSet = forwardImplication (itrm1->second.gateType, gateVector[0], gateVector[1]);
                 break;
             case 3:
                 tempSet = forwardImplication (itrm1->second.gateType, gateVector[0], gateVector[1], gateVector[2]);
                 break;
             default:
                 cerr << "This ATPG generator only works if the number of inputs to a gate is less than 4." << endl;
                 cerr << "Currently there are " << itrm1->second.pointFanIn.size() << " inputs to the gate " <<  itrm1->second.lineNumber<<endl;
                 exit (0);
                 break;
        }
		itrm1->second.lineValue.clear();
        itrm1->second.lineValue = tempSet;
		//itrs1 = itrm1->second.lineValue.begin();
		
		

		red_forward_implication_helper(masterNodeList, (*itrv)->lineNumber, stop_fanout);
		
	}
	
	delete []gateVector;

}
            
void red_forward_implication_v2(map <int, CircuitNode> &masterNodeList, HashTable &implicationNodeList, int inLineNumber, bool inStuckAtValue) 
{
    /*redFILE0 <<endl;
	redFILE0 << "*********************************"<<endl;   
	redFILE0 <<"Enter into red_forward_implication!"<<endl;
	redFILE0 << "*********************************"<<endl;  */ 

	map<int, CircuitNode>::iterator itrm;
    set <int>::iterator itrSet;
   
    // Set all the line values to X. This is important since we don't want
    // the previous values affecting current simulation.
    for (itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++) 
	{
		itrm->second.lineValue.clear();
		if(itrm->second.gateType == 9)
        	itrm->second.lineValue.insert(0);
        else if(itrm->second.gateType == 10)
        	itrm->second.lineValue.insert(1);
        else
        	itrm->second.lineValue.insert(X);
        
    }
    
   /* redFILE1 << "values on lines before implication:"<<endl;
	for (itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
		
		itrSet = itrm->second.lineValue.begin();
		if(*itrSet != X)
		{
			redFILE1 << "Node: "<<itrm->second.lineNumber<<", value:  ";
			redFILE1 << *itrSet<<endl;
		}
	}
	redFILE1 << endl;*/


	itrm = masterNodeList.find(inLineNumber);
	//redFILE1 << "In implication_v2, current node is "<<inLineNumber<<endl;
	itrm->second.lineValue.clear();
	itrm->second.lineValue.insert(inStuckAtValue);
	
	//Perform forward implication recursively.
	vector<int> stop_fanout;
	for(int i = 0; i < outputList.size(); i++)
		stop_fanout.push_back(outputList[i]);
	//stop_fanout.Destroy_HashTable();
	
    red_forward_implication_helper(masterNodeList, inLineNumber, stop_fanout);


	/*redFILE1 << "values on lines after implication:"<<endl;
	for (itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
		
		itrSet = itrm->second.lineValue.begin();
		if(*itrSet != X)
		{
			redFILE1 << "Node: "<<itrm->second.lineNumber<<", value:  ";
			redFILE1 << *itrSet<<endl;
		}
	}
	redFILE1 << endl;*/


	//Put lines with value that is not X into "implicationNodeList".
	for (itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
		itrSet = itrm->second.lineValue.begin();
		if ((*itrSet)!=4)
		{
			int res = implicationNodeList.Insert_Hash(itrm->second.lineNumber);
			while(res == 0)
				res = implicationNodeList.Insert_Hash(itrm->second.lineNumber);
		}
	}
		

}



