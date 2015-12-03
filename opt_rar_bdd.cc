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
#include "lib/radix_convert.h"
#include "function/read_circuit_v2.h"
#include "function/print_circuit.h"
#include "function/copy_point_vector.h"
#include "function/atpg.h"
#include "function/merge_circuit.h"
#include "function/helper.h"
#include "function/run_logic_simulation.h"
#include "function/red_forward_deletion_v2.h"
#include "function/red_backward_deletion.h"
#include "function/compute_MA.h"
#include "function/compute_MA_or.h"
#include "function/upd_circuit.h"
#include "function/upd_circuit_plus.h"
#include "function/idtf_unt.h"
#include "function/ATPG_checker.h"
#include "podem/podem_bdd_v2.h"
#include "function/parallel_test_simulation_bdd.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cudd.h"
#include "/home/eejessie/work/CUDD/cudd-2.4.1/cudd/cuddInt.h"


#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern vector<string> testPool;	
int original_size;
extern vector<int> inputList;
extern ofstream redFILE0, redFILE1, timeRecord;
extern vector<string> masterInputVector;
static double runtime_step1 = 0, runtime_step2 = 0, runtime_fs = 0, runtime_podem = 0, runtime_step3 = 0, runtime_loop = 0;


int cal_cost(map<int, CircuitNode> &tmp_orNodeList, int sa_line, int sa_value)
{
    //print_circuit(tmp_orNodeList, redFILE1);

    int old_size = tmp_orNodeList.size();
    
    simplify_circuit(tmp_orNodeList, sa_line, sa_value);
    
    int new_size = tmp_orNodeList.size();
    int cost = old_size - new_size;

    return cost;
    
}

