//Created by Yi Wu on 21 September, 2014

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
#include "lib/file_operations.h"
#include "lib/forward_implication.h"
#include "lib/radix_convert.h"

#include "function/atpg.h"
#include "function/copy_point_vector.h"
#include "function/helper.h"
#include "function/print_circuit.h"

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif


using namespace std;

extern ofstream redFILE0, redFILE1;
int consistency_flag_un = -1;


void forward_imply_un(map<int, CircuitNode> &masterNodeList, vector<int> &unjust_gates, vector<int> &extra_unjust_gates, map<int, int> &record_values)
{
    //Iterators
	map<int, CircuitNode>::iterator itrm;
	set<int>::iterator itrs, itrs1, itrs2;
	vector<CircuitNode*>::iterator itrv, itrv1, itrv2;
	map<int, int>::iterator itrmi;

	set <int> gateVector[2];
    //gateVector = new set <int> [2];
    vector<int> added_unjust_gates;
    set <int> tempSet;
    
   	
   redFILE0 << endl <<"------------------------------------------"<<endl;
    redFILE0 <<     "Coming into forward_imply!"<<endl;
    redFILE0 << "unjust_gates: "<<endl;
	for(int i = 0; i < unjust_gates.size(); i++)
		redFILE0 << unjust_gates[i] << " ";
	redFILE0 << endl << endl;
    
	for(int i = 0; i < unjust_gates.size(); i++)
	{
		itrm = masterNodeList.find(unjust_gates[i]);
		int gate_type = itrm->second.gateType;
		if(itrm->second.gateType == 7)
		    gate_type = 3;
		else if(itrm->second.gateType == 3)
		    gate_type = 7;
		itrs = itrm->second.lineValue.begin();
		redFILE0 << endl << "Current line = "<<itrm->second.lineNumber<<endl;
		
		//step1. If current line has value X, then use forwardImplication to derive its value.
		if(*itrs == X)
		{
			int inputNumber = 0;
			for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
				gateVector[inputNumber++] = (*itrv)->lineValue;
		    
			switch (itrm->second.pointFanIn.size()) 
			{										
	        	case 1:
	            	tempSet = forwardImplication(gate_type, gateVector[0]);
	                break;
	            case 2:
	                tempSet = forwardImplication(gate_type, gateVector[0], gateVector[1]);
	                break;
	            default:
					cerr << "This node is "<<itrm->second.lineNumber<<endl;
	                cerr << "This implication generator only works if the number of inputs to a gate is less than 4." << endl;
	                exit (0);
	                break;
           	}
           	itrm->second.lineValue = tempSet;			
           	itrs = itrm->second.lineValue.begin();         
           	if(*itrs != X)
           		record_values.insert(pair<int, int>(itrm->second.lineNumber, *itrs));
			redFILE0 << "Current line's new value = "<<*itrs<<endl;
		}
		//Else if current line has value other than X, then check if it is consistent with its inputs.
		else if(*itrs != X)
		{
			//consistency check
			set <int> outputValue = itrm->second.lineValue;	
			int inputNumber = 0;
			int num_known = 0;
			for (itrv1 = itrm->second.pointFanIn.begin(); itrv1 != itrm->second.pointFanIn.end(); itrv1++) 
			{
				gateVector[inputNumber++] = (*itrv1)->lineValue;	
				itrs1 = (*itrv1)->lineValue.begin();
				if(*itrs1 != X)
					num_known++;	
			}
			switch (itrm->second.pointFanIn.size()) 
			{										
			    case 1:
			        consistency_flag_un = check_consistency(gate_type, outputValue, gateVector[0]);
			        break;
			    case 2:
			        consistency_flag_un = check_consistency(gate_type, outputValue, gateVector[0], gateVector[1], num_known);
		            break;
		    }
		    //redFILE1<<"1. after check, consistency_flag_un = "<<consistency_flag_un<<endl;
		    if(consistency_flag_un == 0)
		    {
		    	redFILE1 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
		        return;
		    }
		}
		
		//step2. Check the updated value of unjustified gates
	    for(itrv1 = itrm->second.pointFanOut.begin(); itrv1 != itrm->second.pointFanOut.end(); itrv1++)
		{
			redFILE0 << "Fanout line = "<<(*itrv1)->lineNumber<<endl;
			int gate_type_fanout = (*itrv1)->gateType;
		    if((*itrv1)->gateType == 7)
		        gate_type_fanout = 3;
		    else if((*itrv1)->gateType == 3)
		        gate_type_fanout = 7;
			itrs1 = (*itrv1)->lineValue.begin();
			redFILE0 << "Fanout line's value = "<<*itrs1<<endl;
			int inputNumber = 0;
			int num_known = 0;
			for (itrv2 = (*itrv1)->pointFanIn.begin(); itrv2 != (*itrv1)->pointFanIn.end(); itrv2++) 
			{
				gateVector[inputNumber++] = (*itrv2)->lineValue;	
				itrs2 = (*itrv2)->lineValue.begin();
				redFILE0 << "fanin value: ("<<(*itrv2)->lineNumber<<", "<<*itrs2<<")"<<endl;
				if(*itrs2 != X)
					num_known++;	
			}
			//If current line has fanout that has known value, then check the consistency. 
			if(*itrs1 != X)
			{
				//consistency check
				set <int> outputValue = (*itrv1)->lineValue;
				itrs = outputValue.begin();					
				switch ((*itrv1)->pointFanIn.size()) 
				{										
			        case 1:
					    consistency_flag_un = check_consistency(gate_type_fanout, outputValue, gateVector[0]);
					    break;
					case 2:
					    consistency_flag_un = check_consistency(gate_type_fanout, outputValue, gateVector[0], gateVector[1], num_known);
				        break;
				}
				//redFILE1<<"2. after check, consistency_flag_un = "<<consistency_flag_un<<endl;
				if(consistency_flag_un == 0)
				{
				    redFILE1 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
				    return;
				}
				else
				{
				    if(((*itrv1)->gateType == 3 || (*itrv1)->gateType == 7) && num_known == 2);
				    else if(((*itrv1)->gateType == 1 || (*itrv1)->gateType == 5) && num_known == 1);
				    //There are still inputs with value X to this gates
				    else if((num_known > 0) && (search_vector_int(extra_unjust_gates, (*itrv1)->lineNumber) == -1))
				    {				    	
				    	extra_unjust_gates.push_back((*itrv1)->lineNumber);
				    }
				}
			}
			//Else if current line has fanout which has value X, then use forwardImplication to derive its fanout's value.
			else               //*itrs1 == X
			{
				switch ((*itrv1)->pointFanIn.size()) 
				{										
			        case 1:
			            tempSet = forwardImplication(gate_type_fanout, gateVector[0]);
			            break;
			        case 2:
			            tempSet = forwardImplication(gate_type_fanout, gateVector[0], gateVector[1]);
			            break;
			        default:
						cerr << "This node is "<<itrm->second.lineNumber<<endl;
			            cerr << "This ATPG generator only works if the number of inputs to a gate is less than 4." << endl;
			            exit (0);
			            break;
		        }
		        (*itrv1)->lineValue.clear();
		        (*itrv1)->lineValue =  tempSet;
		        itrs1 = tempSet.begin();
		        if(*itrs1 != X)
           		    record_values.insert(pair<int, int>((*itrv1)->lineNumber, *itrs1));                 
           		redFILE0 << "Fanout line's new value = "<<*itrs1<<endl;
		        if(*itrs1 != X)
		        {
		            if((search_vector_int(unjust_gates, (*itrv1)->lineNumber) == -1) && (search_vector_int(added_unjust_gates, (*itrv1)->lineNumber) == -1))
			        {
			            added_unjust_gates.push_back((*itrv1)->lineNumber);
			            //redFILE0 << "Line "<< (*itrv1)->lineNumber << " is pushed into added_unjust_gates"<<endl;
			        }
		        }
			}
			
	    }//for(itrv1 = itrm->second.pointFanOut.begin(); itrv1 != itrm->second.pointFanOut.end(); itrv1++)
	    
	}//for(int i = 0; i < unjust_gates.size(); i++)

	if(!added_unjust_gates.empty())
	{
		unjust_gates.clear();
		unjust_gates = added_unjust_gates;
		forward_imply_un(masterNodeList, unjust_gates, extra_unjust_gates, record_values);
	}
	else
		return;
}


