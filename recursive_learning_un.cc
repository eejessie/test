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
#include "function/implicate_un.h"
#include "function/idtf_unt.h"

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

void recursive_learning_un(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &orNodeList, int current_node, int r, int rmax, map<int, int> &MA_dir, multimap<int, FaultList> &untSet, int &consist_flag);

extern ofstream redFILE0, redFILE1;


void deter_fault(map<int, CircuitNode> &orNodeList, int nec_line, int nec_value, multimap<int, FaultList> &tempSet)
{
    map<int, CircuitNode>::iterator itrm, itrm1;
    vector<CircuitNode*>::iterator itrv, itrv1;
    //redFILE1 << "In deter_fault:"<<endl;

    itrm = orNodeList.find(nec_line);
		    if(itrm != orNodeList.end())
            {
                FaultList this_fault(nec_line, 1-nec_value);
                tempSet.insert(pair<int, FaultList>(nec_line, this_fault));
                //redFILE1 << "("<<nec_line<<", "<<1-nec_value<<") is determined"<<endl;
                if(nec_value == 1)
                {
                    if(itrm->second.pointFanOut.size() == 1)
                    {
                        itrv = itrm->second.pointFanOut.begin();
                        if((*itrv)->gateType == 7)           //The fanout gate is AND gate.
                        {
                            itrm1 = orNodeList.find((*itrv)->lineNumber);
                            for(itrv1 = itrm1->second.pointFanIn.begin(); itrv1 != itrm1->second.pointFanIn.end(); itrv1++)
                            {
                                if((*itrv1)->lineNumber != nec_line && (*itrv1)->circuitIndex == 1|| (*itrv1)->circuitIndex == 4)
                                {   
                                    this_fault.lineNumber = (*itrv1)->lineNumber;
                                    this_fault.stuckAtValue = 0;
                                    tempSet.insert(pair<int, FaultList>((*itrv1)->lineNumber, this_fault));
                                   // redFILE1 << "("<<(*itrv1)->lineNumber<<", "<<0<<") is determined"<<endl;
                                    this_fault.stuckAtValue = 1;
                                    tempSet.insert(pair<int, FaultList>((*itrv1)->lineNumber, this_fault));
                                   // redFILE1 << "("<<(*itrv1)->lineNumber<<", "<<1<<") is determined"<<endl;
                                }                                                                    
                            }
                        }
                    }
                }
            }
}

map<int, int> find_common_fault_one(map<int, CircuitNode> &orNodeList, map<int, int> &first, multimap<int, FaultList> &untSet)
{
	map<int, int>::iterator itrmi, itrmi1, itrmi2, itrmi3;

	map<int, int> learned_assign;
	map<int, int> this_time_MA;
	//redFILE1 << "Coming into find_common_fault_one:"<<endl;
	//for(itrmi = first.begin(); itrmi != first.end(); itrmi++)
	//    redFILE0 << "("<<itrmi->first<<", "<<itrmi->second<<")"<<endl;
	
	//redFILE1 << "In find_common_fault_one:"<<endl;
	for(itrmi1 = first.begin(); itrmi1 != first.end(); itrmi1++)
	{
		int nec_line = itrmi1->first;
		int nec_value = itrmi1->second;
		if(nec_value != X)
		{
		    //redFILE1 << "*("<<nec_line<<", "<<nec_value<<")"<<endl;
		    this_time_MA.insert(pair<int, int>(nec_line, nec_value));	
		    deter_fault(orNodeList, nec_line, nec_value, untSet);
		}
	}
	
	return this_time_MA;
}