void rar_bdd(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, vector<FaultList> &orFaultList, multimap<int, FaultList> &cost_sf, DdManager *dd, DdNode *dnode, int &first_cost, vector<string> &testPool)
{
    redFILE1 << endl<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
    redFILE1 << "Coming into rar!"<<endl;
    redFILE0 << "Coming into rar!"<<endl;
    redFILE1 << "first_cost = "<<first_cost<<endl;
    
     if(cost_sf.empty())
        return;
        
    map<int, int>::iterator itrmi, itrmi1;
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2;
    vector<CircuitNode*>::iterator itrv;
    set<int>::iterator itrs, itrs1;
    multimap<int, FaultList>::reverse_iterator itrmf_r, itrmf_r1;
    multimap<int, FaultList>::iterator itrmf;
        	
	itrmf_r = cost_sf.rbegin(); 
	int this_cost = itrmf_r->first;
	int sa_line = itrmf_r->second.lineNumber;
	itrm = orNodeList.find(sa_line);
	while(itrm == orNodeList.end())
	{
	    if(!cost_sf.empty())
	    {
	        itrmf = cost_sf.end();
	        itrmf--;
	        cost_sf.erase(itrmf);
	        if(!cost_sf.empty())
	        {
	            itrmf_r = cost_sf.rbegin(); 
	            sa_line = itrmf_r->second.lineNumber;
	            itrm = orNodeList.find(sa_line);
	            this_cost = itrmf_r->first;
	        }
	        else break;
	    }
	    else break;
	}
	if(cost_sf.empty())
	    return;
	    
	int sa_value = itrmf_r->second.stuckAtValue;
	cout << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;	
	redFILE1 << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;
	redFILE0 << endl << "In rar, ("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl; 
	redFILE1 << "cost of current fault: "<<this_cost<<endl;
	//print_circuit(orNodeList, redFILE1);
	
	struct timeb startTime, endTime;
	struct timeb startTime1, endTime1; 
	struct timeb startTime2, endTime2;      
	struct timeb startTime3, endTime3;    	 
	
	//step1. compute the SMA of current fault	
	ftime(&startTime); 
	 
	redFILE1 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	redFILE1 << "step1. Compute the MAs of current fault:"<<endl;
	redFILE0 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	redFILE0 << "step1. Compute the MAs of current fault:"<<endl;
	
	map<int, int> MA_act, MA_obs, MA_forced;
	int totalInputs = inputList.size();	
	int untestable = compute_MA(orNodeList, masterNodeList, itrmf_r->second, MA_act, MA_obs, MA_forced);	
	ftime(&endTime);
	double runtime_s1 = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	redFILE1 << "runtime_s1 = "<<runtime_s1<<endl;
	runtime_step1 += runtime_s1;
	timeRecord << endl << "step1: "<<runtime_s1<<endl;
	
	if(untestable == 1)
	{
	    redFILE1 << "*This fault is untestable!"<<endl;
	    redFILE1 << "In opt_rar, direct implication fails "<<endl;
	    simplify_circuit(orNodeList, itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
        redFILE1 << "after remove redundancy, circuit size = "<<orNodeList.size()<<endl;
        redFILE1 << "Circuit size is reduced."<<endl;   
		merge_circuit(masterNodeList, orNodeList, brNodeList);
        rar_bdd(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, dd, dnode, first_cost, testPool);
        
    }	
	else
	{
	    HashTable tranfanout;
        tranfanout.Init_HashTable();
        find_tranfanout_hash(masterNodeList, tranfanout, sa_line);	    

	    itrmf = cost_sf.end();
	    itrmf--;
	    cost_sf.erase(itrmf);
	    
	    //step2. construt the set of candadicate wires
	    ftime(&startTime);	    
	    redFILE1 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	    redFILE1 << "step2. Construt the set of candadidate wires"<<endl;
	    redFILE0 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	    redFILE0 << "step2. Construt the set of candadidate wires"<<endl;
	    redFILE1 << "MA_act:"<<endl;
	    for(itrmi = MA_act.begin(); itrmi != MA_act.end(); itrmi++)
	        redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";      
	    redFILE1 << endl << "MA_obs:"<<endl;    
	    for(itrmi = MA_obs.begin(); itrmi != MA_obs.end(); itrmi++)
	        redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";        
	    redFILE1 << endl << "MA_forced:"<<endl;
	    for(itrmi = MA_forced.begin(); itrmi != MA_forced.end(); itrmi++)
	        redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";
	     redFILE1 << endl;
	    
	    itrm = orNodeList.find(sa_line);
	    int flag_branch;
	    if(itrm->second.nodeType == 2) //branch
	        flag_branch = 1;
	    else 
	        flag_branch = 0;
	    multimap<int, int> cand_set;
	    for(itrmi = MA_act.begin(); itrmi != MA_act.end(); itrmi++)
	    {
	        if(itrmi->second != 0 && itrmi->second != 1)
	            continue;
	        itrm1 = orNodeList.find(itrmi->first);
	        if(itrm1 == orNodeList.end())
	            continue;
	      //  itrm1 = masterNodeList.find(itrmi->first);	        
	        if(itrm1->second.circuitIndex != 1)
	            continue;
	        if(itrm1->second.gateType == 5)    //ignore the inverters
	            continue;
	        if(itrm1->second.nodeType == 2 || itrm1->second.gateType == 9 || itrm1->second.gateType == 10 || itrm1->second.gateType == 11)
	            continue;
	            	       
	        int p;
	        if(tranfanout.Search_Hash(itrm1->second.lineNumber, p) == 1)
	            continue;
	        int ns = itrmi->first, nd;                
            HashTable tranfanin_ns;
            tranfanin_ns.Init_HashTable();
            find_tranfanin_hash(masterNodeList, tranfanin_ns, ns);
            tranfanin_ns.Traverse_HashTable();
            HashTable tranfanout_ns;
            tranfanout_ns.Init_HashTable();
            tranfanout_ns.Traverse_HashTable();
            find_tranfanout_hash(masterNodeList, tranfanout_ns, ns);
            ftime(&endTime1);                  
	        for(itrmi1 = MA_forced.begin(); itrmi1 != MA_forced.end(); itrmi1++)
	        {
	            itrm2 = orNodeList.find(itrmi1->first);
	            if(itrm2 == orNodeList.end())
	                continue;
	            if(itrmi1->first == ns)
	                continue;
	          //  itrm2 = masterNodeList.find(itrmi1->first);	
	            if(itrm2->second.gateType == 9 || itrm2->second.gateType == 10 || itrm2->second.gateType == 11)
	                continue;
	            if(itrm2->second.gateType == 5)
	                continue;
	            if(flag_branch == 1 && itrmi1->first == sa_line)
	                continue;
	            int p1;
	            if(tranfanin_ns.Search_Hash(itrmi1->first, p1) == 1)
	                continue;
	            if(tranfanout_ns.Search_Hash(itrmi1->first, p1) == 1)
	                continue;
	            nd = itrmi1->first;	                                                   
	            cand_set.insert(pair<int, int>(ns, nd));                  
	        }
	        tranfanin_ns.Destroy_HashTable();
	        tranfanout_ns.Destroy_HashTable();	    
	    }	    
	    tranfanout.Destroy_HashTable();
	    redFILE1 << "cand_set: "<<cand_set.size()<<endl;
	    for(itrmi = cand_set.begin(); itrmi != cand_set.end(); itrmi++)
	        redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";
	    redFILE1<<endl;
	    
	    ftime(&endTime);
	    double runtime_s2 = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	    redFILE1 << "runtime_s2 = "<<runtime_s2<<endl;
	    runtime_step2 += runtime_s2;
	    timeRecord << "step2: "<<runtime_s2<<endl;
	
    	//step3. check the validality of all candadicate wires    	
    	redFILE1 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    	redFILE1 << "step3. Check the validality of all candadicate wires"<<endl;
    	redFILE0 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    	redFILE0 << "step3. Check the validality of all candadicate wires"<<endl;
   if(!cand_set.empty())
   {
    	itrm = brNodeList.end();
    	itrm--;
    	int maxLineNumber_br = itrm->second.lineNumber;
    	
    	multimap<int, int>::iterator itrmmi;    	
    	int flag = 0;
    	ftime(&startTime);    	
    	for(itrmmi = cand_set.begin(); itrmmi != cand_set.end(); itrmmi++)
    	{    	   
    	    //step1. Obtain ns_set & nd_set &maxLineNumber & wa_value
    	    ftime(&startTime2); 	
    	    int ns = itrmmi->first;    
    	    int nd = itrmmi->second; 
    	    redFILE1 << "ns = "<<ns<<", nd = "<<nd<<endl;   	    
    	    itrmi = MA_act.find(ns);
    	    int ns_value = itrmi->second;    	    
    	    itrmi1 = MA_forced.find(nd);
    	    int nd_value = itrmi1->second;
    	    map<int, int> ns_set, nd_set;
    	    ns_set.insert(pair<int, int>(ns, ns_value));
    	    nd_set.insert(pair<int, int>(nd, nd_value));
    	    
    	    itrm = orNodeList.end();
    	    itrm--;
    	    int maxLineNumber_or = itrm->second.lineNumber;
    	    int maxLineNumber;
    	    if(maxLineNumber_br > maxLineNumber_or)
    	        maxLineNumber = maxLineNumber_br+100;
    	    else
    	         maxLineNumber = maxLineNumber_or+100; 
    	             	    
    	    //step2. update the circuit by adding the candidate wire and obtain wa_line
    	    int last_size = orNodeList.size();
    	    map<int, CircuitNode> tmp_orNodeList;
    	    copy_point_vector1(orNodeList, tmp_orNodeList);  
    	    int wa_line = upd_circuit(tmp_orNodeList, ns_set, nd_set, maxLineNumber);
    	    map<int, CircuitNode> tmp_masterNodeList;
            merge_circuit(tmp_masterNodeList, tmp_orNodeList, brNodeList);
    	    maxLineNumber += 4;    
    	    redFILE1 << endl << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;
    	    redFILE1 << "*Current candidate wire is ("<<ns<<", "<<nd<<"), new lineNumber = "<<wa_line<<endl;
    	    redFILE1 << "ns_value = "<<ns_value<<", nd_value = "<<nd_value<<endl;
    	    int wa_value;    	   
    	    if(nd_value == 0 || nd_value == D)
    	        wa_value = 0;
    	    else if(nd_value == 1 || nd_value == B)
    	        wa_value = 1;    	        
    	    redFILE1 << "wa_value = "<<wa_value<<endl;
 
    	    //step3. Check whether the candidate wire wa_line is untestable by fault simulation   
    	    int isTestable_wa_ff = 0, isTestable_wa_podem = 0;
    	    FaultList objFault(wa_line, wa_value);	
    	    int outputLevel = SetLineLevel(tmp_orNodeList, inputList);
	        vector<int> *levelSet;
	        levelSet = new vector<int>[outputLevel];
	        int level;
            for(itrm = tmp_orNodeList.begin(); itrm != tmp_orNodeList.end(); itrm++)
            {
                level = itrm->second.lineLevel;
                if(level > 0)
                   levelSet[level-1].push_back(itrm->second.lineNumber);
            }                         
            redFILE1 << "testPool's size = "<<testPool.size()<<endl;        
    	    if(testPool.size() > 0)
            {
                redFILE1 <<"start parallel pattern fault simulation:"<<endl;
                FaultList fakeFault(-1, 0);                                          
                vector<int> correct_output = parallel_test_simulation_bdd(tmp_orNodeList, fakeFault, testPool, dd, dnode, outputLevel, levelSet);
                vector<int> test_output = parallel_test_simulation_bdd(tmp_orNodeList, objFault, testPool, dd, dnode, outputLevel, levelSet);

                if(correct_output != test_output)  //This fault is testable!
                {
                    isTestable_wa_ff = 1;
                    delete []levelSet;
                    redFILE1 << "ff: *Current fault is testable!"<<endl;
                }
            } 
            ftime(&endTime2);
            double runtime_ff = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
    	    redFILE1 << "runtime_ff = "<<runtime_ff<<endl;
    	    runtime_fs += runtime_ff;
    	    
    	    //step4. If fault simulation cannot tell whether wa_line is testable, then use podem to tell. Other wise, try adding another gate to make wa_line untestable.
    	    if(isTestable_wa_ff == 0)    	        
            {
                //use podem to check if wa_line is testable.
        	    redFILE1 << "start podem: "<<endl;  	    
        	    typedef multimap<int, FaultList>::iterator PAT;     	        
        	    TestList test(wa_line, wa_value);
        	    isTestable_wa_podem = podem_bdd_v2(tmp_orNodeList, tmp_masterNodeList, objFault, test, dd, dnode, outputLevel, levelSet);
        	    delete []levelSet;
        	    redFILE1 << "isTestable_wa_podem = "<<isTestable_wa_podem<<endl; 
           	    flag = 0;
           	    //If untestable, do removal as much as possible.
        	    if(isTestable_wa_podem == 0)
        	    {    	        
        	        redFILE1 << "find alternative wires!"<<endl; 
        	        redFILE1 << "current target fault: ("<<sa_line<<","<<sa_value<<"), cost = "<<this_cost<<endl;
        	        itrm = orNodeList.find(ns);             	        
        	        //Run FILE on wa to find more newly untestable wires
        	        multimap<int, int> ICA;
                    ICA.insert(pair<int, int>(wa_line, 0));
                	ICA.insert(pair<int, int>(wa_line, 1));  
                	multimap<int, FaultList> untFaultSet;             	    	    	
        	        idtf_unt(tmp_orNodeList, tmp_masterNodeList, ICA, untFaultSet);
        	        map<int, CircuitNode> bak_tmp_orNodeList;
        	        copy_point_vector1(tmp_orNodeList, bak_tmp_orNodeList);
        	        multimap<int, FaultList> newly_unt_faults;        	        
        	        redFILE1 << "newly untestable wires: "<<untFaultSet.size()<<endl;
        	        int flag_sa = 0;
        	        for(PAT ite = untFaultSet.begin(); ite != untFaultSet.end(); ite++)
        	        {             	               	            
        	            if(ite->first == wa_line)
        	                continue;
        	            redFILE1 << "("<<ite->first<<","<<ite->second.stuckAtValue<<")  ";
        	            if(ite->first == sa_line && ite->second.stuckAtValue == sa_value)
        	                flag_sa = 1;
        	            FaultList fault(ite->first, ite->second.stuckAtValue);
        	            int current_cost = cal_cost(tmp_orNodeList, ite->first, ite->second.stuckAtValue);
        	            newly_unt_faults.insert(pair<int, FaultList>(current_cost, fault));
        	            copy_point_vector1(bak_tmp_orNodeList, tmp_orNodeList);
        	            redFILE1 << "cost = "<<current_cost<<endl;
        	        }
        	        redFILE1 << endl;    	        
        	        if(flag_sa == 0)
        	        {
        	            FaultList fault(sa_line, sa_value);
        	            newly_unt_faults.insert(pair<int, FaultList>(this_cost, fault));
        	            redFILE1 << "("<<sa_line<<","<<sa_value<<")  ";
        	            redFILE1 << "cost = "<<this_cost<<endl;
        	        }    	        
                    redFILE1 << "last size = "<<last_size<<endl;                    
                    if(newly_unt_faults.size() == 1)
                    {
                        redFILE1 << "No other new untestable faults exist!"<<endl;
                        simplify_circuit(tmp_orNodeList, sa_line, sa_value);
                    //    redFILE1 << "1. Run logic simulation on the whole circuit:"<<endl;
	               //     run_logic_simulation(tmp_orNodeList, totalInputs, masterInputVector, dd, dnode);
                        redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;        
                    }
                    else 
                    {
                        redFILE1 << "##Check newly untestable faults!"<<endl;
                        int k = 0;
                        for(itrmf_r1 = newly_unt_faults.rbegin(); itrmf_r1 != newly_unt_faults.rend(); itrmf_r1++, k++)
                        {
                            int nsa_line = itrmf_r1->second.lineNumber;
                            int nsa_value = itrmf_r1->second.stuckAtValue;
                            int ncost = itrmf_r1->first;
                            redFILE1 << "current fault: ("<<nsa_line<<","<<nsa_value<<"), cost = "<<ncost<<endl;
                            if(k == 0)
                            {
                                simplify_circuit(tmp_orNodeList, nsa_line, nsa_value);
                                redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;
                       //         redFILE1 << "2. Run logic simulation on the whole circuit:"<<endl;
	                   //         run_logic_simulation(tmp_orNodeList, totalInputs, masterInputVector, dd, dnode);
                                continue;
                            } 
                            itrm = tmp_orNodeList.find(nsa_line);
                            if(itrm == tmp_orNodeList.end())
                                continue;                            
                            int outputLevel = SetLineLevel(tmp_orNodeList, inputList);
	                        vector<int> *levelSet;
	                        levelSet = new vector<int>[outputLevel];
	                        int level;
                            for(itrm = tmp_orNodeList.begin(); itrm != tmp_orNodeList.end(); itrm++)
                            {
                                level = itrm->second.lineLevel;
                                if(level > 0)
                                    levelSet[level-1].push_back(itrm->second.lineNumber);
                            }
                            TestList test(nsa_line, nsa_value);
                            redFILE1 << "2. start podem: "<<endl;
                            map<int, CircuitNode> tmp_masterNodeList_new;
                            merge_circuit(tmp_masterNodeList_new, tmp_orNodeList, brNodeList);
                    	    int isTestable = podem_bdd_v2(tmp_orNodeList, tmp_masterNodeList_new, itrmf_r1->second, test, dd, dnode, outputLevel, levelSet);
                    	    delete []levelSet;
                            if(!isTestable)
                            {
                                simplify_circuit(tmp_orNodeList, nsa_line, nsa_value);
                                redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;
                         //       redFILE1 << "3. Run logic simulation on the whole circuit:"<<endl;
	                    //        run_logic_simulation(tmp_orNodeList, totalInputs, masterInputVector, dd, dnode);
                            } 
                            else
                            {
                                int flag_exist = 0;
                                for(int k = 0; k < testPool.size(); k++)
                                    if(testPool[k] == test.testVector)
                                    {
                                        flag_exist = 1;
                                        break;
                                    }
                                if(flag_exist == 0)
                                    testPool.push_back(test.testVector); 
                            }  
                        }
                    }                    
                    int new_size = tmp_orNodeList.size();
                    if(new_size < last_size)
                    {
                        redFILE1 << "circuit size is reduced, cost of this fault = "<<this_cost<<endl;
                        print_circuit(tmp_orNodeList, redFILE1);
                        if(last_size == original_size)
                            first_cost = this_cost;
                        flag = 1;    	            
                        copy_point_vector1(tmp_orNodeList, orNodeList);
                        merge_circuit(masterNodeList, orNodeList, brNodeList);
                    }                                  
            	    if(flag == 1)
            	         break;       
        	    }
        	    else if(isTestable_wa_podem == 1)//If wa_line is testable, try adding another gate
        	    {
        	        redFILE1 << "This candidate is testable!"<<endl;
        	        redFILE1 << "This test vector = "<<test.testVector<<endl;        	    
        	        int flag_exist = 0;
                    for(int k = 0; k < testPool.size(); k++)
                        if(testPool[k] == test.testVector)
                        {
                            flag_exist = 1;
                            break;
                        }
                    if(flag_exist == 0)
                        testPool.push_back(test.testVector);
                }            
            }//if(isTestable_wa_ff == 0)  
 
            if((isTestable_wa_ff == 1 || isTestable_wa_podem == 1) && this_cost > 6)
            {
                redFILE1 << "try adding another gate, this_cost = "<<this_cost << endl;
                map<int, int> MA_act_wa, MA_obs_wa, MA_forced_wa;
                int isTestable = compute_MA(tmp_orNodeList, tmp_masterNodeList, objFault, MA_act_wa, MA_obs_wa, MA_forced_wa);	
                if(isTestable == 1)
                    redFILE1 << "wrong: this fault is testable!"<<endl;
                map<int, int> MA_sa, MA_wa, ndp_set;
                MA_sa = MA_act;
                for(itrmi = MA_obs.begin(); itrmi != MA_obs.end(); itrmi++)
                    MA_sa.insert(pair<int, int>(itrmi->first, itrmi->second));
                MA_wa = MA_act_wa;
                for(itrmi = MA_obs_wa.begin(); itrmi != MA_obs_wa.end(); itrmi++)
                    MA_wa.insert(pair<int, int>(itrmi->first, itrmi->second));
                redFILE1 << "satisfying MA:"<<endl;
                for(itrmi = MA_sa.begin(); itrmi != MA_sa.end(); itrmi++)
                {
                    itrm = tmp_orNodeList.find(itrmi->first);
                    if(itrm == tmp_orNodeList.end())
                        continue;
                    itrmi1 = MA_wa.find(itrmi->first);
                    if(itrmi1 != MA_wa.end())
                    { 
                        itrm1 = tmp_orNodeList.find(itrmi1->first);
                        if(itrm1 == tmp_orNodeList.end())
                            continue;
                        if(itrmi->second + itrmi1->second == 1)
                        {
                            redFILE1 << "&("<<itrmi1->first<<","<<itrmi1->second<<")"<<endl;
                            ndp_set.insert(pair<int, int>(itrmi1->first, itrmi1->second));
                        }
                    }    
                }                
                if(ndp_set.empty())
                    continue;
                else 
                {
                    redFILE1 << "current target fault: ("<<sa_line<<","<<sa_value<<"), cost = "<<this_cost<<endl;
                    map<int, int> wa_set;
                    wa_set.insert(pair<int, int>(wa_line, wa_value));
                    map<int, CircuitNode> bak_tmp_orNodeList;
                    copy_point_vector1(tmp_orNodeList, bak_tmp_orNodeList);
                    for(itrmi = ndp_set.begin(); itrmi != ndp_set.end(); itrmi++)
                    {
                        upd_circuit_plus(tmp_orNodeList, wa_set, ndp_set, maxLineNumber); 
                        simplify_circuit(tmp_orNodeList, sa_line, sa_value);
                        redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;
                   //     redFILE1 << "4. Run logic simulation on the whole circuit:"<<endl;
	              //      run_logic_simulation(tmp_orNodeList, totalInputs, masterInputVector, dd, dnode);
                        redFILE1 << "last_size = "<<last_size<<endl;
                        int new_size = tmp_orNodeList.size();
                        if(new_size < last_size)
                        {
                            redFILE1 << "circuit size is reduced, cost of this fault = "<<this_cost<<endl;                   
                            print_circuit(tmp_orNodeList, redFILE1);         
                            if(last_size == original_size)
                                first_cost = this_cost;   	            
                            copy_point_vector1(tmp_orNodeList, orNodeList);
                            merge_circuit(masterNodeList, orNodeList, brNodeList);
                            flag = 1;
                            break;
                        }  
                        else
                            copy_point_vector1(bak_tmp_orNodeList, tmp_orNodeList);                          
                    }//for(itrmi = ndp_set.begin(); itrmi != ndp_set.end(); itrmi++)
                }//else if(!ndp_set.empty())  
                if(flag == 1)
                    break;                
            }//if(isTestable_wa_ff == 1 || isTestable_wa_podem == 1)    
    	}//for(cand_set) loop
	
 /*   	if(flag == 1)
    	{
    	    cost_sf.clear();   
    	    compute_cost(orNodeList, orFaultList, cost_sf, first_cost);
    	}*/
    	
		ftime(&endTime);
		double runtime_s3 = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
		//redFILE1 << "runtime_s3: "<<runtime_s3<<endl;
		runtime_step3 += runtime_s3;
		redFILE1 << "runtime_step3 = "<<runtime_step3<<endl; 

     } //if(!cand_set.empty())  
     
     rar_bdd(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, dd, dnode, first_cost, testPool);
         
	
	}
	
}

void opt_rar_bdd(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &orFaultList, DdManager *dd, DdNode *dnode)
{
    multimap<int, FaultList> cost_sf;
    
    int first_cost = 100000;
	compute_cost(orNodeList, orFaultList, cost_sf, first_cost);
	
	original_size = orNodeList.size();
		
//	vector<string> testPool;	
	rar_bdd(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, dd, dnode, first_cost, testPool);
	
	redFILE1 << "runtime_step1 = "<<runtime_step1<<endl;
	redFILE1 << "runtime_step2 = "<<runtime_step2<<endl;
//	redFILE1 << "runtime_FILE = "<<runtime_FILE<<endl;
	redFILE1 << "runtime_fs = "<<runtime_fs<<endl;
	redFILE1 << "runtime_podem = "<<runtime_podem<<endl;
//	redFILE1 << "runtime_rem = "<<runtime_rem<<endl;
	redFILE1 << "runtime_step3 = "<<runtime_step3<<endl;
	
	cout << "runtime_step1 = "<<runtime_step1<<endl;
	cout << "runtime_step2 = "<<runtime_step2<<endl;
	//cout << "runtime_FILE = "<<runtime_FILE<<endl;
	cout << "runtime_fs = "<<runtime_fs<<endl;
	cout << "runtime_podem = "<<runtime_podem<<endl;
	//cout << "runtime_rem = "<<runtime_rem<<endl;
//	cout << "runtime_comp = "<<runtime_comp<<endl;
//	cout << "runtime_find = "<<runtime_find<<endl;
//	cout << "runtime_loop = "<<runtime_loop<<endl;
	cout << "runtime_step3 = "<<runtime_step3<<endl;
}

	
