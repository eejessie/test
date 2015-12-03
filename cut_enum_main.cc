/*
 * =====================================================================================
 *
 *       Filename:  cut_enum_main.cc
 *
 *    Description:  
 *
 *        License:  
 *
 *        Version:  1.0
 *        Created:  18/10/2014
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Yi WU <eejessie@sjtu.edu.cn>
 *         
 * =====================================================================================
 */

// #####   HEADER FILE INCLUDES   ################################################### 

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <math.h>

#include "lib/forward_implication.h"  
#include "lib/string_convert.h"
#include "class/CircuitNode.h"         
#include "class/CircuitLine.h"         
#include "class/FaultList.h"           
#include "class/TestList.h"        
#include "function/helper.h"
#include "function/print_circuit.h"
#include "function/copy_point_vector.h"
#include "function/atpg.h"


// Global constant definitions.
#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0, redFILE1;
extern vector<int> outputList;
extern HashTable opl_hash;
map<int, vector<vector<int> > > cut_record;


int merge_vector(map<int, CircuitNode> &masterNodeList, vector<int> &cut3, vector<int> &cut1, vector<int> &cut2)
{
	//cut3 = cut1;
	int result = 0;
	
	map<int, int> cut1_map;
	map<int, int>::iterator itrmi;
	//map<int, int> cut2_map;
	for(int i = 0 ; i < cut1.size(); i++)
	    cut1_map.insert(pair<int, int>(cut1[i], cut1[i]));
	for(int i = 0 ; i < cut2.size(); i++)
	    cut1_map.insert(pair<int, int>(cut2[i], cut2[i]));
	
	for(itrmi = cut1_map.begin(); itrmi != cut1_map.end(); itrmi++)
	    cut3.push_back(itrmi->first);
		
/*    redFILE1 << "cut1: "<<endl;
    for(int i = 0; i < cut1.size(); i++)
        redFILE1 << cut1[i] << " ";
    redFILE1 << endl;
    redFILE1 << "cut2: "<<endl;
    for(int i = 0; i < cut2.size(); i++)
        redFILE1 << cut2[i] << " ";
    redFILE1 << endl;*/
/*	for(int i = 0; i < cut2.size(); i++)
		if(search_vector_int(cut1, cut2[i]) == -1)
		{
		    HashTable tranfanin;
		    tranfanin.Init_HashTable();
		    HashTable tranfanout;
		    tranfanout.Init_HashTable();
			find_tranfanin_hash(masterNodeList, tranfanin, cut2[i]);
			find_tranfanout_hash(masterNodeList, tranfanout, cut2[i]);
				
			for(int j = 0; j < cut1.size(); j++)
				//if(search_vector_int(tranfanin, cut1[j]) != -1 || search_vector_int(tranfanout, cut1[j]) != -1) 
			{
				int p;
				if(tranfanin.Search_Hash(cut1[j], p) == 1 || tranfanout.Search_Hash(cut1[j], p) == 1)
				{
					result = 1;
					tranfanin.Destroy_HashTable();
	                tranfanout.Destroy_HashTable();	
					return result;
				}
	        }	
	        tranfanin.Destroy_HashTable();
	        tranfanout.Destroy_HashTable();			
			cut3.push_back(cut2[i]);						
		}*/
			
	return result;
}


