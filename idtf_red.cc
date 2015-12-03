#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <math.h>
#include <cassert>
#include <ctime>
#include <sys/timeb.h>

#include "class/CircuitNode.h"         
#include "class/CircuitLine.h"        
#include "class/FaultList.h"           
#include "class/TestList.h"  
#include "class/HashTable.h"          
#include "lib/file_operations.h"
#include "function/atpg.h"
#include "function/print_circuit.h"
#include "function/helper.h"
#include "podem/podem_v2.h"
#include "function/run_fault_simulation.h"
#include "function/ATPG_checker.h"



#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern int totalInputs;
extern vector<string> masterInputVector;
extern ofstream redFILE1;

void idtf_red(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &masterNodeList_m, map<int, int> &truthTable, int outputLevel, vector<int> *levelSet, vector<int> &inputList)
{
	
    map<int, CircuitLine> masterLineList_m;
	map<int, CircuitLine>::iterator itrml;
	map<int, CircuitNode>::iterator itrm;
	vector <FaultList> orFaultList;
	masterLineList_m.clear();
	CreateFaultObjects(masterLineList_m, orNodeList_m);
	CreateFaultList(masterLineList_m, orFaultList, orNodeList_m);
	redFILE1 << "before collapse, #fault: "<<2*masterLineList_m.size()<<endl;
	redFILE1 << "Faults in original_sf: "<<orFaultList.size()<<endl;
	for(int j=0; j<orFaultList.size(); j++)
		redFILE1 << "("<<orFaultList[j].lineNumber <<","<<orFaultList[j].stuckAtValue<<"), ";
	redFILE1 <<endl; 
	
	//CollapseFaults(masterLineList_m, orNodeList_m);
	int count = 0;
	for(itrml = masterLineList_m.begin(); itrml != masterLineList_m.end(); itrml++)
	{
		if(itrml->second.isStuckAt_0) count++;
		if(itrml->second.isStuckAt_1) count++;
	}
	redFILE1 << "after collapse, #fault: "<<count<<endl;
	orFaultList.clear();
	CreateFaultList(masterLineList_m, orFaultList, orNodeList_m);
	redFILE1 << "Faults in original_sf: "<<orFaultList.size()<<endl;
	for(int j=0; j<orFaultList.size(); j++)
		redFILE1 << "("<<orFaultList[j].lineNumber <<","<<orFaultList[j].stuckAtValue<<"), ";
	redFILE1 <<endl; 
	
	int num_unt = 0;
	vector<FaultList> untFault;
	vector<FaultList>::iterator itrv_fault;
	while(!orFaultList.empty())
	{
	    itrv_fault = orFaultList.begin();
	    int line = (*itrv_fault).lineNumber;
	    int sa_value = (*itrv_fault).stuckAtValue;
	    cout << "("<<line<<", "<<sa_value<<")"<<endl;
	    redFILE1 << "---------------------------------"<<endl;
		redFILE1 << "("<<line<<", "<<sa_value<<")"<<endl;
		TestList test(line, sa_value);
		int true_ans = podem(orNodeList_m, *(itrv_fault), test, truthTable, outputLevel, levelSet, inputList);
		//bool true_ans = TestAllVectorsATPG (masterNodeList_m, totalInputs, line, sa_value, masterInputVector);
	    	    
		if(true_ans == true)
		{
		    cout << "*current fault is testable!"<<endl;
		    redFILE1 << "*current fault is testable!"<<endl;
		    redFILE1 << "test vector = "<<test.testVector<<endl;

		    /*vector<FaultList> leftFaultList;
		    run_fault_simulation(masterNodeList_m, orFaultList, leftFaultList, test.testVector); 
		    redFILE1 << "Faults in leftFaultList: "<<leftFaultList.size()<<endl;
	        for(int j=0; j<leftFaultList.size(); j++)
		        redFILE1 << "("<<leftFaultList[j].lineNumber <<","<<leftFaultList[j].stuckAtValue<<"), ";
	        redFILE1 <<endl; 
	        orFaultList.clear();
	        orFaultList = leftFaultList;*/
		}
		else
		{
		    cout << "*current fault is untestable!"<<endl;
		    //redFILE1 << "("<<itrm->second.lineNumber<<", 0)"<<endl;
		    redFILE1 << "*current fault is untestable!"<<endl;
		    itrm = orNodeList_m.find(line);
		    if(itrm != orNodeList_m.end())
		    {
		        untFault.push_back(*itrv_fault);
		        num_unt++;
		    }
		}
		orFaultList.erase(itrv_fault);
    }
    cout << "#unt = "<<num_unt<<endl;
    for(int i = 0; i < untFault.size(); i++)
    {
        redFILE1 << "("<<untFault[i].lineNumber<<", "<<untFault[i].stuckAtValue<<")"<<endl;
        cout << "("<<untFault[i].lineNumber<<", "<<untFault[i].stuckAtValue<<")"<<endl;
    }
    redFILE1 << endl;
    cout << endl;
    
}
