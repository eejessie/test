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
int consistency_flag = -1;


void forward_imply(map<int, CircuitNode> &masterNodeList, FaultList wt_error, vector<int> &unjust_gates, vector<int> &extra_unjust_gates, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced, int flag_ao)
{
    //Iterators
	map<int, CircuitNode>::iterator itrm;
	set<int>::iterator itrs, itrs1, itrs2;
	vector<CircuitNode*>::iterator itrv, itrv1, itrv2;
	map<int, int>::iterator itrmi;
	typedef map<int, map<int, int> >::iterator PAT;
	PAT ite;
	
	set <int> gateVector[2];
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
		itrs = itrm->second.lineValue.begin();
		redFILE0 << endl << "Current line = "<<itrm->second.lineNumber<<endl;
		
		//If current line has value X, then use forwardImplication to derive its value.
		if(*itrs == X)
		{
			int inputNumber = 0;
			for (itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++) 
				gateVector[inputNumber++] = (*itrv)->lineValue;
			switch (itrm->second.pointFanIn.size()) 
			{										
	        	case 1:
	            	tempSet = forwardImplication(itrm->second.gateType, gateVector[0]);
	                break;
	            case 2:
	                tempSet = forwardImplication(itrm->second.gateType, gateVector[0], gateVector[1]);
	                break;
	            default:
					cerr << "This node is "<<itrm->second.lineNumber<<endl;
	                cerr << "This ATPG generator only works if the number of inputs to a gate is less than 4." << endl;
	                exit (0);
	                break;
           	}
           	itrm->second.lineValue = tempSet;			
           	itrs = itrm->second.lineValue.begin();

			redFILE0 << "Current line's new value = "<<*itrs<<endl;
			
			//update MA_obs
			if(flag_ao == 1)
			    MA_obs.insert(pair<int, int>(itrm->second.lineNumber, *itrs));
			else if(flag_ao == 0)
			    MA_act.insert(pair<int, int>(itrm->second.lineNumber, *itrs));
		}
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
			if(itrm->second.lineNumber == wt_error.lineNumber)
			{
				outputValue.clear();
				if(*itrs == D)
					outputValue.insert(1);
				else if(*itrs == B)
					outputValue.insert(0);
			}
			switch (itrm->second.pointFanIn.size()) 
			{										
			    case 1:
			        consistency_flag = check_consistency(itrm->second.gateType, outputValue, gateVector[0]);
			        break;
			    case 2:
			        consistency_flag = check_consistency(itrm->second.gateType, outputValue, gateVector[0], gateVector[1], num_known);
		            break;
		    }
		    if(consistency_flag == 0)
		    {
		        consistency_flag = 0;
	//	    	redFILE1 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
		    	redFILE0 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
		        return;
		    }
		}
		
	    //step2. Check the updated value of unjustified gates
		for(itrv1 = itrm->second.pointFanOut.begin(); itrv1 != itrm->second.pointFanOut.end(); itrv1++)
		{
			redFILE0 << "Fanout line = "<<(*itrv1)->lineNumber<<endl;
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
				if((*itrv1)->lineNumber == wt_error.lineNumber)
				{
					outputValue.clear();
					if(*itrs == D)
						outputValue.insert(1);
					else if(*itrs == B)
						outputValue.insert(0);
				}
							
				switch ((*itrv1)->pointFanIn.size()) 
				{										
				    case 1:
				        consistency_flag = check_consistency((*itrv1)->gateType, outputValue, gateVector[0]);
					    break;
				    case 2:
					    consistency_flag = check_consistency((*itrv1)->gateType, outputValue, gateVector[0], gateVector[1], num_known);
				        break;
				}
				if(consistency_flag == 0)
				{
				    consistency_flag = 0;
			//	    redFILE1 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
				    redFILE0 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
				    return;
				}
				else
				{
				    if(((*itrv1)->gateType == 3 || (*itrv1)->gateType == 7) && num_known == 2);
				    else if(((*itrv1)->gateType == 1 || (*itrv1)->gateType == 5) && num_known == 1);
				    //There are still inputs with value X to this gates
				    else if((num_known > 0) && (search_vector_int(extra_unjust_gates, (*itrv1)->lineNumber) == -1))
				    	extra_unjust_gates.push_back((*itrv1)->lineNumber);
				}
			}
			//Else if current line has fanout which has value X, then use forwardImplication to derive its fanout's value.
			else               //*itrs1 == X
			{
				switch ((*itrv1)->pointFanIn.size()) 
			    {										
			        case 1:
			        	tempSet = forwardImplication((*itrv1)->gateType, gateVector[0]);
			            break;
			        case 2:
			            tempSet = forwardImplication((*itrv1)->gateType, gateVector[0], gateVector[1]);
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

		        redFILE0 << "Fanout line's new value = "<<*itrs1<<endl;
		        if(*itrs1 != X)
		        {
		            if(flag_ao == 1)
		                MA_obs.insert(pair<int, int>((*itrv1)->lineNumber, *itrs1));
		            else if(flag_ao == 0)
		                MA_act.insert(pair<int, int>((*itrv1)->lineNumber, *itrs1));
		        
		            if((search_vector_int(unjust_gates, (*itrv1)->lineNumber) == -1) && (search_vector_int(added_unjust_gates, (*itrv1)->lineNumber) == -1))
			            added_unjust_gates.push_back((*itrv1)->lineNumber);
		        }
		    }
        }
	}//for()
	
	if(!added_unjust_gates.empty())
	{
		unjust_gates.clear();
		unjust_gates = added_unjust_gates;
		forward_imply(masterNodeList, wt_error, unjust_gates, extra_unjust_gates, MA_act, MA_obs, MA_forced, flag_ao);
	}
	else
		return;
}