map<int, int> find_common_fault_two(map<int, CircuitNode> &orNodeList, map<int, int> &first, map<int, int> &second, multimap<int, FaultList> &untSet)
{
    map<int, CircuitNode>::iterator itrm, itrm1;
	map<int, int>::iterator itrmi, itrmi1, itrmi2, itrmi3;
	typedef map<int, map<int, int> >::iterator PAT;
	PAT ite;
	
	map<int, int> learned_assign;
	map<int, int> this_time_MA;
	redFILE0 << "Coming into find_common_fault_two:"<<endl;
	
	multimap<int, FaultList> fault_set_first, fault_set_second;
	multimap<int, FaultList>::iterator itrmf;
	
	//redFILE1 << "In find_common_fault_two:"<<endl;
	
	//redFILE1 << "MAs in first:"<<endl;
	for(itrmi1 = first.begin(); itrmi1 != first.end(); itrmi1++)
	{
	    int nec_line = itrmi1->first;
	    int nec_value = itrmi1->second;
	    itrm = orNodeList.find(nec_line);
	    if(itrm != orNodeList.end() && nec_value != X)
	    {
	       //redFILE1 << "**("<<nec_line<<", "<<nec_value<<")"<<endl;
	       deter_fault(orNodeList, nec_line, nec_value, fault_set_first); 
	    }	    
	}
	//redFILE1 << "MAs in second:"<<endl;
	for(itrmi1 = second.begin(); itrmi1 != second.end(); itrmi1++)
	{
	    int nec_line = itrmi1->first;
	    int nec_value = itrmi1->second;
	    itrm = orNodeList.find(nec_line);
	    if(itrm != orNodeList.end() && nec_value != X)
	    {
	       //redFILE1 << "**("<<nec_line<<", "<<nec_value<<")"<<endl;
	       deter_fault(orNodeList, nec_line, nec_value, fault_set_second); 	
	    }    
	}
	
	intersect(fault_set_first, fault_set_second);
	
	//redFILE1 << "faults found:"<<endl;
	for(itrmf = fault_set_first.begin(); itrmf != fault_set_first.end(); itrmf++)
	{
	    //redFILE1 << "("<<itrmf->first<<","<<itrmf->second.stuckAtValue<<")  ";
	    untSet.insert(pair<int, FaultList>(itrmf->first, itrmf->second));
	}
	//redFILE1  << endl;
	
	for(itrmi1 = first.begin(); itrmi1 != first.end(); itrmi1++)
	{
		itrmi2 = second.find(itrmi1->first);
		if(itrmi2 != second.end())
		{
			if(itrmi1->second == itrmi2->second)
			{
				int nec_line = itrmi2->first;
				int nec_value = itrmi2->second;
				if(nec_value != X)
					this_time_MA.insert(pair<int, int>(nec_line, nec_value));		
			}
		}
	}
	return this_time_MA;
}



