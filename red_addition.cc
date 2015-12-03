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
#include "lib/forward_implication.h"

#include "function/atpg.h"
#include "function/upd_circuit_add.h"
#include "function/del_gate.h"
#include "function/merge_circuit.h"
#include "function/print_circuit.h" 
#include "function/run_logic_simulation.h"
#include "function/copy_point_vector.h"
#include "function/helper.h"
#include "function/ATPG_checker.h"
#include "podem/podem_v2.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0, redFILE1, timeRecord;
extern vector<int> inputList;
extern vector<string> masterInputVector, masterInputVector1;

void find_tran_fanout(map <int, CircuitNode> &orNodeList_m, vector <CircuitNode> &tranFanOut, int lineNumber)
{
	map<int,CircuitNode>::iterator itrm, itrm1;
	set<int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv;

	itrm = orNodeList_m.find(lineNumber);
	if (itrm->second.nodeType == 3)         //If corrent line is a PO, then return;
		return;

	for(itrv=itrm->second.pointFanOut.begin(); itrv!=itrm->second.pointFanOut.end(); itrv++)
	{	
		itrm1 = orNodeList_m.find((*itrv)->lineNumber);
		if(search_vector_node(tranFanOut, itrm1->second.lineNumber) == -1)
			tranFanOut.push_back(itrm1->second);
		find_tran_fanout(orNodeList_m, tranFanOut, itrm1->second.lineNumber);
	}
}



/*Judge if both the source and destination of this fault are in the current circuit.
If yes, return 1; else, return 0.*/
int fault_exist(map<int, CircuitNode> &orNodeList_m, ConnFault &cf)
{
	map<int, CircuitNode>::iterator itrm;
	//set<int>::iterator itrs;

	bool exist_src = 0, exist_dest = 0;
	itrm = orNodeList_m.find(cf.src);
	if(itrm != orNodeList_m.end())
		exist_src = 1;
	
	itrm = orNodeList_m.find(cf.dest);
	if(itrm != orNodeList_m.end())
		exist_dest = 1;

	vector <CircuitNode> tranFanOut; 
	if((exist_src == 1) && (exist_dest == 1)) 
	{
		find_tran_fanout(orNodeList_m, tranFanOut, cf.dest);
		if(search_vector_node(tranFanOut, cf.src) != -1)
		{
			redFILE1 << "src is in the transistive fanout of dest."<<endl;
			return 0;		
		}
		else
		{
			//redFILE1 << "This fault exists in the current circuit."<<endl;
			return 1;
		}
	}
	else
	{
		redFILE1 << "This fault doesn't exist in the current circuit."<<endl;
		return 0;
	}
}

/*void find_fanout_gates(map<int, CircuitNode> &masterNodeList_m, int line, queue &Q)
{
	map<int, CircuitNode>::iterator itrm, itrm1;
	vector<CircuitNode*>::iterator itrv;
	
	itrm = masterNodeList_m.find(line);
	for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++)
	{
		Q.push((*itrv)->lineNumber);
		//redFILE1 << "Gate "<<(*itrv)->lineNumber<<"is pushed into Q"<<endl;
		itrm1 = masterNodeList_m.find((*itrv)->lineNumber);
		if((itrm1->second.nodeType == 2) || (itrm1->second.nodeType == 0 && itrm1->second.gateType == 1))
			find_fanout_gates(masterNodeList_m, itrm1->second.lineNumber, Q);
	}
}*/



