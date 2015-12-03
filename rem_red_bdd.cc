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
#include "function/parallel_test_simulation_bdd.h"
#include "function/ATPG_checker.h"
#include "podem/podem_bdd_v2.h"
#include "function/compute_MA.h"

#include "../../CUDD/cudd-2.4.1/cudd/cudd.h"
#include "../../CUDD/cudd-2.4.1/cudd/cuddInt.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0, redFILE1, untFile;
extern vector<int> inputList;
extern vector<string> masterInputVector;
extern int totalInputs;
extern vector<string> testPool;
int flag_unt = 1;


void rem_max_red_bdd(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &original_sf, multimap<int, FaultList> &cost_sf, vector<string> &testPool, DdManager *dd, DdNode *dnode, int outputLevel, vector<int> *levelSet, int &th_cost)
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
//	else  
	{
        struct timeb startTime, endTime;  
     
        FaultList objFault(itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
        int line = itrmf_r->second.lineNumber;
        int sa_value = itrmf_r->second.stuckAtValue;
        int flag_test = 0;
        if(testPool.size() > 0)
        {
            redFILE1 <<"start parallel pattern fault simulation:"<<endl;
            redFILE1 << "testPool's size: "<<testPool.size()<<endl;
       /*     for(int i = 0; i < testPool.size(); i++)
                redFILE1 << testPool[i]<<endl;*/
            FaultList fakeFault(-1, 0);
            ftime(&startTime);
                    
            vector<int> correct_output = parallel_test_simulation_bdd(orNodeList, fakeFault, testPool, dd, dnode, outputLevel, levelSet);
            vector<int> test_output = parallel_test_simulation_bdd(orNodeList, objFault, testPool, dd, dnode, outputLevel, levelSet);
            ftime(&endTime);
	        cout << "Time for fault simulation "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
	        redFILE1 << "Time for fault simulation "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
            
       /*     redFILE1 << "correct_output:"<<endl;
            for(int i = 0; i < correct_output.size(); i++)
                redFILE1 << correct_output[i];
            redFILE1 << endl;
            redFILE1 << "test_output:"<<endl;
            for(int i = 0; i < test_output.size(); i++)
                redFILE1 << test_output[i];
            redFILE1 << endl;*/
            if(correct_output != test_output)  //This fault is testable!
            {
                flag_test = 1;
                redFILE1 << "*Current fault is testable!"<<endl;
                multimap<int, FaultList>::iterator itrmf = cost_sf.end();
                itrmf--;
                cost_sf.erase(itrmf);
                rem_max_red_bdd(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf, testPool, dd, dnode, outputLevel, levelSet, th_cost);
            }
        }
        
        if(flag_test == 1)
            return;
            
        ftime(&startTime);
        TestList test(itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
        redFILE1 << "start podem:"<<endl;

        int isTestable = podem_bdd_v2(orNodeList, masterNodeList, objFault, test, dd, dnode, outputLevel, levelSet);

        ftime(&endTime);
        redFILE1 << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
        redFILE0 << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
	    cout << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;

        //if(!untestable)
        if(isTestable)
        {
            redFILE1 << "**Current fault is testable!"<<endl;
            multimap<int, FaultList>::iterator itrmf = cost_sf.end();
            itrmf--;
            cost_sf.erase(itrmf);
            int flag_exist = 0;
            for(int k = 0; k < testPool.size(); k++)
                if(testPool[k] == test.testVector)
                {
                    flag_exist = 1;
                    break;
                }
            if(flag_exist == 0)
                testPool.push_back(test.testVector);
            redFILE1 << "testVector = "<<test.testVector<<endl;
            rem_max_red_bdd(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf, testPool, dd, dnode, outputLevel, levelSet, th_cost);
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
            print_circuit(orNodeList, redFILE1);
            redFILE1 << "run logic simulation:"<<endl;
            run_logic_simulation(orNodeList, totalInputs, masterInputVector, dd, dnode);
            	    
		    merge_circuit(masterNodeList, orNodeList, brNodeList);

		    cost_sf.clear();
		
		    ftime(&startTime);
		    compute_cost_fanin(orNodeList, original_sf, cost_sf, th_cost);	
		    ftime(&endTime);
            redFILE1 << "Time for compute_cost: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
            
            //clear the old levelSet
        	for(int i = 0; i < outputLevel; i++)
        	    levelSet[i].clear();
        	
        	outputLevel = SetLineLevel(orNodeList, inputList);
        	//redFILE1 << "new output level = "<<outputLevel<<endl;
	        int level;
            for(itrm = orNodeList.begin(); itrm != orNodeList.end(); itrm++)
            {
                level = itrm->second.lineLevel;
                if(level > 0)
                    levelSet[level-1].push_back(itrm->second.lineNumber);
            }
		    rem_max_red_bdd(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf, testPool, dd, dnode, outputLevel, levelSet, th_cost);
				
        }
    }
}



void rem_red_bdd(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m, DdManager *dd, DdNode *dnode,  vector <FaultList> &orFaultList)
{

    map<int, CircuitNode>::iterator itrm;

	redFILE1 << "**start compute_cost:"<<endl;
	
	multimap<int, FaultList> cost_sf;
//	int first_cost = 100000;
//	compute_cost(orNodeList_m, orFaultList, cost_sf, first_cost);
  //  int th_cost = totalInputs/4;
    int th_cost = 0;
    compute_cost_fanin(orNodeList_m, orFaultList, cost_sf, th_cost);
	
	redFILE1 << "**start rem_max_red:"<<endl;

//	vector<string> testPool;
	int outputLevel = SetLineLevel(orNodeList_m, inputList);
	vector<int> *levelSet;
	levelSet = new vector<int>[outputLevel];
	int level;
    for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
    {
        level = itrm->second.lineLevel;
        if(level > 0)
            levelSet[level-1].push_back(itrm->second.lineNumber);
    }
            
	rem_max_red_bdd(orNodeList_m, brNodeList_m, masterNodeList_m, orFaultList, cost_sf, testPool, dd, dnode, outputLevel, levelSet, th_cost);
	delete []levelSet;
	        
	merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
	redFILE1 << "At the end of rem_red, circuit size = "<<orNodeList_m.size()<<endl;
}


