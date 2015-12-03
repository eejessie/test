//Created by Yi Wu on 26 Oct, 2014

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
#include <utility>

#include "class/CircuitNode.h"         
#include "class/CircuitLine.h"         
#include "class/FaultList.h"          
#include "class/TestList.h"          
#include "class/ConnFault.h"
#include "class/HashTable.h"
#include "lib/file_operations.h"
#include "lib/forward_implication.h"
#include "lib/radix_convert.h"
#include "function/atpg.h"
#include "function/copy_point_vector.h"
#include "function/helper.h"
#include "function/print_circuit.h"
#include "function/implicate_new.h"

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

void recursive_learning_new(map<int, CircuitNode> &masterNodeList, FaultList wt_error, int current_node, int r, int rmax, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced, int &consist_flag, int flag_ao);

extern ofstream redFILE0, redFILE1;

map<int, int> find_common_assign_one(map<int, int> &first_act, map<int, int> &first_obs, map<int, int> &first_forced, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced)
{
	map<int, int>::iterator itrmi, itrmi1, itrmi2, itrmi3;
	typedef map<int, map<int, int> >::iterator PAT;
	PAT ite;
	
	map<int, int> learned_assign;
	map<int, int> this_time_MA;
//	redFILE1 << "Coming into find_common_assign_one:"<<endl;
		
	for(itrmi1 = first_act.begin(); itrmi1 != first_act.end(); itrmi1++)
	{
		int nec_line = itrmi1->first;
		int nec_value = itrmi1->second;
	    itrmi = MA_act.find(nec_line);
		if(itrmi == MA_act.end())
		{
		    MA_act.insert(pair<int, int>(nec_line, nec_value));	
			redFILE0 << "Node "<<nec_line<<" is pushed into MA with value "<<nec_value<<endl;	
		}
		else
		{
		    if(nec_value != itrmi->second)
		    {
		        cerr << "error in find_common_assign_one."<<endl;
		        exit(1);
		    }
		}				
		this_time_MA.insert(pair<int, int>(nec_line, nec_value));	
	}
	
	for(itrmi1 = first_obs.begin(); itrmi1 != first_obs.end(); itrmi1++)
	{
		int nec_line = itrmi1->first;
		int nec_value = itrmi1->second;
	    itrmi = MA_obs.find(nec_line);
		if(itrmi == MA_obs.end())
		{
		    MA_obs.insert(pair<int, int>(nec_line, nec_value));	
			redFILE0 << "Node "<<nec_line<<" is pushed into MA with value "<<nec_value<<endl;	
		}
		else
		{
		    if(nec_value != itrmi->second)
		    {
		        cerr << "error in find_common_assign_one."<<endl;
		        exit(1);
		    }
		}				
		this_time_MA.insert(pair<int, int>(nec_line, nec_value));	
	}

	return this_time_MA;
}

map<int, int> find_common_assign_two(map<int, int> &first_act, map<int, int> &first_obs, map<int, int> &first_forced, map<int, int> &second_act, map<int, int> &second_obs, map<int, int> &second_forced, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced)
{
	map<int, int>::iterator itrmi, itrmi1, itrmi2, itrmi3;
	typedef map<int, map<int, int> >::iterator PAT;
	PAT ite;
	
	map<int, int> learned_assign;
	map<int, int> this_time_MA;
	redFILE0 << "Coming into find_common_assign_two:"<<endl;
	
	for(itrmi1 = first_act.begin(); itrmi1 != first_act.end(); itrmi1++)
	{
		itrmi2 = second_act.find(itrmi1->first);
		if(itrmi2 != second_act.end())
		{
			if(itrmi1->second == itrmi2->second)
			{
				int nec_line = itrmi2->first;
				int nec_value = itrmi2->second;
                itrmi = MA_act.find(nec_line);
				if(itrmi == MA_act.end())
				{
				    MA_act.insert(pair<int, int>(nec_line, nec_value));	
					redFILE0 << "Node "<<nec_line<<" is pushed into MA with value "<<nec_value<<endl;	
				}
				else
		        {
		            if(nec_value != itrmi->second)
		            {
		                cerr << "error in find_common_assign_one."<<endl;
		                exit(1);
		            }
		        }		
				this_time_MA.insert(pair<int, int>(nec_line, nec_value));		
			}		
		}
	}
	
	for(itrmi1 = first_obs.begin(); itrmi1 != first_obs.end(); itrmi1++)
	{
		itrmi2 = second_obs.find(itrmi1->first);
		if(itrmi2 != second_obs.end())
		{
			if(itrmi1->second == itrmi2->second)
			{
				int nec_line = itrmi2->first;
				int nec_value = itrmi2->second;
                itrmi = MA_obs.find(nec_line);
				if(itrmi == MA_obs.end())
				{
				    MA_obs.insert(pair<int, int>(nec_line, nec_value));	
					redFILE0 << "Node "<<nec_line<<" is pushed into MA with value "<<nec_value<<endl;	
				}
				else
		        {
		            if(nec_value != itrmi->second)
		            {
		                cerr << "error in find_common_assign_one."<<endl;
		                exit(1);
		            }
		        }		
				this_time_MA.insert(pair<int, int>(nec_line, nec_value));		
			}		
		}
	}
	
	return this_time_MA;
}


