/*
 * =====================================================================================
 *
 *       Filename:  parallel_fault_simulation_bdd.cc
 *
 *    Description:  
 *
 *        License:  
 *
 *        Version:  1.0
 *        Created:  22/3/2015
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
#include "cudd/cudd_comp.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

//extern int PO_line;
extern int numPI, numPO;
extern ofstream redFILE0, redFILE1;
extern ofstream timeRecord;
extern vector<int> cut_nodes;
extern vector<int> inputList;
extern vector<int> outputList;
extern HashTable opl_hash;

vector<int> parallel_test_simulation_cut(map<int, CircuitNode> &orNodeList, FaultList &sf, vector<string> &testVector, map<int, int> &truthTable, int outputLevel, vector<int> *levelSet)
{
    //Iterators
	map<int, CircuitNode>::iterator itrm;
	//int numPI = inputList.size();
   	
    vector <int>::iterator itrv1, itrv2;
	vector<CircuitNode*>::iterator itrv;
	
	struct timeb startTime, endTime;  
	
	int total_vector = testVector.size();
	int round, num_each_round, num_last_round;
	if(total_vector <= 32)
	{
	    round = 1;
	    num_each_round = total_vector;
	    num_last_round = 0;
	}
	else
	{
	    round = total_vector/32;
	    num_each_round = 32;
	    num_last_round = total_vector - round*32;
	}
	
	int lineNumber = sf.lineNumber;
	int stuckAtValue = sf.stuckAtValue;
	map<int, vector<int> > outputVector;
	map<int, vector<int> >::iterator itrmv;
	for(int r = 0; r <= round; r++)
   	{
   	    vector<int> *inputVector;       	
       	inputVector = new vector<int> [numPI];
       	vector <int> *gateVector;
        gateVector = new vector <int> [2];
       
        int num_this_round;
        if(r < round)
            num_this_round = num_each_round;
        else if(r == round)
        {
            if(num_last_round == 0)
                break;
            else
                num_this_round = num_last_round;        
        }
               
       	for(int i = 0; i < numPI; i++)
		    for(int j = 0; j < num_this_round; j++)
			    inputVector[i].push_back(testVector[r*32+j][i] - 48);  

	    // Assign the vector to inputs and keep all other values empty.
	    for (itrm = orNodeList.begin(); itrm != orNodeList.end(); itrm++) 
        {
        	itrm->second.multiValue.clear();
         	if (itrm->second.gateType == G_stat0 )
         	{
         		for(int i = 0; i < num_this_round; i++)
	            	itrm->second.multiValue.push_back(0); 
	            if(itrm->second.nodeType == 3)
	            {
	                itrmv = outputVector.find(itrm->second.lineNumber);
	                if(itrmv == outputVector.end())
	                    outputVector.insert(pair<int, vector<int> >(itrm->second.lineNumber, itrm->second.multiValue));
	                else
	                {
	                    vector<int> this_time_vector = itrm->second.multiValue;
	                    vector<int>::iterator it = itrmv->second.end();
	                    itrmv->second.insert(it, this_time_vector.begin(), this_time_vector.end());
	                }
	            }
	        }
	        if (itrm->second.gateType == G_stat1 )
	        {
	        	for(int i = 0; i < num_this_round; i++)
	            	itrm->second.multiValue.push_back(1);
	            if(itrm->second.nodeType == 3)
	            {
	                itrmv = outputVector.find(itrm->second.lineNumber);
	                if(itrmv == outputVector.end())
	                    outputVector.insert(pair<int, vector<int> >(itrm->second.lineNumber, itrm->second.multiValue));
	                else
	                {
	                    vector<int> this_time_vector = itrm->second.multiValue;
	                    vector<int>::iterator it = itrmv->second.end();
	                    itrmv->second.insert(it, this_time_vector.begin(), this_time_vector.end());
	                }
	            }
	        } 
	    }  
	
        for (int j = 0; j < inputList.size(); j++) 
        {
        	itrm = orNodeList.find(inputList[j]);
        	if (itrm->second.gateType == G_stat0 )
        	{
        	    for(int i = 0; i < num_this_round; i++)
	            	itrm->second.multiValue.push_back(0);
	        }
	        else if (itrm->second.gateType == G_stat1 )
	        {
	        	for(int i = 0; i < num_this_round; i++)
	            	itrm->second.multiValue.push_back(1); 
	        }
        	else itrm->second.multiValue = inputVector[j];
        }
	
	    int length = num_this_round;         //The actual word size.
	    vector <int> tempSet;  // To store the result of implication.	
	    int index_output = 0;
	
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
                        for(int k = 0; k < num_this_round; k++)
                		    itrm->second.multiValue.push_back(sf.stuckAtValue);  
			     	    continue;	
                	}
		    //	    redFILE1 << "current node = "<<itrm->second.lineNumber<<endl;
				    int inputNumber = 0;
	                for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
				    {
					    gateVector[inputNumber++] = (*itrv)->multiValue;
		            }
			
				    tempSet.clear();				
				    switch (itrm->second.pointFanIn.size()) 
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
			     	itrm->second.multiValue = tempSet;
     	
			 /*    	int p;
			     	if(opl_hash.Search_Hash(itrm->second.lineNumber, p) == 1)
			     	{
			     	    itrmv = outputVector.find(itrm->second.lineNumber);
			     	    if(itrmv == outputVector.end())
			     	        outputVector.insert(pair<int, vector<int> >(itrm->second.lineNumber, itrm->second.multiValue));
			     	    else
			     	    {
	                        vector<int> this_time_vector = itrm->second.multiValue;
	                        vector<int>::iterator it = itrmv->second.end();
	                        itrmv->second.insert(it, this_time_vector.begin(), this_time_vector.end());
	                    }			     	    
			     	}*/			     
			    }//if (itrm->second.lineLevel == i)			
		    }//for(int j = 0; j < levelSet[i-1].size(); j++)			
	    }//for (int i = 1; i <= outputLevel; i++)
	    
	    delete []inputVector;
	    delete []gateVector;
		    
	}//for(int r = 0; r < round; r++)
	
	vector<int> outVector;
	int size = numPI + numPO;
	int *assign = new int[size];
	
	//print out the outputVector
/*	for(itrmv = outputVector.begin(); itrmv != outputVector.end(); itrmv++)
	{
	    redFILE1 << "output = "<<itrmv->first<<", value = ";
	    vector<int> outvector = itrmv->second;
	    for(int i = 0; i < outvector.size(); i++)
	        redFILE1 << outvector[i];
	    redFILE1 << endl;
	}*/
	for(int i = 0; i < testVector.size(); i++)
	{
	    vector<int> inputVector_cut;
	    for(int j = 0; j < cut_nodes.size(); j++)
	    {
	        itrm = orNodeList.find(cut_nodes[j]);
	        vector<int> temp = itrm->second.multiValue;
	        inputVector_cut.push_back(temp[i]);
	    }	    
	    
	    int outValue = sim_br(truthTable, inputVector_cut);
	    outVector.push_back(outValue);
	}
	delete []assign;	
	 
	
	return outVector;
	
}

