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
#include "function/compute_MA.h"
#include "function/compute_MA_or.h"
#include "function/upd_circuit.h"
#include "function/upd_circuit_plus.h"
#include "function/idtf_unt.h"
#include "function/parallel_test_simulation_bdd.h"


#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

int original_size;
extern vector<int> inputList;
extern ofstream redFILE0, redFILE1, timeRecord, oufile;
extern vector<string> masterInputVector;
static double runtime_step1 = 0, runtime_step2 = 0, runtime_fs = 0, runtime_podem = 0, runtime_step3 = 0, runtime_loop = 0;


void rar_sat(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, vector<FaultList> &orFaultList, multimap<int, FaultList> &cost_sf, int &first_cost, vector<string> &br_bench_log)
{
    redFILE1 << endl<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
    redFILE1 << "Coming into rar!"<<endl;
    redFILE0 << "Coming into rar!"<<endl;
    redFILE1 << "first_cost = "<<first_cost<<endl;
    
     if(cost_sf.empty())
        return;
        
    map<int, int>::iterator itrmi, itrmi1, itrmi2;
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3;
    vector<CircuitNode*>::iterator itrv, itrv0, itrv1;
    set<int>::iterator itrs, itrs1;
    multimap<int, FaultList>::reverse_iterator itrmf_r;
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
//	print_circuit(orNodeList, redFILE1);
	
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
   //     redFILE1 << "0. run logic simualtion"<<endl;
   //     run_logic_simulation(orNodeList, totalInputs, masterInputVector, dd, dnode); 
		merge_circuit(masterNodeList, orNodeList, brNodeList);
        rar_sat(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, first_cost, br_bench_log);
        
    }	
	else
	{
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
	     
	    HashTable tranfanout;
        tranfanout.Init_HashTable();
        for(itrm1 = orNodeList.begin(); itrm1 != orNodeList.end(); itrm1++)
	        itrm1->second.visit = 0;		
        find_tranfanout_hash(orNodeList, tranfanout, sa_line);	    
	    
	    itrm = orNodeList.find(sa_line);
	    int flag_branch;
	    if(itrm->second.nodeType == 2) //branch
	        flag_branch = 1;
	    else 
	        flag_branch = 0;
	    multimap<int, int> cand_set;
	    for(itrmi = MA_act.begin(); itrmi != MA_act.end(); itrmi++)
	    {
	        int ns, nd;  
	        if(itrmi->second != 0 && itrmi->second != 1)
	            continue;
	        itrm1 = masterNodeList.find(itrmi->first);	        
	        if(itrm1->second.circuitIndex != 1)
	            continue;
	        if(itrm1->second.gateType == 5)    //ignore the inverters
	            continue;
	        if(itrm1->second.gateType == 9 || itrm1->second.gateType == 10 || itrm1->second.gateType == 11)
	            continue;
	        ns = itrmi->first;
	        if(itrm1->second.nodeType == 2)
	        {
	            itrv = itrm1->second.pointFanIn.begin();
	            itrm2 = masterNodeList.find((*itrv)->lineNumber);
	            if(itrm2->second.nodeType == 1)  //PI
	                ns = itrm2->second.lineNumber;
	        }
	            	       
	        int p;
	        if(tranfanout.Search_Hash(ns, p) == 1)
	            continue;
	                      
            HashTable tranfanin_ns, tranfanout_ns;
            tranfanin_ns.Init_HashTable();
            tranfanout_ns.Init_HashTable();  
            map<int, CircuitNode>::iterator itrm_tmp;
            for(itrm_tmp = orNodeList.begin(); itrm_tmp != orNodeList.end(); itrm_tmp++)
	            itrm_tmp->second.visit = 0;	
            find_tranfanin_hash(orNodeList, tranfanin_ns, ns);                
        /*    for(itrm_tmp = orNodeList.begin(); itrm_tmp != orNodeList.end(); itrm_tmp++)
	            itrm_tmp->second.visit = 0;	
            find_tranfanout_hash(orNodeList, tranfanout_ns, ns);  */             
	        for(itrmi1 = MA_forced.begin(); itrmi1 != MA_forced.end(); itrmi1++)
	        {
	            itrm2 = masterNodeList.find(itrmi1->first);
	            if(itrm2->second.circuitIndex != 1)
	                continue;
	            if(itrm2->second.gateType == 5 || itrm2->second.gateType == 9 || itrm2->second.gateType == 10 || itrm2->second.gateType == 11)
	                continue;          
	            nd = itrmi1->first;	
	            if(itrm2->second.nodeType == 2)
	            {
	                itrv = itrm2->second.pointFanIn.begin();
	                itrm3 = masterNodeList.find((*itrv)->lineNumber);
	                if(itrm3->second.nodeType == 1)  //PI
	                    nd = itrm3->second.lineNumber;
	            } 	            
	            if(nd == ns)
	                continue;
	            if(flag_branch == 1 && nd == sa_line)
	                continue;	 
	            int p1;
	            if(tranfanin_ns.Search_Hash(nd, p1) == 1)
	                continue;
	         //   if(tranfanout_ns.Search_Hash(nd, p1) == 1)
	         //       continue;                                                  
	            cand_set.insert(pair<int, int>(ns, nd));                  
	        }
	        tranfanin_ns.Destroy_HashTable();
	        tranfanout_ns.Destroy_HashTable();	    
	    }	    
	    tranfanout.Destroy_HashTable();
	    
	    redFILE1 << "size of cand_set:"<<cand_set.size();
	    multimap<int, int> cand_delete;
	    for(itrmi1 = cand_set.begin(); itrmi1 != cand_set.end(); itrmi1++)
	    {
	        for(itrmi2 = cand_set.begin(); itrmi2 != cand_set.end(); itrmi2++)
	        {
	            if(itrmi1->first == itrmi2->first && itrmi1->second != itrmi2->second)
	            {
	                itrm1 = orNodeList.find(itrmi1->second);
	                itrm2 = orNodeList.find(itrmi2->second);
	                int gt1 = itrm1->second.gateType;
	                int gt2 = itrm2->second.gateType;
	                if(gt1 != 3 && gt1 != 7 && gt2 != 3 && gt2 != 7)
	                    continue;
	                //n2 is fanin of n1: delete n1    
	                if((gt1 == 3 || gt1 == 7) && (gt2 != 3 && gt2 != 7))
	                {
	                    itrv0 = itrm1->second.pointFanIn.begin();
	                    itrv1 = itrm1->second.pointFanIn.end();
	                    itrv1--;
	                    int fanin0 = (*itrv0)->lineNumber;
	                    int fanin1 = (*itrv1)->lineNumber;
	                    if(fanin0 == itrmi2->second || fanin1 == itrmi2->second)
	                        cand_delete.insert(pair<int, int>(itrmi1->first, itrmi1->second));
	                }
	                //n1 is fanin of n2: delete n2
	                else if((gt2 == 3 || gt2 == 7) && (gt1 != 3 && gt1 != 7))
	                {
	                    itrv0 = itrm2->second.pointFanIn.begin();
	                    itrv1 = itrm2->second.pointFanIn.end();
	                    itrv1--;
	                    int fanin0 = (*itrv0)->lineNumber;
	                    int fanin1 = (*itrv1)->lineNumber;
	                    if(fanin0 == itrmi1->second || fanin1 == itrmi1->second)
	                        cand_delete.insert(pair<int, int>(itrmi2->first, itrmi2->second));
	                }
	                else
	                {
	                    //n2 is fanin of n1: delete n1
	                    itrv0 = itrm1->second.pointFanIn.begin();
	                    itrv1 = itrm1->second.pointFanIn.end();
	                    itrv1--;
	                    int fanin0 = (*itrv0)->lineNumber;
	                    int fanin1 = (*itrv1)->lineNumber;
	                    if(fanin0 == itrmi2->second || fanin1 == itrmi2->second)
	                        cand_delete.insert(pair<int, int>(itrmi1->first, itrmi1->second));
	                    else
	                    {
	                        //n1 is fanin of n2: delete n2
	                        itrv0 = itrm2->second.pointFanIn.begin();
	                        itrv1 = itrm2->second.pointFanIn.end();
	                        itrv1--;
	                        fanin0 = (*itrv0)->lineNumber;
	                        fanin1 = (*itrv1)->lineNumber;
	                        if(fanin0 == itrmi1->second || fanin1 == itrmi1->second)
	                            cand_delete.insert(pair<int, int>(itrmi2->first, itrmi2->second));
	                    }
	                }
	                
	            }
	        }
	    }
	    typedef multimap<int, int>::iterator PAT;
	    multimap<int, int> cand_set_new;
	    for(itrmi = cand_set.begin(); itrmi != cand_set.end(); itrmi++)
	    {
	        int flag_ignore = 0;
	        pair<PAT, PAT> ret = cand_delete.equal_range(itrmi->first);
	        for(PAT ite = ret.first; ite != ret.second; ite++)
	        {
	            if(ite->second == itrmi->second)
	            {
	                flag_ignore = 1;
	                break;
	            }
	        }
	        if(flag_ignore == 0)
	            cand_set_new.insert(pair<int, int>(itrmi->first, itrmi->second));
        }
        cand_set.clear();
        cand_set = cand_set_new;
        redFILE1 << "size of cand_set:"<<cand_set.size();
	    
	    ftime(&endTime);
	    double runtime_s2 = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	    redFILE1 << "runtime_s2 = "<<runtime_s2<<endl;
	    runtime_step2 += runtime_s2;
	
    	//step3. check the validality of all candadicate wires    	
    	redFILE1 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    	redFILE1 << "step3. Check the validality of all candadicate wires"<<endl;
    	redFILE0 << endl << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    	redFILE0 << "step3. Check the validality of all candadicate wires"<<endl;

    	itrm = brNodeList.end();
    	itrm--;
    	int maxLineNumber_br = itrm->second.lineNumber;
    	
    	multimap<int, int>::iterator itrmmi;    	
    	int flag_reduce = 0;
    	ftime(&startTime);    	
    	for(itrmmi = cand_set.begin(); itrmmi != cand_set.end(); itrmmi++)
    	{    	   
    	    //step1. Obtain ns_set & nd_set &maxLineNumber & wa_value
    	    int ns = itrmmi->first;    
    	    int nd = itrmmi->second;     
    	    itrmi = MA_act.find(ns);
    	    int ns_value = itrmi->second;    	    
    	    itrmi1 = MA_forced.find(nd);
    	    int nd_value = itrmi1->second;
    	    map<int, int> ns_set, nd_set;
    	    ns_set.insert(pair<int, int>(ns, ns_value));
    	    nd_set.insert(pair<int, int>(nd, nd_value));
    	    redFILE1 << endl << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;
    	    redFILE1 << "ns = "<<ns<<", nd = "<<nd<<endl;
    	    
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
    	    map<int, CircuitNode> tmp_orNodeList, tmp_masterNodeList;
    	    copy_point_vector1(orNodeList, tmp_orNodeList);  
    	    int wa_line = upd_circuit(tmp_orNodeList, ns_set, nd_set, maxLineNumber);
            merge_circuit(tmp_masterNodeList, tmp_orNodeList, brNodeList);
    	    maxLineNumber += 4;    
    	    redFILE1 << "*Current candidate wire is ("<<ns<<", "<<nd<<"), new lineNumber = "<<wa_line<<endl;
    	    redFILE1 << "ns_value = "<<ns_value<<", nd_value = "<<nd_value<<endl;
    	    int wa_value;    	   
    	    if(nd_value == 0 || nd_value == D)
    	        wa_value = 0;
    	    else if(nd_value == 1 || nd_value == B)
    	        wa_value = 1;    	        
    	    redFILE1 << "wa_value = "<<wa_value<<endl;
 
    	    //step3. Check whether the candidate wire wa_line is untestable by sat
    	    ftime(&startTime2);  
    	    FaultList objFault(wa_line, wa_value);	
    	    map<int, CircuitNode> sim_tmp_orNodeList;
            copy_point_vector1(tmp_orNodeList, sim_tmp_orNodeList);
            simplify_circuit(sim_tmp_orNodeList, sa_line, sa_value);   
            int isTestable_wa_sat = check_red_sat_abc(sim_tmp_orNodeList, brNodeList, br_bench_log, oufile); 
            ftime(&endTime2);
            double runtime_sat = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
    	    redFILE1 << "runtime_sat = "<<runtime_sat<<endl;
    	    
    	    redFILE1 << "isTestable_wa_sat = "<<isTestable_wa_sat<<endl;
    	    
    	    //step4. If wa_line is untestable, do removal. Other wise, try adding another gate to make wa_line untestable.
    	    if(isTestable_wa_sat == 0)      //untestable	        
            {             	        
        	    redFILE1 << "find alternative wires!"<<endl; 
        	    redFILE1 << "current target fault: ("<<sa_line<<","<<sa_value<<"), cost = "<<this_cost<<endl;     	        
        	    //Run FILE on wa to find more newly untestable wires
        	    multimap<int, int> ICA;
                ICA.insert(pair<int, int>(wa_line, 0));
                ICA.insert(pair<int, int>(wa_line, 1)); 
                redFILE1 << "last size = "<<last_size<<endl;    
                rem_new_unt(tmp_orNodeList, brNodeList, tmp_masterNodeList, ICA, wa_line, sa_line, sa_value,  this_cost, br_bench_log, oufile);  
                
                int new_size = tmp_orNodeList.size();
                redFILE1 << "new size = "<<new_size<<endl;   
                if(new_size < last_size)
                {
                    redFILE1 << "circuit size is reduced, cost of this fault = "<<this_cost<<endl;
                    if(last_size == original_size)
                        first_cost = this_cost;
                    flag_reduce = 1;    	            
                    copy_point_vector1(tmp_orNodeList, orNodeList);
                    merge_circuit(masterNodeList, orNodeList, brNodeList);
                }                                  
            	if(flag_reduce == 1)
            	    break;       
        	}//if(isTestable_wa_sat == 0)  
        	 
            else if(isTestable_wa_sat == 1 && this_cost > 6)
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
                        redFILE1 << "last_size = "<<last_size;
                        int new_size = tmp_orNodeList.size();
                        redFILE1 << "new_size = "<<new_size<<endl;
                        if(new_size < last_size)
                        {
                            redFILE1 << "circuit size is reduced, cost of this fault = "<<this_cost<<endl;                          
                            if(last_size == original_size)
                                first_cost = this_cost;   	            
                            copy_point_vector1(tmp_orNodeList, orNodeList);
                            merge_circuit(masterNodeList, orNodeList, brNodeList);
                            flag_reduce = 1;
                            break;
                        }  
                        else
                            copy_point_vector1(bak_tmp_orNodeList, tmp_orNodeList);                          
                    }//for(itrmi = ndp_set.begin(); itrmi != ndp_set.end(); itrmi++)
                }//else if(!ndp_set.empty())  
                if(flag_reduce == 1)
                    break;                
            }//else if(isTestable_wa_sat == 1 && this_cost > 6)
        }//for(cand_set) loop
    	
	    ftime(&endTime);
	    double runtime_s3 = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	    redFILE1 << "runtime_s3: "<<runtime_s3<<endl;
	    runtime_step3 += runtime_s3;
         
         rar_sat(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, first_cost, br_bench_log);
         	
	}	
}

void opt_rar_sat(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<FaultList> &orFaultList, vector<string> &br_bench_log)
{
    multimap<int, FaultList> cost_sf;
    
    int first_cost = 100000;
    redFILE1 << "compute cost:"<<endl;
	compute_cost(orNodeList, orFaultList, cost_sf, first_cost);
	
	original_size = orNodeList.size();
			
	rar_sat(masterNodeList, orNodeList, brNodeList, orFaultList, cost_sf, first_cost, br_bench_log);
	
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

	