map<int, int> justify_un(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &orNodeList, vector<int> input_values, int node, int just_value, int gateType, multimap<int, FaultList> &untSet, int r, int rmax, int &consist_flag)
{
    //iterators
	map<int, CircuitNode>::iterator itrm, itrm1, itrm_in0, itrm_in1;
	vector<CircuitNode*>::iterator itrv0, itrv1;
	set<int>::iterator itrs;
	map<int, int>::iterator itrmi, itrmi1, itrmi2, itrmi3;

	map<int, int> record_values_first, record_values_second, record_values_third;
	int consist_flag_first = -1, consist_flag_second = -1, consist_flag_third = -1;
	int consist_flag_first_f = -1, consist_flag_second_f = -1, consist_flag_third_f = -1;
	int consist_flag_first_s = -1, consist_flag_second_s = -1, consist_flag_third_s = -1;
	
	itrm = masterNodeList.find(node);
	itrv0 = itrm->second.pointFanIn.begin(); //first input
	itrm_in0 = masterNodeList.find((*itrv0)->lineNumber);
	itrv1 = itrm->second.pointFanIn.end();		
	itrv1--;                                 //second input
	itrm_in1 = masterNodeList.find((*itrv1)->lineNumber);
	int first_value = input_values[0];
	int second_value = input_values[1];
	int just_index = -1;

	redFILE0 << "starting justifying node "<<node<<endl;
	if(gateType == 7)
	{
		redFILE0 << "Gate type: AND gate."<<endl;		
		if(just_value == 1)
		{
            //(X, X, 0) : j2
			if(first_value == X && second_value == X)
			{
				just_index = 1;
				redFILE0 << "Justification 1: set first input "<<(*itrv0)->lineNumber<<"to 1."<<endl;
				record_values_first.insert(pair<int, int>((*itrv0)->lineNumber, 1));
				itrm_in0->second.lineValue.clear(); 
				itrm_in0->second.lineValue.insert(1);

				recursive_learning_un(masterNodeList, orNodeList, (*itrv0)->lineNumber, r+1, rmax, record_values_first, untSet, consist_flag_first);
				
				//Clear the values set to  lines during justification 1
				for(itrmi1 = record_values_first.begin(); itrmi1 != record_values_first.end(); itrmi1++)
				{

					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				}
	
				redFILE0 << "Justification 2: set second input "<<(*itrv1)->lineNumber<<"to 1."<<endl;
				record_values_second.insert(pair<int, int>((*itrv1)->lineNumber, 1));
				itrm_in1->second.lineValue.clear(); 
				itrm_in1->second.lineValue.insert(1);
					
				recursive_learning_un(masterNodeList, orNodeList, (*itrv1)->lineNumber, r+1, rmax, record_values_second, untSet, consist_flag_second);	
				
				//Clear the values set to  lines during justification 2
				for(itrmi2 = record_values_second.begin(); itrmi2 != record_values_second.end(); itrmi2++)
				{

					itrm = masterNodeList.find(itrmi2->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
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
				just_index = 1;
					
			    redFILE0 << "Justification 1: set first input "<<(*itrv0)->lineNumber<<"to 0."<<endl;
				record_values_first.insert(pair<int, int>((*itrv0)->lineNumber, 0));
				itrm_in0->second.lineValue.clear(); 
				itrm_in0->second.lineValue.insert(0);
				
				recursive_learning_un(masterNodeList, orNodeList, (*itrv0)->lineNumber, r+1, rmax, record_values_first, untSet, consist_flag_first);
				
				//Clear the values set to  lines during justification 1
				for(itrmi1 = record_values_first.begin(); itrmi1 != record_values_first.end(); itrmi1++)
				{

					itrm = masterNodeList.find(itrmi1->first);
					itrm->second.lineValue.clear();
					itrm->second.lineValue.insert(X);
				}
						
				redFILE0 << "Justification 2: set second input "<<(*itrv1)->lineNumber<<"to 0."<<endl;
				record_values_second.insert(pair<int, int>((*itrv1)->lineNumber, 0));
				itrm_in1->second.lineValue.clear(); 
				itrm_in1->second.lineValue.insert(0);
				
				recursive_learning_un(masterNodeList, orNodeList, (*itrv1)->lineNumber, r+1, rmax, record_values_second, untSet, consist_flag_second);	
				
				//Clear the values set to  lines during justification 2
				for(itrmi2 = record_values_second.begin(); itrmi2 != record_values_second.end(); itrmi2++)
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
	    redFILE0 << "record_values_first:"<<endl;
		for(itrmi = record_values_first.begin(); itrmi != record_values_first.end(); itrmi++)
		    redFILE0 << itrmi->first<<", "<<itrmi->second<<endl;
		redFILE0 << "record_values_second:"<<endl;
		for(itrmi = record_values_second.begin(); itrmi != record_values_second.end(); itrmi++)
		    redFILE0 << itrmi->first<<", "<<itrmi->second<<endl;
		    
		this_time_MA = find_common_fault_two(orNodeList, record_values_first, record_values_second, untSet);
	}
	else if(consist_flag_first + consist_flag_second == 1)
	{
		if(consist_flag_first == 0)
		{
			record_values_first.clear();
			this_time_MA = find_common_fault_one(orNodeList, record_values_second, untSet);
		}
		else if(consist_flag_second == 0)
		{
			record_values_second.clear();
			this_time_MA = find_common_fault_one(orNodeList, record_values_first, untSet);
		}
	}
	else if(consist_flag_first + consist_flag_second == 0)//If all justifications are inconsistent
	{
		record_values_first.clear();
		record_values_second.clear();
		consist_flag = 0;
	}
	
	return this_time_MA;

}


//recursive_learning
void recursive_learning_un(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &orNodeList, int current_node, int r, int rmax, map<int, int> &MA_dir, multimap<int, FaultList> &untSet, int &consist_flag)
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
	consist_flag = path_propagation_un(masterNodeList, current_node, remaining_unjust_gates, MA_dir);
	
	if(consist_flag == 0)   //inconsistent
	{
	    //cerr << "In recursive learning, direct implication fails."<<endl;
	    //redFILE1 << "In recursive learning, direct implication fails."<<endl;
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
				    this_time_MA = justify_un(masterNodeList, orNodeList, input_values, node, 1, 7, untSet, r, rmax, consist_flag);
				else if(itrm->second.gateType == 3)              //OR gate
				    this_time_MA = justify_un(masterNodeList, orNodeList, input_values, node, 0, 3, untSet, r, rmax, consist_flag);
															
				redFILE0 << "this_time_MA:"<<endl;
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
				}					
			}//for()					
		}//if(r < rmax && !remaining_unjust_gates.empty())
	
	}	

	return;
				
}//recursive_learning