void cut_enum(map<int, CircuitNode> &masterNodeList, int current_node, vector<vector<int> > &cut_set, int sink, int th_num)
{
    //Iterators
	map<int, CircuitNode>::iterator itrm;
	map<int, vector<vector<int> > >::iterator itrmv;
	vector<CircuitNode*>::iterator itrv, itrv1, itrv2;
	set<int>::iterator itrs;

	itrm = masterNodeList.find(current_node);
	vector<int> current_cut_set;
	if((itrm->second.nodeType == 1) || (itrm->second.nodeType == 2) || (itrm->second.nodeType == 0 && (itrm->second.gateType == 3 || itrm->second.gateType == 7)))
	{
	//    redFILE1 << "Current node "<<current_node<<" is pushed into current_cut_set"<<endl;
	    current_cut_set.push_back(itrm->second.lineNumber);
	    cut_set.push_back(current_cut_set);
	}

	if(itrm->second.nodeType == 1)                    //PI
	{		
	//	redFILE1 << "Current node "<<itrm->second.lineNumber<<" is a PI!"<<endl;
		vector<vector<int> > PI_cut;
		PI_cut.push_back(current_cut_set);
		cut_record.insert(pair<int, vector<vector<int> > >(itrm->second.lineNumber, PI_cut));
		return;
	}
	else
	{
		int current_fanin;
		vector<vector<int> > *temp_cut;
		temp_cut = new vector<vector<int> > [2];
		if(temp_cut == NULL)
		{
			cerr<<"error!"<<endl;
			exit(1);
		}		
		int index = 0;
		for(itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++)
		{
			current_fanin = (*itrv)->lineNumber;	
	//		redFILE1 << "fanin: "<<current_fanin<<endl;	
			itrmv = cut_record.find(current_fanin);
			if(itrmv != cut_record.end())
				temp_cut[index++] = itrmv->second;
			else
				cut_enum(masterNodeList, current_fanin, temp_cut[index++], sink, th_num);
		}
		vector<vector<int> >::iterator itrvv1, itrvv2;
	//	redFILE1 << "!!Return: node = "<<itrm->second.lineNumber<<endl;
		if(index == 2)              //Current node has two inputs
		{		
			itrv1 = itrm->second.pointFanIn.begin();                  //first input
			itrv2 = itrm->second.pointFanIn.end();                    //second input
			itrv2--;
			if(temp_cut[0].empty())
				cut_set.insert(cut_set.end(), temp_cut[1].begin(), temp_cut[1].end());
			else if(temp_cut[1].empty())
				cut_set.insert(cut_set.end(), temp_cut[0].begin(), temp_cut[0].end());
			else
			{
		//	    redFILE1 << "temp_cut[0].size = "<<temp_cut[0].size()<<", temp_cut[1].size = "<<temp_cut[1].size()<<endl;
			    int i = 0;
				for(itrvv1 = temp_cut[0].begin(); itrvv1 != temp_cut[0].end(); itrvv1++, i++)
				{
				    if(i >= 500)
				        break;
				    int j = 0;
					for(itrvv2 = temp_cut[1].begin(); itrvv2 != temp_cut[1].end(); itrvv2++, j++)
					{
					    if(j >= 500)
					        break;
						//current_cut_set = *itrvv1;
						vector<int> temp_cut_set;
						int result = merge_vector(masterNodeList, temp_cut_set, *itrvv1, *itrvv2);
						if(result == 0)
							if(temp_cut_set.size() < th_num)
								cut_set.push_back(temp_cut_set);
					}
			    }
			}
					
			cut_record.insert(pair<int, vector<vector<int> > >((*itrv1)->lineNumber, temp_cut[0]));
			cut_record.insert(pair<int, vector<vector<int> > >((*itrv2)->lineNumber, temp_cut[1]));
			
		}
		else if(index == 1)           //Current node has one input
		{
			itrv1 = itrm->second.pointFanIn.begin();  
		//	if(temp_cut[0].size() < th_num) 
				cut_set.insert(cut_set.end(), temp_cut[0].begin(), temp_cut[0].end());
			cut_record.insert(pair<int, vector<vector<int> > >((*itrv1)->lineNumber, temp_cut[0]));
		}
		
		delete []temp_cut;
		return;
	}
		
}