void backward_imply_un(map<int, CircuitNode> &masterNodeList, vector<int> &unjust_gates, vector<int> &extra_unjust_gates, vector<int> &remaining_unjust_gates, map<int, int> &record_values)
{
    //Iterators
	map<int, CircuitNode>::iterator itrm;
	set<int>::iterator itrs, itrs1, itrs2, itrs_old0, itrs_old1, itrs_new0, itrs_new1;
	vector<CircuitNode*>::iterator itrv, itrv1, itrv2;
	map<int, int>::iterator itrmi;

	set <int> gateVector[2];

        
    redFILE0 << endl<<"------------------------------------------"<<endl;
    redFILE0 << 		 "Coming into backward_imply!"<<endl;
    redFILE0 << "unjust_gates: "<<endl;
	for(int i = 0; i < unjust_gates.size(); i++)
		redFILE0 << unjust_gates[i] << " ";
	redFILE0 << endl << endl;
	
	vector<int> added_unjust_gates;
	added_unjust_gates.clear();
	
	for(int i = 0; i < unjust_gates.size(); i++)
	{
		itrm = masterNodeList.find(unjust_gates[i]);
		int gate_type = itrm->second.gateType;
		if(itrm->second.gateType == 7)
		    gate_type = 3;
		else if(itrm->second.gateType == 3)
		    gate_type = 7;
		//step 1. If current line is PI, then skip.
		if(itrm->second.nodeType == 1)          
            continue;
            
		redFILE0 << endl <<"Current line = "<<itrm->second.lineNumber<<endl;
		itrs = itrm->second.lineValue.begin();
		redFILE0 << "Current line's value = "<<*itrs<<endl;
		
		set <int> outputValue = itrm->second.lineValue;
		vector<int> value_diff;
	    value_diff.clear();
	    int old_value0, old_value1, new_value0, new_value1;
	    //step 2. If current line's value is known, then start backtracing.
		if(*itrs != X)                                      
		{
			int inputNumber = 0;
			int num_known = 0;
			int value_known = -1;
			for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
			{
				gateVector[inputNumber++] = (*itrv)->lineValue;
				itrs1 = (*itrv)->lineValue.begin();
				if(*itrs1 != X)
				{
					value_known = *itrs1;
					num_known++;
				}
			}
			//step 2.1 consistency check
			switch (itrm->second.pointFanIn.size()) 
			{										
		        case 1:
		        	consistency_flag_un = check_consistency(gate_type, outputValue, gateVector[0]);
		            break;
		        case 2:
		            consistency_flag_un = check_consistency(gate_type, outputValue, gateVector[0], gateVector[1], num_known);
	                break;
	        }
	        if(consistency_flag_un == 0)
	        {
	            redFILE1 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
	            return;
	        }
	            
	        //step 2.2 backwardImplication	
	        //redFILE0 << "num_known = "<<num_known<<endl;            
	        if(num_known < itrm->second.pointFanIn.size())
	        {
	           // redFILE1 <<"number of input: "<<itrm->second.pointFanIn.size()<<endl;
			    switch (itrm->second.pointFanIn.size())             
				{										
				    case 1:
				        itrs_old0 = gateVector[0].begin();
				        old_value0 = *itrs_old0;
				        backwardImplication1(gate_type, outputValue, gateVector[0]);
				        itrv1 = itrm->second.pointFanIn.begin();
			            (*itrv1)->lineValue = gateVector[0];	
			            itrs_new0 = gateVector[0].begin();
			            new_value0 = *itrs_new0; 
			            if(old_value0 != new_value0)
			                value_diff.push_back((*itrv1)->lineNumber);   
		
			            itrs2 = (*itrv1)->lineValue.begin();
                        if(*itrs2 != X)
			            {
			            	record_values.insert(pair<int, int>((*itrv1)->lineNumber, *itrs2));
			            	//redFILE1 << "("<<(*itrv1)->lineNumber<<", "<<*itrs2<<") is pushed into MA."<<endl;
			            }
			            break;
				    case 2:
				    {
				        itrs2 = outputValue.begin();
				        int indicate_remain = -1;
				        //If current line is AND gate and has value 0, then put it into remaining_unjust_gates.
				        if(gate_type == 7 && *itrs2 == 0 )             
				        {
				            indicate_remain = 0;				                   			
				            if(*itrs2 == 0 && value_known == 1)//(X, 1, 0)
				                indicate_remain = 1;					            
				            if(indicate_remain == 0)  //(X, X, 0)need to be justified	
					        {
					            remaining_unjust_gates.push_back(itrm->second.lineNumber);
					            redFILE0 << "Gate "<<itrm->second.lineNumber<<" is pushed into remaining_unjust_gates."<<endl;
				            }
				        }
				        //If current line is OR gate and has value 1, then put it into remaining_unjust_gates.
				        if(gate_type == 3 && *itrs2 == 1 )
				        {
				            indicate_remain = 0;
				            if(*itrs2 == 1 && value_known == 0)//(X, 0, 1)
				                indicate_remain = 1;				            
				            if(indicate_remain == 0)//(X, X, 1)need to be justified
					        {
					            remaining_unjust_gates.push_back(itrm->second.lineNumber);
				            	redFILE0 << "Gate "<<itrm->second.lineNumber<<" is pushed into remaining_unjust_gates."<<endl;
				            }
				        }
				        
				        //redFILE0 << "indicate_remain = "<<indicate_remain<<endl;
				        if(indicate_remain == 0)
				            break;
				        else    //This gate is not pushed into remaining_unjust_gates
					    {
					        itrs_old0 = gateVector[0].begin();
					        itrs_old1 = gateVector[1].begin();
					        old_value0 = *itrs_old0;
					        old_value1 = *itrs_old1;
					        backwardImplication2(gate_type, outputValue, gateVector[0], gateVector[1]);
					        itrv1 = itrm->second.pointFanIn.begin();          //first input
					        (*itrv1)->lineValue.clear();
					        (*itrv1)->lineValue = gateVector[0];
					        itrv2 = itrm->second.pointFanIn.end();
					        itrv2--;                                          //second input
					        (*itrv2)->lineValue.clear();
					        (*itrv2)->lineValue = gateVector[1];
					        itrs_new0 = gateVector[0].begin();
					        itrs_new1 = gateVector[1].begin();
					        new_value0 = *itrs_new0;
					        new_value1 = *itrs_new1;
	
					        if(old_value0 != new_value0)
					        {
					            value_diff.push_back((*itrv1)->lineNumber);
					            itrs1 = (*itrv1)->lineValue.begin();
                                if(*itrs1 != X)
					            {
					                record_values.insert(pair<int, int>((*itrv1)->lineNumber, *itrs1));
					                //redFILE1 << "("<<(*itrv1)->lineNumber<<", "<<*itrs1<<") is pushed into MA."<<endl;
					            }
					        } 
					        if(old_value1 != new_value1)
					        {
					            value_diff.push_back((*itrv2)->lineNumber);
					            itrs2 = (*itrv2)->lineValue.begin();
					            if(*itrs2 != X)
			            		{
			            			record_values.insert(pair<int, int>((*itrv2)->lineNumber, *itrs2));
			            			//redFILE1 << "("<<(*itrv2)->lineNumber<<", "<<*itrs2<<") is pushed into MA."<<endl;
			            		}
                            }
                        }
                        break;
                    }//case 2
				    default:
					    cerr << "This node is "<<itrm->second.lineNumber<<endl;
				        cerr << "This ATPG generator only works if the number of inputs to a gate is less than 3." << endl;
				        exit (0);
				        break;
			    }//switch (itrm->second.pointFanIn.size()) 
			}//if(num_known < itrm->second.pointFanIn.size())
	           	
	           	
	        //step 2.3 If current line is a branch, push its partners with X into extar_unjust_gates for next loop of implication
	        if(itrm->second.nodeType == 2 || (itrm->second.nodeType == 0 && itrm->second.gateType == 1))  	           	
	            if(itrm->second.pointFanIn.size() == 1)
	           	{
	           	    itrv = itrm->second.pointFanIn.begin();
	           		for (itrv1 = (*itrv)->pointFanOut.begin(); itrv1 != (*itrv)->pointFanOut.end(); itrv1++) 
					{
						itrs = (*itrv1)->lineValue.begin();
						if(((*itrv1)->lineNumber != itrm->second.lineNumber) && (*itrs == X))
							if(search_vector_int(extra_unjust_gates, (*itrv1)->lineNumber) == -1)
							{
								extra_unjust_gates.push_back((*itrv1)->lineNumber);
								//redFILE0 << "Line "<< (*itrv1)->lineNumber << " is pushed into extra_unjust_gates"<<endl;
							}
					}
				}

		}//if(*itrs != X) 
		

        //step 3. update added_unjust_gates
		for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
		{
			redFILE0 << "Current line's fanin "<<(*itrv)->lineNumber<< " has value ";
			itrs = (*itrv)->lineValue.begin();
			redFILE0 << *itrs <<endl;
			if((*itrs != X) && (search_vector_int(value_diff, (*itrv)->lineNumber) != -1))
			{
				if(search_vector_int(unjust_gates, (*itrv)->lineNumber) == -1 && (search_vector_int(added_unjust_gates, (*itrv)->lineNumber) == -1))
		        {
					added_unjust_gates.push_back((*itrv)->lineNumber);
					//redFILE0 << "Line "<< (*itrv)->lineNumber << " is pushed into added_unjust_gates"<<endl;
				}
			}
		}
		
		
	}//for(int i = 0; i < unjust_gates.size(); i++)
	
	if(!added_unjust_gates.empty())
	{
		unjust_gates.clear();
		unjust_gates = added_unjust_gates;
		backward_imply_un(masterNodeList, unjust_gates, extra_unjust_gates, remaining_unjust_gates, record_values);
	}
	else
		return;
		
    
}


