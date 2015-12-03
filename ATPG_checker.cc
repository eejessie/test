/*
 * =====================================================================================
 *
 *       Filename:  atpg.cc
 *
 *    Description:  
 *
 *        License:  
 *
 *        Version:  1.0
 *        Created:  12/02/2011
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
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


#include "lib/forward_implication.h"  
#include "lib/radix_convert.h"         
#include "lib/string_convert.h"    
#include "class/CircuitNode.h"        
#include "class/CircuitLine.h"         
#include "class/FaultList.h"          
#include "class/TestList.h"   
#include "function/atpg.h"
#include "function/print_circuit.h"        

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern int PO_line;
extern vector<int> inputList; 
extern ofstream redFILE0, redFILE1;


bool CheckVectorForATPG (map<int, CircuitNode> &masterNodeList, int totalInputs, int inLineNumber, bool inStuckAtValue, string inVector) 
{
   // redFILE1  << "current vector is "<<inVector<<endl;
    map<int, CircuitNode>::iterator itrm, itrm1;
    vector<CircuitNode*>::iterator itrv;

    int *inputVector;
    inputVector = new int [totalInputs];
    set <int> *gateVector;
    gateVector = new set <int> [3];
    set <int>::iterator itrSet;


    // Convert the input vector string to integer
    // values to be applied to the circuit for simulation.
    // (- 48) is done to convert ascii to integer.
    for (int i = 0; i < totalInputs; i++) {
        inputVector[i] = inVector[i] - 48;
    }

    // Clear all the sets. This is important since we don't want
    // the previous values affecting current simulation.
    for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
    {
        itrm->second.lineValue.clear();
        if(itrm->second.gateType == 9)                  //G_stat0
			itrm->second.lineValue.insert(0);
		else if(itrm->second.gateType == 10)            //G_stat1
			itrm->second.lineValue.insert(1);
    }
    
    // Assign the vector to inputs and keep all other values empty.
    int j = 0;
    for(int i = 0; i < inputList.size(); i++)
    {
        itrm = masterNodeList.find(inputList[i]);
        //redFILE1 << "PI "<<inputList[i]<<" is set to "<<inputVector[j]<<endl;;
        itrm->second.lineValue.insert(inputVector[j++]);
       
    }
    
    
    bool isFaultExcited = false;        // Check if the fault can be excited.
    bool isTestGenerated = false;       // Check if the test is generated.

    // For all the nodes in the circuit.
    //      -- For all the primary inputs.
    //          -- If the current line == the line where fault exists.
    //              -- If the fault can be excited.
    //                  -- Set the value to D or Dbar.
    for(int i = 0; i < inputList.size(); i++)
    {
       if (inLineNumber == inputList[i]) 
       {
            itrm = masterNodeList.find(inputList[i]);
            itrSet = itrm->second.lineValue.begin(); 
            if (*itrSet == 0 && inStuckAtValue == true) 
            {
                    isFaultExcited = true;
                    itrm->second.lineValue.clear();
                    itrm->second.lineValue.insert(3);
                    break;
            }
            if (*itrSet == 1 && inStuckAtValue == false) 
            {
                    isFaultExcited = true;
                    itrm->second.lineValue.clear();
                    itrm->second.lineValue.insert(2);
                    break;
            }
       }
    }

    set <int> tempSet;  // To store the result of implication.
    
    int outputLevel = SetLineLevel(masterNodeList, inputList); 

	for (int i = 1; i <= outputLevel; i++)
	{
	    for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
		{
			if (itrm->second.lineLevel == i)
			{
			    //redFILE1 << "check node "<<itrm->second.lineNumber<<endl;
				int inputNumber = 0;
	            for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
				{
	                 int input = (*itrv)->lineNumber;
	                 itrm1 = masterNodeList.find(input);
	                 gateVector[inputNumber++] = itrm1->second.lineValue;
	            }
	            tempSet.clear();
				switch (itrm->second.pointFanIn.size()) 
				{
	                case 1:
	                    tempSet = forwardImplication (itrm->second.gateType, gateVector[0]);
	                    break;
	                case 2:
	                    tempSet = forwardImplication (itrm->second.gateType, gateVector[0], gateVector[1]);
	                    break;
	                default:
	                    cerr << "This ATPG generator only works if the number of inputs to a gate is less than 3." << endl;
	                    cerr << "Currently there are " << itrm->second.numberFanIn << " inputs to the gate." << endl;
	                    exit (0);
	                    break;
            	}
            	itrSet = tempSet.begin();
            	//redFILE1 << "value = "<<*itrSet<<endl;
            	//redFILE1 << "inLineNumber = "<<inLineNumber<<endl;
		        if (inLineNumber == itrm->second.lineNumber) 
				{
				    //redFILE1 << "YYYYYYYYYYYYYYYYYYYYYYYYYYYY"<<endl;
		             if (*itrSet == 0 && inStuckAtValue == true) 
					 {
		                 isFaultExcited = true;
		                 tempSet.clear();
		                 tempSet.insert(3);
		             }
		             if (*itrSet == 1 && inStuckAtValue == false) 
					 {
		                 isFaultExcited = true;
		                 tempSet.clear();
		                 tempSet.insert(2);
		             }
		            // redFILE1 << "isFaultExcited = "<<isFaultExcited<<endl;
		            // If fault cannot be excited, exit.
		            if (isFaultExcited == false)
		                return false;
		        }		        
		        itrm->second.lineValue = tempSet;
			}//if()
	    }//for()
	}//for()

    // Once the simulation is done, check all the outputs.
    //      -- If any output has either a D or a Dbar then the fault is detected.
    //      -- The vector can test the fault.
    itrm = masterNodeList.find(PO_line);
    itrSet = itrm->second.lineValue.begin(); 
   // redFILE1 << "PO_line: "<<*itrSet<<endl;    
    if (*itrSet == 2 || *itrSet == 3)
    {
       // redFILE1<<"PO's value "<<PO_line<<" = "<<*itrSet<<endl;
        isTestGenerated = true;
    }
   
    if (isTestGenerated)
        return true;
    else
        return false;
}


bool TestAllVectorsATPG (map<int, CircuitNode> &masterNodeList, int totalInputs, int inLineNumber, bool inStuckAtValue, vector <string> &inVectorList) 
{

    string outVector;

    //for(int j=inVectorList.size()-1; j>=inVectorList.size()-pow(2, totalInputs); j--)
    for(int j=inVectorList.size()-1; j>=0; j--)
    {
        outVector = inVectorList[j];
        if (CheckVectorForATPG(masterNodeList, totalInputs, inLineNumber, inStuckAtValue, outVector)) 
        {
            redFILE1 << "test vector = "<<outVector<<endl;
            //print_circuit(masterNodeList, redFILE1);
            return true;
        }
    }

    return false;

}







 
