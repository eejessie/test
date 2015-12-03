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

#include "class/FaultList.h"
#include "class/TestList.h"
#include "function/atpg.h"
#include "function/merge_circuit.h"
#include "function/red_forward_deletion_v2.h"
#include "function/red_backward_deletion.h"
#include "function/run_logic_simulation.h"
#include "function/print_circuit.h"
#include "function/copy_point_vector.h"
#include "function/parallel_fault_simulation.h"
#include "function/helper.h"
#include "function/rem_red.h"
#include "podem/podem_v2.h"


// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

#define wordSize 32

static double fsTime = 0, atpgTime = 0, redTime = 0, newTime = 0;

extern vector<int> inputList;
extern vector <string> masterInputVector;
extern ofstream redFILE0, redFILE1, timeRecord;


bool remove_vector(vector<FaultList> & NumVec, vector<int> & FlagVec)
{
	  int iSize = (int)NumVec.size();  
	  if (iSize != (int)FlagVec.size())  
	  	return false;

	  int iWritePos = 0;  
	  for (int iIterPos = 0; iIterPos < iSize; iIterPos++)
	  {
	  	if (!FlagVec.at(iIterPos))  
	    {
	      if (iWritePos != iIterPos)  
	        NumVec.at (iWritePos) = NumVec.at (iIterPos);
	      iWritePos++;
	    }
	  }

	if (iSize != iWritePos)  
		 NumVec.resize(iWritePos);

  return true;
}


//Run redundancy removal based on new_sf		
void red_removal_new(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m, vector<FaultList> &new_sf, map<int, int> &truthTable)
{		
	//redFILE1 << "*********************************************"<<endl;
	//redFILE1<< "Enter into red_removal_new:"<<endl;
	//redFILE1 << "*********************************************"<<endl;

    vector<FaultList>::iterator itrvf;
	map<int, CircuitNode>::iterator itrm, itrm1;
	set <int>::iterator itrs;
	bool exist_line = 0;
	while(!new_sf.empty())
	{
		//Judge if line of this fault are in the current circuit.
		//If yes, continue the code; else, skip this fault.
		itrvf = new_sf.begin();
		itrm = orNodeList_m.find((*itrvf).lineNumber);
		if(itrm == orNodeList_m.end())
		{
			new_sf.erase(itrvf);
			continue;
		}

		int red_line = (*itrvf).lineNumber;
		bool red_stuck = (*itrvf).stuckAtValue;
	
		redFILE0 << endl << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
		redFILE0  << "Test if the saf is redundant:"<<endl;
		FaultList *thisFault;
		redFILE0  <<"("<<red_line<<","<<red_stuck<<")"<<endl;

		//Run atpg to test if this saf is redundant.
		//ATPG should be operated on the whole circuit "masterNodeList_m".
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
		TestList test(red_line, red_stuck);
		int isTestable = podem(orNodeList_m, (*itrvf), test, truthTable, outputLevel, levelSet, inputList);
		delete []levelSet;

		if(!isTestable) 
		{
			redFILE0 << "Current fault "<<"	("<<red_line<<","<<red_stuck<<")"<<" is untestable!"<<endl;
			redFILE0 << endl<< "Before forward deletion, circuit size is "<<orNodeList_m.size()<<endl;
			//redundancy forward deletion
			HashTable implicationNodeList;
			implicationNodeList.Init_HashTable();
			red_forward_deletion_v2(orNodeList_m, implicationNodeList, red_line, red_stuck);
			implicationNodeList.Destroy_HashTable();
			collapse_circuit(orNodeList_m);
			redFILE0 << endl<< "After forward deletion, circuit size is "<<orNodeList_m.size()<<endl;
			
			//redundancy backward deletion
			red_backward_deletion(orNodeList_m);
			collapse_circuit(orNodeList_m);
			redFILE0 << endl <<"After backward deletion, circuit size is "<<orNodeList_m.size()<<endl;

			masterNodeList_m.clear();
			int maxLineNumber = merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);			

		}
		else if(isTestable)
			redFILE0 << "Current fault "<<"	("<<red_line<<","<<red_stuck<<")"<<" is testable!"<<endl;

		new_sf.erase(itrvf);
		merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
	}
}