void red_addition(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, vector<ConnFault> &cf, int threshold, vector<string> &testPool, map<int, int> &truthTable, int maxLineNumber)
{
    //Iterators and variables
	vector <TestList> masterTestList;
	vector<CircuitNode>::iterator itrv;
	vector<ConnFault>::iterator itrvc, itrvc1, itrvc2;
	vector<string>::iterator itrvs;
	set<int>::iterator itrs;
	map<int, CircuitNode>::iterator itrm;
	
	map<int, CircuitNode> buOrNodeList_m;
	static double fsTime = 0;
	struct timeb startTime, endTime;
	int totalInputs = inputList.size();

	int flag=0;
	int t = 0;
	int num_add = 0;
	while((!cf.empty()) && (t < threshold))
	{		
		//Backup the initial "orNodeList_m" and "masterNodeList".
		copy_point_vector1(orNodeList_m, buOrNodeList_m);
		
		
		redFILE1<<"**************************************************************"<<endl;
		itrvc = cf.begin();
		redFILE1<<"(S, D, P) = ("<<(*itrvc).src<<","<<(*itrvc).dest<<","<<(*itrvc).pola<<")"<<endl;
		
		//redFILE1 << "Before update: "<<endl;
		//print_circuit(orNodeList_m, redFILE1);

		//Judge if this CF exists in the current circuit.
		int exist = fault_exist(orNodeList_m, *itrvc);
		if(!exist)
		{
			cf.erase(itrvc);
			continue;
		}
			
		//Judge if the gateType of destination is satisfying.
		itrm = orNodeList_m.find((*itrvc).dest);
		int destType = itrm->second.gateType;
		if((destType != 2) && (destType != 3) && (destType != 4) && (destType != 6) && (destType != 7))
		{
			redFILE1 << "Gate type of "<<itrm->second.lineNumber<<" is "<<destType<<endl;
			redFILE1 << "Not satisfying."<<endl;
			cf.erase(itrvc);		
			continue;
		}
		
		//1. Update circuit to prepare for redundancy addition
		FaultList fault(-1, false);										
		int newID1 = upd_circuit_add(orNodeList_m, *itrvc, destType, fault, maxLineNumber);
		//redFILE1 << "Original circuit after upd_circuit_add: "<<endl;
		//print_circuit(orNodeList_m, redFILE1);

		vector<int> new_inputList =  inputList;
		//redFILE1 << "The new input is "<<fault.lineNumber<<", sf-value = "<<fault.stuckAtValue<<endl;
		new_inputList.push_back(fault.lineNumber);
			
		//2. Run atpg to test if this cf is redundant.
		int outputLevel = SetLineLevel(orNodeList_m);
	    vector<int> *levelSet;
	    levelSet = new vector<int>[outputLevel];
	    int level;
        for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
        {
            level = itrm->second.lineLevel;
            if(level > 0)
                levelSet[level-1].push_back(itrm->second.lineNumber);
        }		
		TestList test(fault.lineNumber, fault.stuckAtValue);
		map<int, CircuitNode> masterNodeList;
		merge_circuit(masterNodeList, orNodeList_m, brNodeList_m);
		//int isTestable = TestAllVectorsATPG(masterNodeList, totalInputs+1, fault.lineNumber, fault.stuckAtValue, masterInputVector1);
		int isTestable = podem(orNodeList_m, fault, test, truthTable, outputLevel, levelSet, new_inputList);
		
		delete []levelSet;
		
			
		//3. If this cf fault is untestable, then add this cf connection.
		if(!isTestable)
		{
			redFILE1<<"The current cf is untestable and added!"<<endl;
			num_add++;
			
			//If this connection is added, delete the auxiliary lines.
			//redFILE1 << endl << "**Starting del_gate:"<<endl;
			del_gate(orNodeList_m, newID1+2);
			//print_circuit(orNodeList_m, redFILE1);

			//redFILE1 << endl <<"**Run logic simulation after del_gate and merge_circuit"<<endl;	
			RunLogicSimulation(orNodeList_m, totalInputs, masterInputVector, truthTable);	
		}												
		//4. If this cf fault is testable, then recover the masterNodeList_m.
		else if(isTestable)
		{
			redFILE1 <<"The current cf is testable!"<<endl;
			copy_point_vector1(buOrNodeList_m, orNodeList_m);	
		}

		cf.erase(itrvc);	
		t = t + 1;	
		
			
	}
	
	redFILE1 << "number of added wires in this round: "<<num_add<<endl;
	//redFILE1 << "countTestableCF = "<<countTestableCF<<endl;
	//timeRecord << endl << "fsTime = "<<fsTime;

}