void backward_imply(map<int, CircuitNode> &masterNodeList, FaultList wt_error, vector<int> &unjust_gates, vector<int> &extra_unjust_gates, vector<int> &remaining_unjust_gates, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced, int flag_ao)
{
    //Iterators
	map<int, CircuitNode>::iterator itrm;
	set<int>::iterator itrs, itrs1, itrs2, itrs_old0, itrs_old1, itrs_new0, itrs_new1;
	vector<CircuitNode*>::iterator itrv, itrv1, itrv2;
	map<int, int>::iterator itrmi;
	typedef map<int, map<int, int> >::iterator PAT;
	PAT ite;
	
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
		if(itrm->second.nodeType == 1)      //If current line is PI, then skip.
		{
			itrs = itrm->second.lineValue.begin();
			if(*itrs == D || *itrs == B)
			{	
				consistency_flag = 0;
		//		redFILE1 << "Inconsistency exists at PI "<<itrm->second.lineNumber<<"!"<<endl;
				redFILE0 << "Inconsistency exists at PI "<<itrm->second.lineNumber<<"!"<<endl;
				return;
			}
			else continue;
		}
		redFILE0 << endl <<"Current line = "<<itrm->second.lineNumber<<endl;
		itrs = itrm->second.lineValue.begin();
		redFILE0 << "Current line's value = "<<*itrs<<endl;
		
		set <int> outputValue = itrm->second.lineValue;
		if(itrm->second.lineNumber == wt_error.lineNumber)
	    {
	        outputValue.clear();
			if(wt_error.stuckAtValue)
				outputValue.insert(0);
			else
				outputValue.insert(1);
		}
		vector<int> value_diff;
	    value_diff.clear();
	    int old_value0, old_value1, new_value0, new_value1;
	    //If current line's value is known, then start backtracing.
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
			//consistency check
		    switch (itrm->second.pointFanIn.size()) 
			{										
		    	case 1:
		    		consistency_flag = check_consistency(itrm->second.gateType, outputValue, gateVector[0]);
		            break;
		       case 2:
		          	consistency_flag = check_consistency(itrm->second.gateType, outputValue, gateVector[0], gateVector[1], num_known);
	                break;
	        }
	        if(consistency_flag == 0)
	        {
	            consistency_flag = 0;
	  //          redFILE1 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
	            redFILE0 << "Inconsistency exists at "<<itrm->second.lineNumber<<"!"<<endl;
	            return;
	        }
	            
	        //backwardImplication	
	     //   redFILE0 << "num_known = "<<num_known<<", value_known = "<<value_known<<endl;  
	        if(num_known < itrm->second.pointFanIn.size())
	        {
			    switch (itrm->second.pointFanIn.size())             
				{										
				   	case 1:
				        itrs_old0 = gateVector[0].begin();
				        old_value0 = *itrs_old0;             //old value
				        backwardImplication1(itrm->second.gateType, outputValue, gateVector[0]);
				        itrv1 = itrm->second.pointFanIn.begin();
			            (*itrv1)->lineValue = gateVector[0];	
			            itrs_new0 = gateVector[0].begin();   
			            new_value0 = *itrs_new0;             //new value
			            if(old_value0 != new_value0)
			                value_diff.push_back((*itrv1)->lineNumber);   
		
			            //update MA_forced
			           // redFILE1 << "* node "<<(*itrv1)->lineNumber<<" is inserted into MA_forced"<<endl;
			            MA_forced.insert(pair<int, int>((*itrv1)->lineNumber, new_value0));
			            //update MA_obs
			            if(flag_ao == 1)
			                MA_obs.insert(pair<int, int>((*itrv1)->lineNumber, new_value0));
			            else if(flag_ao == 0)
			                MA_act.insert(pair<int, int>((*itrv1)->lineNumber, new_value0));
				        break;
				        
				    case 2:
				    {
				        itrs2 = outputValue.begin();
				        int indicate_remain = -1;				        
				        if(itrm->second.gateType == 7 && (*itrs2 == 0 || *itrs2 == D || *itrs2 == B))              				            	
				        {
				        	indicate_remain = 0;				            				            if(*itrs2 == 0 && value_known == 1)//(X, 1, 0)
				                indicate_remain = 1;
				            else if(*itrs2 == D && (value_known == 1 || value_known == D))//(X, 1, D), (X, D, D)
				                indicate_remain = 1;
				            else if(*itrs2 == B && (value_known == 1 || value_known == B))//(X, 1, B), (X, B, B)
				                indicate_remain = 1;
				            							            			
				            //(X, D, 0), (X, B, 0), (X, X, 0), (X, X, D), (X, X, B) need to be justified				            		
				            if(indicate_remain == 0)
					        {
					            int dont_add = 0;
					            for(int k = 0; k < remaining_unjust_gates.size(); k++)
					                if(remaining_unjust_gates[k] == itrm->second.lineNumber)
					                {
					                    dont_add = 1;
					                    break;
					                }
					            if(dont_add == 0)
					            {
					                remaining_unjust_gates.push_back(itrm->second.lineNumber);
					                redFILE0 << "Gate "<<itrm->second.lineNumber<<" is pushed into remaining_unjust_gates."<<endl;
					            }
				            }
				         }

				         if(itrm->second.gateType == 3 && (*itrs2 == 1 || *itrs2 == D || *itrs2 ==B ))
				         {
				            indicate_remain = 0;
				            if(*itrs2 == 1 && value_known == 0)//(X, 0, 1)
				                indicate_remain = 1;
				            else if(*itrs2 == D && (value_known == 0 || value_known == D))//(X, 0, D), (X, D, D)
				                indicate_remain = 1;
				            else if(*itrs2 == B && (value_known == 0 || value_known == B))//(X, 0, B), (X, B, B)
				            	indicate_remain = 1;
				            			
				            //(X, D, 1), (X, B, 1), (X, X, 1), (X, X, D), (X, X, B) need to be justified
				            if(indicate_remain == 0)
					        {
					            remaining_unjust_gates.push_back(itrm->second.lineNumber);
				            	redFILE0 << "Gate "<<itrm->second.lineNumber<<" is pushed into remaining_unjust_gates."<<endl;
				             }
				         }
				         //redFILE0 << "indicate_remain = "<<indicate_remain<<endl;
				         if(indicate_remain == 0)
				            break;
				         else         //This gate is not pushed into remaining_unjust_gates
					     {
					          itrs_old0 = gateVector[0].begin();
					          itrs_old1 = gateVector[1].begin();
					          old_value0 = *itrs_old0;    //old value
					          old_value1 = *itrs_old1;    //old value
					          backwardImplication2(itrm->second.gateType, outputValue, gateVector[0], gateVector[1]);
					          itrv1 = itrm->second.pointFanIn.begin();         
					          (*itrv1)->lineValue.clear();
					          (*itrv1)->lineValue = gateVector[0];
					          itrv2 = itrm->second.pointFanIn.end();
					          itrv2--;                                        
					          (*itrv2)->lineValue.clear();
					          (*itrv2)->lineValue = gateVector[1];
					          itrs_new0 = gateVector[0].begin();
					          itrs_new1 = gateVector[1].begin();
					          new_value0 = *itrs_new0;      //new value
					          new_value1 = *itrs_new1;      //new value
					          if(old_value0 != new_value0)
					          {
					               value_diff.push_back((*itrv1)->lineNumber);
					               itrs1 = (*itrv1)->lineValue.begin();
					               MA_forced.insert(pair<int, int>((*itrv1)->lineNumber, *itrs1));
					               //update MA_obs
					               if(flag_ao == 1)
					                    MA_obs.insert(pair<int, int>((*itrv1)->lineNumber, *itrs1)); 
					               else if(flag_ao == 0)
					                    MA_act.insert(pair<int, int>((*itrv1)->lineNumber, *itrs1)); 
					          } 
					          if(old_value1 != new_value1)
					          {
					               value_diff.push_back((*itrv2)->lineNumber);
					               itrs2 = (*itrv2)->lineValue.begin();
					               MA_forced.insert(pair<int, int>((*itrv2)->lineNumber, *itrs2));
					               //update MA_obs
					               if(flag_ao == 1)
					                    MA_obs.insert(pair<int, int>((*itrv2)->lineNumber, *itrs2)); 
					               else if(flag_ao == 0)
					                     MA_act.insert(pair<int, int>((*itrv2)->lineNumber, *itrs2)); 
					          }
					      }
					      
				          break;
				      }
				      default:
						  cerr << "This node is "<<itrm->second.lineNumber<<endl;
				          cerr << "This ATPG generator only works if the number of inputs to a gate is less than 4." << endl;
				          exit (0);
				          break;
			        }
			    }
	           	
	           	
	           	//If current line is a branch, push its partners with X into extar_unjust_gates for next loop of implication
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

		}
		
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
				}
			}
		}
	}//for()
	
	if(!added_unjust_gates.empty())
	{
		unjust_gates.clear();
		unjust_gates = added_unjust_gates;
		backward_imply(masterNodeList, wt_error, unjust_gates, extra_unjust_gates, remaining_unjust_gates, MA_act, MA_obs, MA_forced, flag_ao);
	}
	else
		return;
}





