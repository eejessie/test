/*
 * =====================================================================================
 *
 *       Filename:  parallel_fault_simulation.cc
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
#include <ctime>
#include <sys/timeb.h>

#include "class/CircuitNode.h"
#include "class/FaultList.h"
#include "class/TestList.h"
#include "class/ConnFault.h"
#include "class/queue.h"
#include "class/HashTable.h"
#include "lib/forward_implication.h"

#include "function/atpg.h"
#include "function/upd_circuit.h"
#include "function/del_gate.h"
#include "function/merge_circuit.h"
#include "function/print_circuit.h" 
#include "function/run_logic_simulation.h"
#include "function/copy_point_vector.h"
#include "function/helper.h"
#include "function/sim_br.h"
#include "podem/podem_v2.h"



// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

//extern int PO_line;
extern int numPI, numPO;
extern int PO_line;
extern ofstream redFILE0, redFILE1;
extern ofstream timeRecord;
extern vector<int> inputList;
extern vector<int> outputList;
extern HashTable opl_hash;


map<int, vector<int> > parallel_test_simulation(map<int, CircuitNode> &orNodeList, FaultList &sf, vector<string> &testVector, int outputLevel, vector<int> *levelSet)
{
    //Iterators
	map<int, CircuitNode>::iterator itrm;
   	vector<int> *inputVector;
   	map<int, vector<int> > outputVector;
   	inputVector = new vector<int> [numPI];
   	vector <int> *gateVector;
    gateVector = new vector <int> [2];
    vector <int>::iterator itrv1, itrv2;
	vector<CircuitNode*>::iterator itrv;
	
	struct timeb startTime, endTime;  

   	for(int i = 0; i < numPI; i++)
		for(int j = 0; j < testVector.size(); j++)
			inputVector[i].push_back(testVector[j][i] - 48);  

    int lineNumber = sf.lineNumber;
	int stuckAtValue = sf.stuckAtValue;
	   	

	// Assign the vector to inputs and keep all other values empty.
	for (itrm = orNodeList.begin(); itrm != orNodeList.end(); itrm++) 
    {
    	itrm->second.multiValue.clear();
     	if (itrm->second.gateType == G_stat0 )
     	{
     		for(int i = 0; i < testVector.size(); i++)
	        	itrm->second.multiValue.push_back(0); 
	        outputVector.insert(pair<int, vector<int> >(itrm->second.lineNumber, itrm->second.multiValue));
	    }
	    if (itrm->second.gateType == G_stat1 )
	    {
	    	for(int i = 0; i < testVector.size(); i++)
	        	itrm->second.multiValue.push_back(1);
	        outputVector.insert(pair<int, vector<int> >(itrm->second.lineNumber, itrm->second.multiValue));
	    } 
	}  
	
    for (int j = 0; j < inputList.size(); j++) 
    {
    	itrm = orNodeList.find(inputList[j]);
    	itrm->second.multiValue = inputVector[j];
    }

	
	int length = testVector.size();         //The actual word size.
	vector <int> tempSet;  // To store the result of implication.	
	int index_output = 0;
	vector<int> outVector;
	for (int i = 1; i <= outputLevel; i++)
	{
	    for(int j = 0; j < levelSet[i-1].size(); j++)
		{
		    itrm = orNodeList.find(levelSet[i-1][j]);
			if (itrm->second.lineLevel == i)
			{
			    if(sf.lineNumber == itrm->second.lineNumber)
            	{
                    itrm->second.multiValue.clear();
                    for(int k = 0; k < testVector.size(); k++)
            		    itrm->second.multiValue.push_back(sf.stuckAtValue);  
			 	    continue;	
            	}
				int inputNumber = 0;
	            for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
					gateVector[inputNumber++] = (*itrv)->multiValue;
			
				tempSet.clear();				
				switch (itrm->second.pointFanIn.size()) 
				{										
	                case 1:
	                    tempSet = forwardImplication(itrm->second.gateType, gateVector[0]);
	                    break;
	                case 2:
	                    tempSet = forwardImplication(itrm->second.gateType, gateVector[0], gateVector[1]);
	                    break;
	                default:
						cerr << "This node is "<<itrm->second.lineNumber<<endl;
	                    cerr << "This ATPG generator only works if the number of inputs to a gate is less than 4." << endl;
	                    cerr << "Currently there are " << itrm->second.numberFanIn << " inputs to the gate." << endl;
	                    exit (0);
	                    break;
            	}     
            	if(itrm->second.lineNumber == sf.lineNumber)
	            {	            	
	            	if(sf.stuckAtValue == 1)
	            	{
	            	    for(int k = 0; k < tempSet.size(); k++)
	                	{
	                		if((tempSet[k] == 0))          //Fault has been excited.
	                   			tempSet[k] = 3;
	                    }
	               	}
					else if(sf.stuckAtValue == 0)
					{
						for(int k = 0; k < tempSet.size(); k++)
	                	{
	                		if((tempSet[k] == 1))          //Fault has been excited.
	                   			tempSet[k] = 2;
	                    }
	               	}
			    }			 	            	
            	
			 	itrm->second.multiValue = tempSet;
			 				 				 	
			 	int p;
			 	if(opl_hash.Search_Hash(itrm->second.lineNumber, p) == 1)
			 	    outputVector.insert(pair<int, vector<int> >(itrm->second.lineNumber, itrm->second.multiValue));			 	    
			 	
			}			
		}			
	}
	
/*	vector<int> outVector;
	map<int, vector<int> >::iterator itrmv;
	for(int i = 0; i < testVector.size(); i++)
	{
	    vector<int> inputVector_br;
	    for(int j = 0; j < numPI; j++)
	        inputVector_br.push_back(inputVector[j][i]);
	    for(int j = 0; j < outputList.size(); j++)
	    {
	        itrmv = outputVector.find(outputList[j]);
	        vector<int> temp = itrmv->second;
	        inputVector_br.push_back(temp[i]);
	    }
	    int outValue = sim_br(truthTable, inputVector_br);
	    outVector.push_back(outValue);
	}
	*/
	
	 
	delete []inputVector;
	delete []gateVector;

	return outputVector;
//	return outVector;
	
}