map<int, int> justify_new(map<int, CircuitNode> &masterNodeList, FaultList wt_error, vector<int> input_values, int node, int just_value, int gateType, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced, int r, int rmax, int &consist_flag, int flag_ao)
{
    //iterators
	map<int, CircuitNode>::iterator itrm, itrm1, itrm_in0, itrm_in1;
	vector<CircuitNode*>::iterator itrv0, itrv1;
	set<int>::iterator itrs;
	map<int, int>::iterator itrmi, itrmi1, itrmi2, itrmi3;

	map<int, int> record_first_act, record_first_obs, record_first_forced;
	map<int, int> record_second_act, record_second_obs, record_second_forced;
	int consist_flag_first = -1, consist_flag_second = -1;
	
	itrm = masterNodeList.find(node);
	itrv0 = itrm->second.pointFanIn.begin(); //first input
	itrm_in0 = masterNodeList.find((*itrv0)->lineNumber);
	itrv1 = itrm->second.pointFanIn.end();		
	itrv1--;                                 //second input
	itrm_in1 = masterNodeList.find((*itrv1)->lineNumber);
	int first_value = input_values[0];
	int second_value = input_values[1];
//	redFILE0 << "just_value = "<<just_value<<endl;
//	redFILE0 << "first_value = "<<first_value<<", second_value = "<<second_value<<endl;

	redFILE0 << "starting justifying node "<<node<<endl;
	if(gateType == 7)
	{
		redFILE0 << "Gate type: AND gate."<<endl;		
		if(just_value == 0)
		{
            //(X, X, 0) : j2
			if(first_value == X && second_value == X)
			{
				redFILE0 << endl << "Justification 1: set first input "<<(*itrv0)->lineNumber<<"to 0."<<endl;
				//print_circuit(masterNodeList, redFILE0);
				record_first_act.insert(pair<int, int>((*itrv0)->lineNumber, 0));
				itrm_in0->second.lineValue.clear(); 
				itrm_in0->second.lineValue.insert(0);

				recursive_learning_new(masterNodeList, wt_error, (*itrv0)->lineNumber, r+1, rmax, record_first_act, record_first_obs, record_first_forced, consist_flag_first, flag_ao);
				
				//Clear the values set to  lines during justification 1
				for(itrmi1 = record_first_act.begin(); itrmi1 != record_first_act.end(); itrmi1++)
				{
					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				//	redFILE0 << "node "<<itrmi1->first<<"'s value is cleared."<<endl;
				}
				for(itrmi1 = record_first_obs.begin(); itrmi1 != record_first_obs.end(); itrmi1++)
				{
					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				//	redFILE0 << "node "<<itrmi1->first<<"'s value is cleared."<<endl;
				}
				for(itrmi1 = record_first_forced.begin(); itrmi1 != record_first_forced.end(); itrmi1++)
				{
					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				//	redFILE0 << "node "<<itrmi1->first<<"'s value is cleared."<<endl;
				}
	
				redFILE0 << endl <<"Justification 2: set second input "<<(*itrv1)->lineNumber<<"to 0."<<endl;
				//print_circuit(masterNodeList, redFILE0);
				record_second_act.insert(pair<int, int>((*itrv1)->lineNumber, 0));
				itrm_in1->second.lineValue.clear(); 
				itrm_in1->second.lineValue.insert(0);
					
			    redFILE0 << "before, consist_flag_second = "<<consist_flag_second<<endl;
				recursive_learning_new(masterNodeList, wt_error, (*itrv1)->lineNumber, r+1, rmax, record_second_act, record_second_obs, record_second_forced, consist_flag_second, flag_ao);	
				redFILE0 << "after, consist_flag_second = "<<consist_flag_second<<endl;
				
				//Clear the values set to  lines during justification 2
				for(itrmi2 = record_second_act.begin(); itrmi2 != record_second_act.end(); itrmi2++)
				{
					itrm = masterNodeList.find(itrmi2->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				//	redFILE0 << "node "<<itrmi2->first<<"'s value is cleared."<<endl;
				}		
				for(itrmi2 = record_second_obs.begin(); itrmi2 != record_second_obs.end(); itrmi2++)
				{
					itrm = masterNodeList.find(itrmi2->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				//	redFILE0 << "node "<<itrmi2->first<<"'s value is cleared."<<endl;
				}	
				for(itrmi2 = record_second_forced.begin(); itrmi2 != record_second_forced.end(); itrmi2++)
				{
					itrm = masterNodeList.find(itrmi2->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				//	redFILE0 << "node "<<itrmi2->first<<"'s value is cleared."<<endl;
				}			
			}//if(first_value == X && second_value == X)
		}//if(just_value == 0)
	}//if(gateType == 7)
	
	
	else if(gateType == 3)		
	{		
		if(just_value == 0)
		{			
			//(X, X, 1)
			if(first_value == X && second_value == X)
			{

			    redFILE0 << endl <<"Justification 1: set first input "<<(*itrv0)->lineNumber<<"to 1."<<endl;
				record_first_act.insert(pair<int, int>((*itrv0)->lineNumber, 1));
				itrm_in0->second.lineValue.clear(); 
				itrm_in0->second.lineValue.insert(1);
				
				recursive_learning_new(masterNodeList, wt_error, (*itrv0)->lineNumber, r+1, rmax, record_first_act, record_first_obs, record_first_forced, consist_flag_first, flag_ao);
				
				//Clear the values set to  lines during justification 1
				for(itrmi1 = record_first_act.begin(); itrmi1 != record_first_act.end(); itrmi1++)
				{
					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				}
				for(itrmi1 = record_first_obs.begin(); itrmi1 != record_first_obs.end(); itrmi1++)
				{
					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				}
				for(itrmi1 = record_first_forced.begin(); itrmi1 != record_first_forced.end(); itrmi1++)
				{
					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				}
						
				redFILE0 << endl <<"Justification 2: set second input "<<(*itrv1)->lineNumber<<"to 1."<<endl;
				record_second_act.insert(pair<int, int>((*itrv1)->lineNumber, 1));
				itrm_in1->second.lineValue.clear(); 
				itrm_in1->second.lineValue.insert(1);
				
				recursive_learning_new(masterNodeList, wt_error, (*itrv1)->lineNumber, r+1, rmax, record_second_act, record_second_obs, record_second_forced, consist_flag_second, flag_ao);	
				
				//Clear the values set to  lines during justification 2
				for(itrmi2 = record_second_act.begin(); itrmi2 != record_second_act.end(); itrmi2++)
				{
					itrm = masterNodeList.find(itrmi2->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);			
				}
				for(itrmi2 = record_second_obs.begin(); itrmi2 != record_second_obs.end(); itrmi2++)
				{
					itrm = masterNodeList.find(itrmi2->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);			
				}
				for(itrmi2 = record_second_forced.begin(); itrmi2 != record_second_forced.end(); itrmi2++)
				{
					itrm = masterNodeList.find(itrmi2->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);			
				}
			}//if(first_value == X && second_value == X											
		}//if(just_value == 1)	
	}//else if(gateType == 3)	
						
			
	redFILE0 << endl << "After justifications: "<<endl;
	map<int, int> learned_assign;
	map<int, int> this_time_MA;	
	redFILE0 << "consist_flag_first = "<<consist_flag_first<<endl;
	redFILE0 << "consist_flag_second = "<<consist_flag_second<<endl;
												
	if(consist_flag_first == 1 && consist_flag_second == 1)
	{	
	    /*redFILE0 << "record_values_first:"<<endl;
		for(itrmi = record_values_first.begin(); itrmi != record_values_first.end(); itrmi++)
		    redFILE0 << itrmi->first<<", "<<itrmi->second<<endl;
		redFILE0 << "record_values_second:"<<endl;
		for(itrmi = record_values_second.begin(); itrmi != record_values_second.end(); itrmi++)
		    redFILE0 << itrmi->first<<", "<<itrmi->second<<endl;*/
		    
		this_time_MA = find_common_assign_two(record_first_act, record_first_obs, record_first_forced, record_second_act, record_second_obs, record_second_forced, MA_act, MA_obs, MA_forced);
	}
	else if(consist_flag_first + consist_flag_second == 1)
	{
		if(consist_flag_first == 0)
		{
			record_first_act.clear();
			record_first_obs.clear();
			record_first_forced.clear();
			this_time_MA = find_common_assign_one(record_second_act, record_second_obs, record_second_forced, MA_act, MA_obs, MA_forced);
		}
		else if(consist_flag_second == 0)
		{
			record_second_act.clear();
			record_second_obs.clear();
			record_second_forced.clear();
			this_time_MA = find_common_assign_one(record_first_act, record_first_obs, record_first_forced, MA_act, MA_obs, MA_forced);
		}
	}
	else if(consist_flag_first + consist_flag_second == 0)
	{
		record_first_act.clear();
		record_first_obs.clear();
		record_first_forced.clear();
		record_second_act.clear();
		record_second_obs.clear();
		record_second_forced.clear();
		consist_flag = 0;
	}
	
	return this_time_MA;

}


//recursive_learning
void recursive_learning_new(map<int, CircuitNode> &masterNodeList, FaultList wt_error, int current_node, int r, int rmax, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced, int &consist_flag, int flag_ao)
{
	redFILE0 << endl << "recursive learning for current node "<<current_node<<endl;
	
	//containers
	map<int, CircuitNode>::iterator itrm, itrm1;
	map<int, map<int, int> >::iterator itrmm;
	map<int, int>::iterator itrmi;
	set<int>::iterator itrs, itrs1, itrs2;
	vector<CircuitNode*>::iterator itrv0, itrv1;
	typedef map<int, map<int, int> >::iterator PAT;
	
	vector<int> remaining_unjust_gates;
	remaining_unjust_gates.clear();
	redFILE0 << endl << "$$$First step: direct implications: current_node = "<<current_node<<endl;
	consist_flag = path_propagation_new(masterNodeList, wt_error, current_node,  remaining_unjust_gates, MA_act, MA_obs, MA_forced, flag_ao);
	
	if(consist_flag == 0)   //inconsistent
	{
	    cerr << "In recursive learning, direct implication fails"<<endl;
	    redFILE0 << "In recursive learning, direct implication fails"<<endl;
	    return;
	}		
	else
	{			
		if(r < rmax && !remaining_unjust_gates.empty())
		{
			redFILE0 << endl << "%%%Second step: learning stage!"<<endl;
			for(int i = 0; i < remaining_unjust_gates.size(); i++)
			{
				//redFILE1 << endl << "Current learning gate: "<<remaining_unjust_gates[i]<<endl;
				int node = remaining_unjust_gates[i];
				itrm = masterNodeList.find(remaining_unjust_gates[i]);
				itrs = itrm->second.lineValue.begin();
				int just_value = *itrs;
				itrv0 = itrm->second.pointFanIn.begin();
				itrv1 = itrm->second.pointFanIn.end();
				itrv1--;
				vector<int> input_values;
				itrs1 = (*itrv0)->lineValue.begin();
				itrs2 = (*itrv1)->lineValue.begin();
				input_values.push_back(*itrs1);
				input_values.push_back(*itrs2);
				
				map<int, int> this_time_MA;
				if(itrm->second.gateType == 7)                   //AND gate
				    this_time_MA = justify_new(masterNodeList, wt_error, input_values, node, 0, 7, MA_act, MA_obs, MA_forced, r, rmax, consist_flag, flag_ao);
				else if(itrm->second.gateType == 3)              //OR gate
				    this_time_MA = justify_new(masterNodeList, wt_error, input_values, node, 1, 3, MA_act, MA_obs, MA_forced, r, rmax, consist_flag, flag_ao);
															
			/*	redFILE0 << "this_time_MA:"<<endl;
				for(itrmi = this_time_MA.begin(); itrmi != this_time_MA.end(); itrmi++)
				{
					itrm = masterNodeList.find(itrmi->first);
					if(itrm != masterNodeList.end())
					{
						itrm->second.lineValue.clear();
						itrm->second.lineValue.insert(itrmi->second);
					}
					else 
						cout << "In recursive learning, error!"<<endl;
				}*/					
			}//for()					
		}//if(r < rmax && !remaining_unjust_gates.empty())
	
	}	

	return;
				
}//recursive_learning