int path_propagation_new(map<int, CircuitNode> &masterNodeList, FaultList wt_error, int current_node, vector<int> &remaining_unjust_gates, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced, int flag_ao)
{
	
	redFILE0 << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	redFILE0 << "Starting path_propagtion!"<<endl;
	redFILE0 << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
	
	//print_circuit(masterNodeList, redFILE0);
	
	consistency_flag = -1;
	vector<int> unjust_gates, bak_unjust_gates;
	unjust_gates.push_back(current_node);
	bak_unjust_gates = unjust_gates;
	
	vector<int> extra_unjust_gates;
	extra_unjust_gates.clear();
	backward_imply(masterNodeList, wt_error, unjust_gates, extra_unjust_gates, remaining_unjust_gates, MA_act, MA_obs, MA_forced, flag_ao);
	if(consistency_flag == 0)                        //Inconsistency occurs.
		return consistency_flag;
	
	unjust_gates = bak_unjust_gates;
	unjust_gates.insert(unjust_gates.end(), extra_unjust_gates.begin(), extra_unjust_gates.end());
	
	while(!unjust_gates.empty())
	{
		//bak_unjust_gates = unjust_gates;
		redFILE0 <<endl<<"--Starting forward_imply!"<<endl;
		extra_unjust_gates.clear();
		forward_imply(masterNodeList, wt_error, unjust_gates, extra_unjust_gates, MA_act, MA_obs, MA_forced, flag_ao);
		if(consistency_flag == 0)
		    return consistency_flag;
		
		unjust_gates = extra_unjust_gates;
		redFILE0 << "--Starting backward_imply!"<<endl;
		extra_unjust_gates.clear();
		if(!unjust_gates.empty())
		{
			backward_imply(masterNodeList, wt_error, unjust_gates, extra_unjust_gates, remaining_unjust_gates, MA_act, MA_obs, MA_forced, flag_ao);
			if(consistency_flag == 0)
				return consistency_flag;
		}
		
		unjust_gates.clear();
		unjust_gates = extra_unjust_gates; 
	}
	consistency_flag = 1;
	return consistency_flag;                          //1: yes; 0: no
}