//Check if this line is in the original stuck-af-fault list.
int check_osf(map<int, CircuitNode> &buOrNodeList_m, map<int, CircuitNode> &masterNodeList_m, int lineNumber)
{
	vector<CircuitNode>::iterator itrv;
	vector<CircuitNode*>::iterator itrv1;
	map<int, CircuitNode>::iterator itrm, itrm1;


	for(itrm = buOrNodeList_m.begin(); itrm != buOrNodeList_m.end(); itrm++)
		if(itrm->first == lineNumber)
		{
			return 1;
		}

	if ( itrm == buOrNodeList_m.end())
	{
		itrm1 = masterNodeList_m.find(lineNumber);
		if(itrm1->second.listFanIn.size() == 1)
		{
			itrv1 = itrm1->second.pointFanIn.begin();
			for(int i=0; i < inputList.size(); i++)
				if((*itrv1)->lineNumber == inputList[i])
				{
					return 1;
					break;
				}
		}
	}
			
	return 0;
}


//red_removal
void red_removal_v2(map<int, CircuitNode> &buOrNodeList_m, map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m, map<int, int> &truthTable)
//buOrNodeList: initial circuit; check whether a CF is original or new
//orNodeList_m: run redundancy removal on it
//masterNodeList_m: to perform atpg
{
    //Iterators and variabls
	vector<FaultList> original_sf, new_sf;             
	vector<FaultList>::iterator itrvf;

	map <int, CircuitLine> masterLineList_m;
	vector <FaultList> orFaultList;	

	set<int>::iterator itrs;
	map<int, CircuitNode>::iterator itrm;
	map<int, CircuitLine>::iterator itrml;


	//Generate a fault list of stuck-at faults for the new original network.
	//Faults are stored in "orFaultList".
	masterLineList_m.clear();
	CreateFaultObjects(masterLineList_m, masterNodeList_m);
	redFILE1 << "before collapse, #fault: "<<2*masterLineList_m.size()<<endl;
	CollapseFaults(masterLineList_m, masterNodeList_m);
	int count = 0;
	for(itrml = masterLineList_m.begin(); itrml != masterLineList_m.end(); itrml++)
	{
		if(itrml->second.isStuckAt_0) count++;
		if(itrml->second.isStuckAt_1) count++;
	}
	redFILE1 << "after collapse, #fault: "<<count<<endl;
	CreateFaultList(masterLineList_m, orFaultList, masterNodeList_m);
	
	//Generate stuck-at-faults for original connections and new added connections
	for(itrvf = orFaultList.begin(); itrvf != orFaultList.end(); itrvf++)
		if (check_osf(buOrNodeList_m, masterNodeList_m, (*itrvf).lineNumber))     
			original_sf.push_back(*itrvf);
		else
			new_sf.push_back(*itrvf);

	redFILE0 << "Faults in original_sf: "<<original_sf.size()<<endl;
	for(int j=0; j<original_sf.size(); j++)
		redFILE0 << "("<<original_sf[j].lineNumber <<","<<original_sf[j].stuckAtValue<<"), ";
	redFILE0 <<endl; 

	redFILE0 << "Faults in new_sf: "<<new_sf.size()<<endl;
	for(int j=0; j<new_sf.size(); j++)
		redFILE0 << "("<<new_sf[j].lineNumber <<","<<new_sf[j].stuckAtValue<<"), ";
	redFILE0 <<endl; 
	
	
	//Remove redundancies based on original_sf.
	redFILE1 << "**start removal original redundancies:"<<endl;
	vector<string> testPool;
	int outputLevel = SetLineLevel(orNodeList_m);
	if(outputLevel == -1)
	    return;
	
	vector<int> *levelSet;
	levelSet = new vector<int>[outputLevel];
	int level;
    for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
    {
        level = itrm->second.lineLevel;
        if(level > 0)
            levelSet[level-1].push_back(itrm->second.lineNumber);
    }      
    multimap<int, FaultList> cost_sf; 
    int first_cost = 10000;
    compute_cost(orNodeList_m, original_sf, cost_sf, first_cost);     
	rem_max_red(orNodeList_m, brNodeList_m, masterNodeList_m, original_sf, cost_sf, testPool, truthTable, outputLevel, levelSet, first_cost);
	delete []levelSet;
	
	timeRecord << endl << endl <<"fsTime = "<<fsTime<<", atpgTime = "<<atpgTime<<", redTime = "<<redTime;
	timeRecord <<"orTime = "<<fsTime + atpgTime + redTime<<endl;
	
	//Remove redundancies based on new_sf.
	struct timeb startTime, endTime;
	ftime(&startTime);
	redFILE1 << "**start removal new redundancies:"<<endl;
	red_removal_new(orNodeList_m, brNodeList_m, masterNodeList_m, new_sf, truthTable);
	ftime(&endTime);
	newTime = newTime + ((endTime.time - startTime.time)*1000 + ((endTime.millitm - startTime.millitm)))/1000.0;
	timeRecord <<"newTime = "<<newTime<<endl;

	collapse_circuit(orNodeList_m);
}


