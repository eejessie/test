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
#include "function/upd_circuit.h"
#include "function/idtf_unt.h"
#include "podem/podem_v2.h"
#include "function/ATPG_checker.h"


#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

int original_size;
extern vector<int> inputList;
extern ofstream redFILE0, redFILE1, timeRecord;
extern vector<string> masterInputVector;


int cal_cost(map<int, CircuitNode> &tmp_orNodeList, int sa_line, int sa_value)
{
    //print_circuit(tmp_orNodeList, redFILE1);

    int old_size = tmp_orNodeList.size();

    HashTable implicationNodeList;
	implicationNodeList.Init_HashTable();
	red_forward_deletion_v2(tmp_orNodeList, implicationNodeList, sa_line, sa_value);
	implicationNodeList.Destroy_HashTable();
		
	red_backward_deletion(tmp_orNodeList);
    collapse_circuit(tmp_orNodeList);
    
    int new_size = tmp_orNodeList.size();
    int cost = old_size - new_size;

    return cost;
    
}


void rar(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, vector<FaultList> &orFaultList, multimap<int, FaultList> &cost_sf, map<int, int> &truthTable, int &first_cost)
{
    redFILE1 << endl<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
    redFILE1 << "Coming into rar!"<<endl;
    redFILE0 << "Coming into rar!"<<endl;
    redFILE1 << "first_cost = "<<first_cost<<endl;
    
   // print_circuit(masterNodeList, redFILE1);

     if(cost_sf.empty())
        return;

    map<int, int>::iterator itrmi, itrmi1;
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2;
    vector<CircuitNode*>::iterator itrv;
    set<int>::iterator itrs, itrs1;
    multimap<int, FaultList>::reverse_iterator itrmf_r, itrmf_r1;
    multimap<int, FaultList>::iterator itrmf;
    	
	itrmf_r = cost_sf.rbegin(); 
	int sa_line = itrmf_r->second.lineNumber;
	itrm = orNodeList.find(sa_line);
	while(itrm == orNodeList.end())
	{
	    itrmf = cost_sf.end();
	    itrmf--;
	    cost_sf.erase(itrmf);
	    itrmf_r = cost_sf.rbegin(); 
	    sa_line = itrmf_r->second.lineNumber;
	    itrm = orNodeList.find(sa_line);
	}
	    
	int sa_value = itrmf_r->second.stuckAtValue;
	cout << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;	
	redFILE1 << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;
	redFILE0 << endl << "In rar, ("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl; 
    
    int this_cost = itrmf_r->first;
	redFILE1 << "cost of current fault: "<<this_cost<<endl;
	
	
	map<int, int> MA_act, MA_obs, MA_forced;
	int totalInputs = inputList.size();
	
	struct timeb startTime, endTime;
	struct timeb startTime1, endTime1;        	 
	
	//step1. compute the SMA of current fault	
	ftime(&startTime); 
	 
	redFILE1 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	redFILE1 << "step1. Compute the MAs of current fault:"<<endl;
	redFILE0 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	redFILE0 << "step1. Compute the MAs of current fault:"<<endl;
	
	int untestable = compute_MA(masterNodeList, itrmf_r->second, MA_act, MA_obs, MA_forced);	
	ftime(&endTime);
	double runtime = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	timeRecord << endl << "step1: "<<runtime<<endl;
	
	if(untestable == 1)
	{
	    redFILE1 << "*This fault is untestable!"<<endl;
	    redFILE1 << "In opt_rar, direct implication fails "<<endl;
	    HashTable implicationNodeList;
		implicationNodeList.Init_HashTable();
		red_forward_deletion_v2(orNodeList, implicationNodeList, itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
		implicationNodeList.Destroy_HashTable();
		
		red_backward_deletion(orNodeList);
        collapse_circuit(orNodeList);
        redFILE1 << "after remove redundancy, circuit size = "<<orNodeList.size()<<endl; 
       // redFILE1 << "run logic simulation:"<<endl; 
       // RunLogicSimulation(orNodeList, totalInputs, masterInputVector, truthTable);	
              
		merge_circuit(masterNodeList, orNodeList, brNodeList);

		//cost_sf.clear();
		//orFaultList.clear();
		//create_fault_list(orNodeList, orFaultList);
		//compute_cost(orNodeList, orFaultList, cost_sf, first_cost);	

        rar(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, truthTable, first_cost);
        
    }	
	else
	{
	    //redFILE1 << "*This fault is testable!"<<endl;
	    
	    redFILE1 << "MA_act:"<<endl;
	    for(itrmi = MA_act.begin(); itrmi != MA_act.end(); itrmi++)
	    {
	        itrm = masterNodeList.find(itrmi->first);
	        if(itrm->second.circuitIndex == 1)
	            redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";
	    }	        
	    redFILE1 << endl << "MA_obs:"<<endl;    
	    for(itrmi = MA_obs.begin(); itrmi != MA_obs.end(); itrmi++)
	    {
	        itrm = masterNodeList.find(itrmi->first);
	        if(itrm->second.circuitIndex == 1)
	            redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";
	    }	        
	    redFILE1 << endl << "MA_forced:"<<endl;
	    for(itrmi = MA_forced.begin(); itrmi != MA_forced.end(); itrmi++)
	    {
	        itrm = masterNodeList.find(itrmi->first);
	        if(itrm->second.circuitIndex == 1)
	            redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";
	    }
	    redFILE1 << endl;
		
	    itrmf = cost_sf.end();
	    itrmf--;
	    cost_sf.erase(itrmf);
	    
	    
	    //step2. construt the set of candadicate wires
	    ftime(&startTime);
	    
	    redFILE1 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	    redFILE1 << "step2. Construt the set of candadicate wires"<<endl;
	    redFILE0 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	    redFILE0 << "step2. Construt the set of candadicate wires"<<endl;
	    
	    HashTable tranfanout;
        tranfanout.Init_HashTable();
        find_tranfanout_hash(masterNodeList, tranfanout, sa_line);
	    
	    multimap<int, int> cand_set;
	    for(itrmi = MA_act.begin(); itrmi != MA_act.end(); itrmi++)
	    {
	        itrm = masterNodeList.find(itrmi->first);
	        if(itrm->second.gateType == 5)    //ignore the inverters
	            continue;
	        if(itrm->second.circuitIndex == 1)
	        {
	            itrs = itrm->second.lineValue.begin();
	            if(*itrs == 0 || *itrs == 1)
	            {
	                int p;
	                if(tranfanout.Search_Hash(itrm->second.lineNumber, p) == 1)
	                    continue;
                    HashTable tranfanin_ns;
                    tranfanin_ns.Init_HashTable();
                    find_tranfanin_hash(masterNodeList, tranfanin_ns, itrmi->first);
                    HashTable tranfanout_ns;
                    tranfanout_ns.Init_HashTable();
                    find_tranfanout_hash(masterNodeList, tranfanout_ns, itrmi->first);
	                for(itrmi1 = MA_forced.begin(); itrmi1 != MA_forced.end(); itrmi1++)
	                {
	                    itrm1 = masterNodeList.find(itrmi1->first);
	                    int p;
	                    if(tranfanin_ns.Search_Hash(itrmi1->first, p) == 1)
	                        continue;
	                    if(tranfanout_ns.Search_Hash(itrmi1->first, p) == 1)
	                        continue;
	                    if(itrm1->second.circuitIndex == 1 && (itrm1->second.gateType == 3 || itrm1->second.gateType == 7))
	                    {
	                        itrs1 = itrm1->second.lineValue.begin();
	                        int ns, nd;
	                        if(*itrs1 != X && itrm->second.lineNumber != itrm1->second.lineNumber)
	                        {   
	                            ns = itrm->second.lineNumber;
	                            nd = itrm1->second.lineNumber;
	                            if(itrm->second.nodeType == 2) //branch
                        	    {
                        	        itrv = itrm->second.pointFanIn.begin();
                        	        itrm2 = masterNodeList.find((*itrv)->lineNumber);
                        	        if(itrm2->second.nodeType == 1)
                        	            ns = itrm2->second.lineNumber;
                        	    }
                        	    //redFILE1 << "ns = "<<ns<<", nd = "<<nd<<endl;
	                            cand_set.insert(pair<int, int>(ns, nd));
	                        }	                        
	                    }
	                }
	                tranfanin_ns.Destroy_HashTable();
	                tranfanout_ns.Destroy_HashTable();
	                	    
	            }
	        }
	    }	    
	    tranfanout.Destroy_HashTable();
	    redFILE1 << "cand_set: "<<cand_set.size()<<endl;
	    for(itrmi = cand_set.begin(); itrmi != cand_set.end(); itrmi++)
	        redFILE1 << "("<<itrmi->first<<","<<itrmi->second<<")  ";
	    redFILE1<<endl;
	    
	    ftime(&endTime);
	    runtime = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	    timeRecord << "step2: "<<runtime<<endl;
	
    	//step3. check the validality of all candadicate wires
    	ftime(&startTime);    	
    	redFILE1 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    	redFILE1 << "step3. Check the validality of all candadicate wires"<<endl;
    	redFILE0 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    	redFILE0 << "step3. Check the validality of all candadicate wires"<<endl;
    	
    	multimap<int, int>::iterator itrmmi;    	
    	int flag = 0;
    	for(itrmmi = cand_set.begin(); itrmmi != cand_set.end(); itrmmi++)
    	{    	        	
    	    int ns = itrmmi->first;    
    	    int nd = itrmmi->second;    	    
    	    itrmi = MA_act.find(ns);
    	    int ns_value = itrmi->second;    	    
    	    itrmi1 = MA_forced.find(nd);
    	    int nd_value = itrmi1->second;
    	    map<int, int> ns_set, nd_set;
    	    ns_set.insert(pair<int, int>(ns, ns_value));
    	    nd_set.insert(pair<int, int>(nd, nd_value));
    	    
    	    itrm = masterNodeList.end();
    	    itrm--;
    	    int maxLineNumber = itrm->second.lineNumber;
    	    int wa_line = maxLineNumber + 1;
    	    redFILE1 << endl << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;
    	    redFILE1 << "*Current candidate wire is ("<<ns<<", "<<nd<<"), new lineNumber = "<<wa_line<<endl;
    	    redFILE1 << "ns_value = "<<ns_value<<", nd_value = "<<nd_value<<endl;
    	    int wa_value;
    	   
    	    if(ns_value == 1)
    	        wa_value = 0;
    	    else if(ns_value == 0)
    	        wa_value = 1;
    	        
    	    redFILE1 << "wa_value = "<<wa_value<<endl;
    	    
    	    int last_size = orNodeList.size();
    	    map<int, CircuitNode> tmp_orNodeList;
    	    copy_point_vector1(orNodeList, tmp_orNodeList);
    	    upd_circuit(tmp_orNodeList, ns_set, nd_set, maxLineNumber); 
    	    merge_circuit(masterNodeList, tmp_orNodeList, brNodeList);  
    	    
    	    typedef multimap<int, FaultList>::iterator PAT; 
    	    FaultList objFault(wa_line, wa_value);	    
    	    TestList test(wa_line, wa_value);
    	    int outputLevel = SetLineLevel(tmp_orNodeList);
	        vector<int> *levelSet;
	        levelSet = new vector<int>[outputLevel];
	        int level;
            for(itrm = tmp_orNodeList.begin(); itrm != tmp_orNodeList.end(); itrm++)
            {
                level = itrm->second.lineLevel;
                if(level > 0)
                    levelSet[level-1].push_back(itrm->second.lineNumber);
            }
    	    int isTestable_wa = podem(tmp_orNodeList, objFault, test, truthTable, outputLevel, levelSet, inputList);
    	    redFILE1 << "isTestable_wa = "<<isTestable_wa<<endl;
 
       	    flag = 0;
    	    if(!isTestable_wa)
    	    {
    	        redFILE1 << "find alternative wires!"<<endl;
    	        
    	        //Run FILE on wa to find more newly untestable wires
    	        multimap<int, int> ICA;
                ICA.insert(pair<int, int>(wa_line, 0));
            	ICA.insert(pair<int, int>(wa_line, 1));   
            	multimap<int, FaultList> untFaultSet; 	     	    	
    	        idtf_unt(tmp_orNodeList, masterNodeList, ICA, untFaultSet);
    	        map<int, CircuitNode> bak_tmp_orNodeList;
    	        copy_point_vector1(tmp_orNodeList, bak_tmp_orNodeList);
    	        multimap<int, FaultList> newly_unt_faults;
    	        redFILE1 << "current target fault: ("<<sa_line<<","<<sa_value<<"), cost = "<<this_cost<<endl;
    	        
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
    	        }
	        
                redFILE1 << "last size = "<<last_size<<endl;
                redFILE1 << "##Check newly untestable faults!"<<endl;
                for(itrmf_r1 = newly_unt_faults.rbegin(); itrmf_r1 != newly_unt_faults.rend(); itrmf_r1++ )
                {
                    int nsa_line = itrmf_r1->second.lineNumber;
                    int nsa_value = itrmf_r1->second.stuckAtValue;
                    int ncost = itrmf_r1->first;
                    redFILE1 << "current fault: ("<<nsa_line<<","<<nsa_value<<"), cost = "<<ncost<<endl;
                    itrm = tmp_orNodeList.find(nsa_line);
                    if(itrm == tmp_orNodeList.end())
                        continue;
                    
                    int outputLevel = SetLineLevel(tmp_orNodeList);
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
            	    int isTestable = podem(tmp_orNodeList, itrmf_r1->second, test, truthTable, outputLevel, levelSet, inputList);
            	    delete []levelSet;
                    if(!isTestable)
                    {
                        HashTable implicationNodeList;
		                implicationNodeList.Init_HashTable();
		                red_forward_deletion_v2(tmp_orNodeList, implicationNodeList, nsa_line, nsa_value);
		                implicationNodeList.Destroy_HashTable();		
		                red_backward_deletion(tmp_orNodeList);
                        collapse_circuit(tmp_orNodeList);
                        redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;
                    }    
                }
                
                int new_size = tmp_orNodeList.size();
                if(new_size < last_size)
                {
                    redFILE1 << "circuit size is reduced, cost of this fault = "<<this_cost<<endl;
                    if(last_size == original_size)
                        first_cost = this_cost;
                    flag = 1;    	            
                    copy_point_vector1(tmp_orNodeList, orNodeList);
                }   
                merge_circuit(masterNodeList, orNodeList, brNodeList);
        	        
        	    if(flag == 1)
        	         break;               
    	    }
    	    else 
    	        merge_circuit(masterNodeList, orNodeList, brNodeList);
    	    	    
    	 /*   idtf_unt(tmp_orNodeList, masterNodeList, ICA, untFaultSet);
    	    PAT ite = untFaultSet.find(wa_line);
    	    if(ite == untFaultSet.end())
    	    {
    	        merge_circuit(masterNodeList, orNodeList, brNodeList);
    	        continue;
    	    }        	    
    	    pair<PAT, PAT> range = untFaultSet.equal_range(wa_line);    	    
    	    
    	    redFILE1 << "last_size = "<<last_size<<endl;    	    
    	    
    	    int flag = 0;
    	    for(PAT ite = range.first; ite != range.second; ite++)
    	    {    	        
    	         if(ite->second.stuckAtValue == wa_value)
    	         {
    	             redFILE1 << "find alternative wires!"<<endl;
    	             
    	             HashTable implicationNodeList;
		             implicationNodeList.Init_HashTable();
		             red_forward_deletion_v2(tmp_orNodeList, implicationNodeList, sa_line, sa_value);
		             implicationNodeList.Destroy_HashTable();
		
		             red_backward_deletion(tmp_orNodeList);
                     collapse_circuit(tmp_orNodeList);
                     redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;
                     int new_size = tmp_orNodeList.size();
                     if(new_size <= last_size)
                     {
                        flag = 1;    	            
                        copy_point_vector1(tmp_orNodeList, orNodeList);
                     }                                         		                  
    	         }
        	     merge_circuit(masterNodeList, orNodeList, brNodeList);
        	        
        	     if(flag == 1)
        	         break;
    	     }    	     
    	     if(flag == 1)
    	        break;
    	     else 
    	        merge_circuit(masterNodeList, orNodeList, brNodeList);*/
    	}
    	
    	if(flag == 1)
    	{
    	   // cost_sf.clear();
    	   // orFaultList.clear();
		   // create_fault_list(orNodeList, orFaultList);
		   // compute_cost(orNodeList, orFaultList, cost_sf, first_cost);	
		}
		/*redFILE1 << "cost_sf: "<<cost_sf.size()<<endl;
		for(itrmf = cost_sf.begin(); itrmf != cost_sf.end(); itrmf++)
		    redFILE1 << "("<<itrmf->second.lineNumber<<","<<itrmf->second.stuckAtValue<<")  ";
		redFILE1 << endl;*/
		ftime(&endTime);
		runtime = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	    timeRecord << "step3: "<<runtime<<endl;

       // print_circuit(orNodeList, redFILE1);
        rar(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, truthTable, first_cost);
	
	}
	
}

void opt_rar(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &orFaultList, map<int, int> &truthTable)
{
    multimap<int, FaultList> cost_sf;
    
    int first_cost = 100000;
	compute_cost(orNodeList, orFaultList, cost_sf, first_cost);
	
	original_size = orNodeList.size();
		
	rar(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, truthTable, first_cost);
}

	
