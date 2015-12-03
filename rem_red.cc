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
#include <ctime>
#include <sys/timeb.h>

#include "class/CircuitNode.h"        
#include "class/CircuitLine.h" 
#include "class/ConnFault.h"
#include "function/copy_point_vector.h"
#include "function/print_circuit.h"
#include "function/helper.h"
#include "function/atpg.h"
#include "function/merge_circuit.h"
#include "function/run_logic_simulation.h"
#include "function/red_forward_deletion_v2.h"
#include "function/red_backward_deletion.h"
#include "function/parallel_test_simulation.h"
#include "function/ATPG_checker.h"
//#include "podem/podem_v2.h"
#include "podem/podem_sim.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0, redFILE1, untFile;
extern vector<int> inputList;
extern vector<string> masterInputVector;
extern int totalInputs;
int flag_unt = 1;


void rem_max_red(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &original_sf, multimap<int, FaultList> &cost_sf, vector<string> &testPool, int outputLevel, vector<int> *levelSet, int &first_cost)
{
    map<int, CircuitNode>::iterator itrm;

    if(cost_sf.empty())
        return;

     multimap<int, FaultList>::reverse_iterator itrmf_r;
	 itrmf_r = cost_sf.rbegin(); 
	 cout << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;
	 redFILE1 << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;
	 redFILE0 << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl; 
    
	redFILE1 << "cost of current fault: "<<itrmf_r->first<<endl;
    struct timeb startTime, endTime;  
 
    FaultList objFault(itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
    int line = itrmf_r->second.lineNumber;
    int sa_value = itrmf_r->second.stuckAtValue;
    int flag_test = 0;
    redFILE1 <<"testPool's size = "<<testPool.size()<<endl;
    if(testPool.size() > 0)
    {
        redFILE1 <<"start parallel pattern fault simulation:"<<endl;
        //cout <<"testPool's size = "<<testPool.size()<<endl;
        
        FaultList fakeFault(-1, 0);
        ftime(&startTime);
                
        vector<int> correct_output = parallel_test_simulation(masterNodeList, fakeFault, testPool, outputLevel, levelSet);
        vector<int> test_output = parallel_test_simulation(masterNodeList, objFault, testPool, outputLevel, levelSet);
        ftime(&endTime);
	    cout << "Time for fault simulation "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
	    //redFILE1 << "Time for fault simulation "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
        
        if(correct_output != test_output)  //This fault is testable!
        {
            flag_test = 1;
            redFILE1 << "*Current fault is testable!"<<endl;
            multimap<int, FaultList>::iterator itrmf = cost_sf.end();
            itrmf--;
            cost_sf.erase(itrmf);
            rem_max_red(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf, testPool, outputLevel, levelSet, first_cost);
        }
    }
    
    if(flag_test == 1)
        return;
        
    ftime(&startTime);
    TestList test(itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
    redFILE1 << "start podem:"<<endl;

  //  int isTestable = podem(orNodeList, objFault, test, truthTable, outputLevel, levelSet, inputList);
    int isTestable = podem_sim(orNodeList, masterNodeList, objFault, test, outputLevel, levelSet);

    ftime(&endTime);
    redFILE1 << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
    redFILE0 << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
	cout << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;

    if(isTestable)
    {
        redFILE1 << "**Current fault is testable!"<<endl;
        redFILE1 << "Test vector = "<<test.testVector<<endl;
        multimap<int, FaultList>::iterator itrmf = cost_sf.end();
        itrmf--;
        cost_sf.erase(itrmf);
        update_testpool(testPool, test.testVector);
       
        rem_max_red(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf, testPool, outputLevel, levelSet, first_cost);
    }        
    else
    {
        cout << "**Current fault is untestable!"<<endl;
        redFILE1 << "**Current fault is untestable!"<<endl;
        flag_unt = 1;
        
        untFile<<"("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"), cost = "<<itrmf_r->first<<endl;
        
        int old_size = orNodeList.size();        
        simplify_circuit(orNodeList, itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
        int new_size = orNodeList.size();
        int cost = old_size - new_size;
        
        redFILE1 << "after remove redundancy, circuit size = "<<orNodeList.size()<<", cost = "<<cost<<endl;
        merge_circuit(masterNodeList, orNodeList, brNodeList);
    /*    print_circuit(masterNodeList, redFILE1);
        redFILE1 << "run logic simulation:"<<endl;
        run_logic_simulation(masterNodeList, totalInputs, masterInputVector);*/
        	    
		cost_sf.clear();
		
		ftime(&startTime);
		compute_cost(orNodeList, original_sf, cost_sf, first_cost);	
		ftime(&endTime);
        redFILE1 << "Time for compute_cost: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
        
        //clear the old levelSet
    	for(int i = 0; i < outputLevel; i++)
    	    levelSet[i].clear();
    	
    	outputLevel = SetLineLevel(masterNodeList);
	    int level;
        for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
        {
            level = itrm->second.lineLevel;
            if(level > 0)
                levelSet[level-1].push_back(itrm->second.lineNumber);
        }
		rem_max_red(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf, testPool, outputLevel, levelSet, first_cost);
				
    }
}



void rem_red(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m, vector <FaultList> &orFaultList)
{
    map<int, CircuitNode>::iterator itrm;

	redFILE1 << "**start compute_cost:"<<endl;
	
	multimap<int, FaultList> cost_sf;
	int first_cost = 100000;
	compute_cost(orNodeList_m, orFaultList, cost_sf, first_cost);
	
	        redFILE1 << "**start rem_max_red:"<<endl;

	        vector<string> testPool;
	        int outputLevel = SetLineLevel(masterNodeList_m);
	        vector<int> *levelSet;
	        levelSet = new vector<int>[outputLevel];
	        int level;
            for(itrm = masterNodeList_m.begin(); itrm != masterNodeList_m.end(); itrm++)
            {
                level = itrm->second.lineLevel;
                if(level > 0)
                    levelSet[level-1].push_back(itrm->second.lineNumber);
            }
            
	        rem_max_red(orNodeList_m, brNodeList_m, masterNodeList_m, orFaultList, cost_sf, testPool, outputLevel, levelSet, first_cost);
	        delete []levelSet;
	        
	        merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
	        redFILE1 << "At the end of rem_red, circuit size = "<<orNodeList_m.size()<<endl;
}


