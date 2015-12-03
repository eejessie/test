
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


#include "class/CircuitNode.h"       
#include "function/atpg.h"
#include "lib/radix_convert.h"
#include "function/print_circuit.h"
#include "function/sim_br.h"
#include "cudd/cudd_comp.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;
//using namespace nameSpace_ATPG;

extern int PO_line;
extern int numPI, numPO;
extern vector<int> inputList, outputList;
extern ofstream redFILE1, logFile;
extern int EM, BN;


void run_logic_simulation(map<int, CircuitNode> &orNodeList, int totalInputs, vector<string> & InputVector, DdManager *dd, DdNode *dnode)
//void run_logic_simulation(map<int, CircuitNode> &orNodeList, int totalInputs, vector<string> & InputVector)
{
	int	flag = 0;
	map<int, CircuitNode>::iterator itrm, itrm1;
	set <int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv;
	
	ofstream fout;
	fout.open("final_pla.log", ios::out);
	
	int size = numPI+numPO;
	int *assign = new int[size];
		
	int outputLevel = SetLineLevel(orNodeList, inputList);
/*	if(outputLevel == -1)
	{
	    vector<int> inputVector_br;	
	    fout << "all output nodes have fixed values: "<<endl;
	    for(int i = 0; i < outputList.size(); i++)
	    {
	        itrm = orNodeList.find(outputList[i]);
	        itrs = itrm->second.lineValue.begin();
	        inputVector_br.push_back(*itrs);	
	        fout << *itrs;        
	    }
	    fout << endl;
	    fout.close();
	    
	    itrm = orNodeList.find(PO_line);
	    itrs = itrm->second.lineValue.begin();
	    int res = *itrs;

	    if(res == 0)
		{
		    //redFILE1 << "Vector is " << InputVector[j] << endl;
			redFILE1 << "inputVector_br:"<<endl;
	        for(int i = 0; i< inputVector_br.size(); i++)
	            redFILE1 << inputVector_br[i];
	        redFILE1 << endl;
		    redFILE1 << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
		    logFile << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
			cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
		}  
	}
	
	else */
	{
	    vector<int> *levelSet;
	    levelSet = new vector<int>[outputLevel];
	    int level;
        for(itrm = orNodeList.begin(); itrm != orNodeList.end(); itrm++)
        {
            level = itrm->second.lineLevel;
            if(level > 0)
                levelSet[level-1].push_back(itrm->second.lineNumber);
        }
        
        map<int, int>::iterator itrmi;
        for(int j=0; j < InputVector.size(); j++)
	    {		
	        map<int, int> outputValue;
		    LogicSimulation(orNodeList, totalInputs, InputVector[j], outputLevel, levelSet, outputValue);
		 //   int res = -1;
		 //   LogicSimulation(orNodeList, totalInputs, InputVector[j], outputLevel, levelSet, res);
            fout << InputVector[j];
            fout << "  ";
            for(itrmi = outputValue.begin(); itrmi != outputValue.end(); itrmi++)
                fout << itrmi->second;       
            fout << endl;
			
	        vector<int> inputVector_br;	
	        for(int i = 0; i < inputList.size(); i++)
	        {	            
	            itrm = orNodeList.find(inputList[i]);
	            inputVector_br.push_back(InputVector[j][i] - 48);
	        }        

	        for(int i = 0; i < outputList.size(); i++)
	        {
	            itrm = orNodeList.find(outputList[i]);
	            if(itrm->second.gateType == 9)
	                inputVector_br.push_back(0);
	            else if(itrm->second.gateType == 10)
	               inputVector_br.push_back(1);
	            else
	            {
	                itrs = itrm->second.lineValue.begin();
	                inputVector_br.push_back(*itrs);	
	            }        
	        }	

	         for(int k = 0; k < inputVector_br.size(); k++)
	             assign[k] = inputVector_br[k];  
	         
	 /*       redFILE1 << "inputVector_br: ";
	        for(int i = 0; i< inputVector_br.size(); i++)
	        {
	            redFILE1 << inputVector_br[i];	
	            if(i == BN-1 || i == 2*BN-1)
	                redFILE1 << "  ";   
	        }         
	        redFILE1 << endl;
	        int int_f = 0, int_s = 0, int_sum = 0;
	        for(int i = 0; i< BN; i++)
	            int_f += inputVector_br[i]*pow(2, i);
	        for(int i = BN; i< 2*BN; i++)
	            int_s += inputVector_br[i]*pow(2, i-BN);
	        for(int i = 2*BN; i< inputVector_br.size(); i++)
	            int_sum += inputVector_br[i]*pow(2, i-2*BN);
	        redFILE1 << int_f<<" + "<<int_s<<" = "<<int_sum<<endl;*/
	                     
	        int res = cudd_comp(dd, dnode, size, assign);
	      //  redFILE1 << "res = "<<res<<endl;
	       // int res = sim_br(truthTable, inputVector_br);
	       
	        if(res == 0)
		    {
		        redFILE1 << "Vector is " << InputVector[j] << endl;
			    redFILE1 << "inputVector_br:"<<endl;
	            for(int i = 0; i< inputVector_br.size(); i++)
	                redFILE1 << inputVector_br[i];
	            redFILE1 << endl;
		        redFILE1 << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
			    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
			
		    }  
	    }	
	    
	    delete []levelSet;
	}
	delete []assign;
	fout.close();
}