int path_propagation_un(map<int, CircuitNode> &masterNodeList, int current_node, vector<int> &remaining_unjust_gates,  map<int, int> &MA)
{

	redFILE0 << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	redFILE0 << "Starting path_propagtion!"<<endl;
	redFILE0 << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	
	map<int, CircuitNode>::iterator itrm;
	set<int>::iterator itrs;
	
	consistency_flag_un = -1;
	vector<int> unjust_gates, bak_unjust_gates;
	unjust_gates.push_back(current_node);
	bak_unjust_gates = unjust_gates;
	
	vector<int> extra_unjust_gates;
	extra_unjust_gates.clear();
	backward_imply_un(masterNodeList, unjust_gates, extra_unjust_gates, remaining_unjust_gates, MA);
	//redFILE1  << "1. consistency_flag_un = "<<consistency_flag_un<<endl;

	if(consistency_flag_un == 0)                        //Inconsistency occurs.
		return consistency_flag_un;
	
	unjust_gates = bak_unjust_gates;
	unjust_gates.insert(unjust_gates.end(), extra_unjust_gates.begin(), extra_unjust_gates.end());
	
	while(!unjust_gates.empty())
	{
		redFILE0 <<endl<<"--Starting forward_imply!"<<endl;
		extra_unjust_gates.clear();
		forward_imply_un(masterNodeList, unjust_gates, extra_unjust_gates, MA);
		//redFILE1  << "2. consistency_flag_un = "<<consistency_flag_un<<endl;
		if(consistency_flag_un == 0)
			break;
		
		unjust_gates = extra_unjust_gates;
		redFILE0 << "--Starting backward_imply!"<<endl;
		extra_unjust_gates.clear();
		if(!unjust_gates.empty())
		{
			backward_imply_un(masterNodeList, unjust_gates, extra_unjust_gates, remaining_unjust_gates, MA);
			if(consistency_flag_un == 0)
				break;
		}
		
		unjust_gates.clear();
		unjust_gates = extra_unjust_gates; 
	}
	
	return consistency_flag_un;                          //1: yes; 0: no
}

