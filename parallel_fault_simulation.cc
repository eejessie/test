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
#include "lib/forward_implication.h"

#include "function/atpg.h"
#include "function/upd_circuit.h"
#include "function/del_gate.h"
#include "function/merge_circuit.h"
#include "function/print_circuit.h" 
#include "function/run_logic_simulation.h"
#include "function/copy_point_vector.h"
#include "function/helper.h"
#include "podem/podem_v2.h"



// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern int PO_line;
extern ofstream redFILE0, redFILE1;
extern ofstream timeRecord;
extern vector<int> inputList;

#define wordSize 32


void parallel_fault_simulation(map<int, CircuitNode> &masterNodeList, vector<FaultList> &sf, string testVector, vector<int> &testRecord)
{
	map<int, CircuitNode>::iterator itrm;
	int totalInputs = inputList.size();
   	vector<int> *inputVector;
   	inputVector = new vector<int> [totalInputs];
   	vector <int> *gateVector;
    gateVector = new vector <int> [2];
    vector <int>::iterator itrv1, itrv2;
	vector<CircuitNode*>::iterator itrv;
	
	redFILE0 << endl<<"Coming into parallel_fault_simulation:"<<endl;
	redFILE0 << "testVector: "<<testVector<<endl;
	redFILE0 << "fault size: "<<sf.size()<<endl;
   	
   	for(int i = 0; i < totalInputs; i++)
		for(int j = 0; j < sf.size()+1; j++)
		{
			//redFILE0 << testVector[i] - 48 << " ";
			inputVector[i].push_back(testVector[i] - 48);  
		}
	//redFILE0 << endl;		    
	
	
	for(int i = 0; i < sf.size(); i++)
	{
		int lineNumber = sf[i].lineNumber;
	   	int stuckAtValue = sf[i].stuckAtValue;
	   	
	   	//redFILE0 << "("<<lineNumber<<","<<stuckAtValue<<")"<<endl;
	   		
	   	int result = search_vector_int(inputList, lineNumber);
	   	if(result != -1)   	   //If this sf is at some input node
	   	{
	   		//redFILE0 << "SF is at input node: "<<lineNumber<<endl;
	   		inputVector[result][i+1] = stuckAtValue;
	   	}	   	
	 }
	 
	redFILE0 << "inputVector: "<<endl;
	for(int i = 0; i < totalInputs; i++)
	{
		for(itrv1 = inputVector[i].begin(); itrv1 != inputVector[i].end(); itrv1++)
			redFILE0 << *itrv1;  
		redFILE0 << endl;    
	} 
	 
	// Assign the vector to inputs and keep all other values empty.
	for (itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++) 
    {
    	itrm->second.multiValue.clear();
     	if (itrm->second.gateType == G_stat0 )
     		for(int i = 0; i < sf.size()+1; i++)
	        	itrm->second.multiValue.push_back(0); 
	    if (itrm->second.gateType == G_stat1 )
	    	for(int i = 0; i < sf.size()+1; i++)
	        	itrm->second.multiValue.push_back(1); 
	}  
	
    for (int j = 0; j < inputList.size(); j++) 
    {
    	itrm = masterNodeList.find(inputList[j]);
    	itrm->second.multiValue = inputVector[j];
    	//for(int i = 0; i < sf.size()+1; i++)
    	//	itrm->second.multiValue.push_back(inputVector[j][i]);
    	//redFILE0 << "Input "<<itrm->second.lineNumber<<": "<<endl;
    	//for(int k = 0; k < itrm->second.multiValue.size(); k++)
    	//	redFILE0 << itrm->second.multiValue[k];
    	//redFILE0 << endl;
    }
    
   
	int outputLevel = SetLineLevel(masterNodeList);
	
	int length = sf.size() + 1;                         //The actual word size.
	vector <int> tempSet;  // To store the result of implication.	
	for (int i = 1; i <= outputLevel; i++)
	{
		for (itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
		{
			if (itrm->second.lineLevel == i)
			{
			    //redFILE1 << "current node:"<<itrm->second.lineNumber<<endl;
				int inputNumber = 0;
	            for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
					gateVector[inputNumber++] = (*itrv)->multiValue;
			
				tempSet.clear();				
				switch (itrm->second.listFanIn.size()) 
				{										
	                case 1:
	                    tempSet = forwardImplication(itrm->second.gateType, gateVector[0], length);
	                    break;
	                case 2:
	                    tempSet = forwardImplication(itrm->second.gateType, gateVector[0], gateVector[1], length);
	                    break;
	                default:
						cerr << "This node is "<<itrm->second.lineNumber<<endl;
	                    cerr << "This ATPG generator only works if the number of inputs to a gate is less than 4." << endl;
	                    cerr << "Currently there are " << itrm->second.numberFanIn << " inputs to the gate." << endl;
	                    exit (0);
	                    break;
            	}     
                
            	for(int j = 0; j < sf.size(); j++)
            		if(sf[j].lineNumber == itrm->second.lineNumber)
            		{
            		   // redFILE1 << sf[j].lineNumber<<", "<<sf[j].stuckAtValue<<endl;
            			tempSet[j+1] = sf[j].stuckAtValue; 	
            		}
			 	itrm->second.multiValue = tempSet;
			 
			}			
		}			
	}
	 
	/*for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
	    if(itrm->second.circuitIndex == 1 || itrm->second.circuitIndex == 4)
	    {
	        redFILE1 <<"node "<<itrm->second.lineNumber<<"'s value:"<<endl;
	        for(int i = 0; i< itrm->second.multiValue.size(); i++)
                redFILE1 << itrm->second.multiValue[i];
            redFILE1 << endl;
        }
	}*/
	 
	for(int i = 0; i < sf.size(); i++)
		testRecord.push_back(0);
    
    itrm = masterNodeList.find(PO_line);
	itrv1 = itrm->second.multiValue.begin();
	int correctValue = *itrv1;
	redFILE0 << "At the end, correctValue = "<<correctValue<<endl;
	
	for(int i = 1; i < sf.size()+1; i++)
	{
	    redFILE0 << itrm->second.multiValue[i];
	    if(itrm->second.multiValue[i] != correctValue)
		    testRecord[i-1] = 1;
		else
		    testRecord[i-1] = 0;
    }
    redFILE0 << endl;
		
	redFILE0 << "after, testRecord0:"<<endl;
	for(int i = 0; i < testRecord.size(); i++)
		redFILE0 << testRecord[i]<< " ";
	redFILE0 << endl;
		
	delete []inputVector;
	delete []gateVector;
	
	return;
}