void cut_enum_main(map<int, CircuitNode> &orNodeList,  vector<int> &last_ef_node, vector<vector<int> > &cut_set, int th_num)
{
	//Iterators
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3;
	set<int>::iterator itrs, itrs1;
	vector<CircuitNode*>::iterator itrv;

	
	//Add the sink node	
	int num_add;
	itrm1 = orNodeList.end();
	itrm1--;
	int maxLineNumber = itrm1->second.lineNumber; 
	int start_ln = maxLineNumber + 1;
	int sink;
	vector<int> nodes_for_sink;
	if(last_ef_node.size() == 1)
	{            
		num_add = 1;
		CircuitNode *addedNode_sin;
		addedNode_sin = new CircuitNode(PO);
		if(addedNode_sin == NULL)
		{
			cerr << "malloc error!"<<endl;
			exit(1);
		}
	  
		addedNode_sin->gateType = 11;                 //buffer         
		addedNode_sin->circuitIndex = 1;
		addedNode_sin->internal = 0;
		addedNode_sin->rectFlag = 0;
		addedNode_sin->lineNumber = start_ln;
		start_ln += 1;
		addedNode_sin->listFanIn.insert(last_ef_node[0]); //listFanIn
		addedNode_sin->numberFanIn = 1;            //numberFanIn
		addedNode_sin->numberFanOut = 0;                    //numberFanOut
		orNodeList.insert(pair<int, CircuitNode>(addedNode_sin->lineNumber, *addedNode_sin));
		itrm2 = orNodeList.find(addedNode_sin->lineNumber);
		itrm2->second.lineValue.insert(D);
		itrm3 = orNodeList.find(last_ef_node[0]);
		itrm2->second.pointFanIn.push_back(&itrm3->second);  //pointFanIn
		//sink = addedNode_sin->lineNumber;
		nodes_for_sink.push_back(addedNode_sin->lineNumber);
		
		delete addedNode_sin;
	}
	
	if(last_ef_node.size() > 1)	
	{
		num_add = last_ef_node.size() - 1;
		CircuitNode *addedNode_mul;
		addedNode_mul = new CircuitNode[num_add];  
		if(addedNode_mul == NULL)
		{
			cerr << "malloc error!"<<endl;
			exit(1);
		}
	
		for(int i = 0; i < num_add; i++)
		{
			addedNode_mul[i].nodeType = 0;
			if(i == num_add - 1)
				addedNode_mul[i].nodeType = 3;               
			addedNode_mul[i].gateType = 7;                  
			addedNode_mul[i].circuitIndex = 1;
			addedNode_mul[i].internal = 0;
			addedNode_mul[i].rectFlag = 0;
			addedNode_mul[i].lineNumber = start_ln;
			addedNode_mul[i].lineValue.insert(D);
			start_ln += 1;
			nodes_for_sink.push_back(addedNode_mul[i].lineNumber);
			
			if(i == 0)
			{
				addedNode_mul[i].listFanIn.insert(last_ef_node[0]); //listFanIn
				addedNode_mul[i].listFanIn.insert(last_ef_node[1]);
			}
			else
			{
				addedNode_mul[i].listFanIn.insert(addedNode_mul[i-1].lineNumber); //listFanIn
				addedNode_mul[i].listFanIn.insert(last_ef_node[i+1]); //listFanIn
			}
			addedNode_mul[i].numberFanIn = 2;   //numberFanIn
			addedNode_mul[i].numberFanOut = 0;                    //numberFanOut
			orNodeList.insert(pair<int, CircuitNode>(addedNode_mul[i].lineNumber, addedNode_mul[i]));
			itrm2 = orNodeList.find(addedNode_mul[i].lineNumber);
			for(itrs = itrm2->second.listFanIn.begin(); itrs != itrm2->second.listFanIn.end(); itrs++)
			{
				itrm3 = orNodeList.find(*itrs);
				itrm2->second.pointFanIn.push_back(&itrm3->second);  //pointFanIn
			}			
		}
		//sink = addedNode_mul[0].lineNumber;
		
		delete []addedNode_mul;
	}
		
	sink = start_ln - 1;
//	redFILE1 << "sink = "<<sink<<endl;
	
	/*cut_enum(): find all feasible cuts. */
	cut_enum(orNodeList, sink, cut_set, sink, th_num);	
	
	map<int, vector<vector<int> > >::iterator itrm_vv;
	vector<vector<int> >::iterator itrvv;
	vector<vector<int> > cut_vector;
	
	vector<int> FlagVec(cut_set.size(), 0);
	int i = 0;
	for(itrvv = cut_set.begin(); itrvv != cut_set.end(); itrvv++, i++)
	{
		vector<int> each_cut = *itrvv;
		int num_branch = 0;
		for(int j = 0; j < each_cut.size(); j++)
		{
			int p;
			if(search_vector_int(nodes_for_sink, each_cut[j]) != -1 || opl_hash.Search_Hash(each_cut[j], p) == 1)
			{
				FlagVec[i] = 1;
				break;
			}
			
			itrm = orNodeList.find(each_cut[j]);
			if(itrm->second.nodeType == 0 && itrm->second.gateType == 5)   //Ignore inverters.
			{
				FlagVec[i] = 1;
				break;
			}
			if(itrm->second.nodeType == 2 && itrm->second.gateType == 1)   //Ignore branches.
			    num_branch++;
		}
		if(num_branch == each_cut.size())
		    FlagVec[i] = 1;
		
	}
	remove_vector(cut_set, FlagVec);
	
	//Erase the sink node.
	for(int i = 0; i < nodes_for_sink.size(); i++)
	{
		itrm = orNodeList.find(nodes_for_sink[i]);
		orNodeList.erase(itrm);
	}
	
/*	map<int, vector<vector<int> > >::iterator itrmv;
	for(itrmv = cut_record.begin(); itrmv != cut_record.end(); itrmv++)
	{
		redFILE1 << "Node "<<itrmv->first<<":"<<endl;
		for(int i = 0; i < itrmv->second.size(); i++)
			{
				vector<int> temp = itrmv->second[i];
				redFILE1 << "Cut "<<i<<": {";
				for(int j = 0; j < temp.size(); j++)
					redFILE1 << temp[j] << " ";
				redFILE1 << "}"<<endl;
			}
			redFILE1 << endl;
	}*/
	
/*	redFILE1 << "Final cut_set:"<<endl;
	for(int i = 0; i < cut_set.size(); i++)
	{
		redFILE1 << "Cut "<<i<<": {";
		for(int j = 0; j < cut_set[i].size(); j++)
			redFILE1 << cut_set[i][j] << " ";
		redFILE1 << "}"<<endl;
	}
	redFILE1 << endl;*/
}
