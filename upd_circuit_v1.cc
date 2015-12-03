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

#include "class/CircuitNode.h"        
#include "class/HashTable.h"
#include "function/copy_point_vector.h"
#include "function/print_circuit.h"
#include "function/helper.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0, redFILE1;


void upd_circuit_v1(map<int, CircuitNode> &orNodeList, map<int, int> &ns_set, map<int, int> &nd_set, int maxLineNumber)
{
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;
    map<int, int>::iterator itrmi, itrmi1;
    vector<CircuitNode*>::iterator itrv, itrv1;
    
    itrmi = ns_set.begin();
    int ns = itrmi->first;
    int ns_value = itrmi->second;
    itrmi1 = nd_set.begin();
    int nd = itrmi1->first;
    int nd_value = itrmi1->second;
    
    //redFILE1 << "In upd_circuit, ns = "<<ns<<", ns_value = "<<ns_value<<", nd = "<<nd<<", nd_value = "<<nd_value<<endl;
    
    //print_circuit(orNodeList, redFILE1);
    
    itrm1 = orNodeList.find(ns);
    itrm2 = orNodeList.find(nd);
    
    CircuitNode *addedNode_ns;
	addedNode_ns = new CircuitNode(FB);
	addedNode_ns->lineNumber = maxLineNumber + 1;
	addedNode_ns->gateType = 1;         //branch
	addedNode_ns->circuitIndex = 1;
	addedNode_ns->listFanIn.insert(ns);
	orNodeList.insert(pair<int, CircuitNode>(addedNode_ns->lineNumber, *addedNode_ns));
	map<int, CircuitNode>::iterator itrm_ns = orNodeList.find(addedNode_ns->lineNumber);
	itrm_ns->second.pointFanIn.push_back(&itrm1->second);
	
	CircuitNode *addedNode_out;
	addedNode_out = new CircuitNode(FB);
	addedNode_out->lineNumber = maxLineNumber + 2;
	addedNode_out->gateType = 1;         //branch
	addedNode_out->circuitIndex = 1;
	addedNode_out->listFanIn.insert(ns);	
	orNodeList.insert(pair<int, CircuitNode>(addedNode_out->lineNumber, *addedNode_out));
	//orNodeList.insert(pair<int, CircuitNode>(addedNode_out->lineNumber, *addedNode_out));
	map<int, CircuitNode>::iterator itrm_out = orNodeList.find(addedNode_out->lineNumber);
	itrm_out->second.pointFanIn.push_back(&itrm1->second);
	
	for(itrv = itrm1->second.pointFanOut.begin(); itrv != itrm1->second.pointFanOut.end(); itrv++)
	{
	    itrm3 = orNodeList.find((*itrv)->lineNumber);
	    itrm3->second.listFanIn.erase(ns);
	    vector_erase(itrm3->second, ns, 1);
	    itrm3->second.listFanIn.insert(addedNode_out->lineNumber);
	    itrm3->second.pointFanIn.push_back(&itrm_out->second);	    
	    itrm_out->second.pointFanOut.push_back(&itrm3->second);
	}
	
	itrm1->second.pointFanOut.clear();
	itrm1->second.pointFanOut.push_back(&itrm_ns->second);
	itrm1->second.pointFanOut.push_back(&itrm_out->second);
	
	CircuitNode *addedNode_gate;
	addedNode_gate = new CircuitNode(GT);
	addedNode_gate->lineNumber = maxLineNumber + 3;
	addedNode_gate->circuitIndex = 1;
	orNodeList.insert(pair<int, CircuitNode>(addedNode_gate->lineNumber, *addedNode_gate));
	map<int, CircuitNode>::iterator itrm_gate = orNodeList.find(addedNode_gate->lineNumber);
	
	if(nd_value == 0)
	    itrm_gate->second.gateType =  3;
	else if(nd_value == 1)
	    itrm_gate->second.gateType =  7; 
	    
	int flag_case = -1;
	if(itrm2->second.nodeType == 2 || (itrm2->second.nodeType == 0 && itrm2->second.gateType == 1))
	{
	    itrv = itrm2->second.pointFanIn.begin();
	    HashTable tranfanin_ns;
        tranfanin_ns.Init_HashTable();
        find_tranfanin_hash(orNodeList, tranfanin_ns, ns);
        int p;
        if(tranfanin_ns.Search_Hash((*itrv)->lineNumber, p) == 0)
            flag_case = 1;
        else
            flag_case = 2;	  
        tranfanin_ns.Destroy_HashTable();  
	}
	
	
	if(flag_case == 1 || (itrm2->second.gateType == 7 && nd_value == 1) || (itrm2->second.gateType == 3 && nd_value == 0))
	{
	    itrv = itrm2->second.pointFanIn.begin();
	    itrm3 = orNodeList.find((*itrv)->lineNumber);   //new nd: the input of original nd
	    int new_nd = itrm3->second.lineNumber;
	    
	    for(itrv1 = itrm3->second.pointFanOut.begin(); itrv1 != itrm3->second.pointFanOut.end(); itrv1++)
	    {
	        itrm4 = orNodeList.find((*itrv1)->lineNumber);
	        itrm4->second.listFanIn.erase(new_nd);
	        vector_erase(itrm4->second, new_nd, 1);
	        itrm4->second.listFanIn.insert(addedNode_gate->lineNumber);
	        itrm4->second.pointFanIn.push_back(&itrm_gate->second);	    
	        itrm_gate->second.pointFanOut.push_back(&itrm4->second);
	    }
	    itrm3->second.pointFanOut.clear();
	    itrm3->second.pointFanOut.push_back(&itrm_gate->second);
	    
	    itrm_gate->second.listFanIn.insert(new_nd);
	    itrm_gate->second.pointFanIn.push_back(&itrm3->second);
	}
	else if(flag_case == 2 || (itrm2->second.gateType == 7 && nd_value == 0) || (itrm2->second.gateType == 3 && nd_value == 1) || itrm2->second.nodeType == 1 )
	{
	    for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++)
	    {
	        itrm4 = orNodeList.find((*itrv)->lineNumber);
	        itrm4->second.listFanIn.erase(nd);
	        vector_erase(itrm4->second, nd, 1);
	        itrm4->second.listFanIn.insert(addedNode_gate->lineNumber);
	        itrm4->second.pointFanIn.push_back(&itrm_gate->second);	    
	        itrm_gate->second.pointFanOut.push_back(&itrm4->second);
	    }
	    itrm2->second.pointFanOut.clear();
	    itrm2->second.pointFanOut.push_back(&itrm_gate->second);
	    
	    itrm_gate->second.listFanIn.insert(nd);
	    itrm_gate->second.pointFanIn.push_back(&itrm2->second);
	}

    if(ns_value == nd_value)            //need an inverter
    {
        CircuitNode *addedNode_inv;
	    addedNode_inv = new CircuitNode(GT);
	    addedNode_inv->lineNumber = maxLineNumber + 4;
	    addedNode_inv->gateType = 5;         //inverter
	    addedNode_inv->circuitIndex = 1;	           	        
	    orNodeList.insert(pair<int, CircuitNode>(addedNode_inv->lineNumber, *addedNode_inv));
	    map<int, CircuitNode>::iterator itrm_inv = orNodeList.find(addedNode_inv->lineNumber);
	    itrm_inv->second.listFanIn.insert(addedNode_ns->lineNumber);	 	  
	    itrm_inv->second.pointFanIn.push_back(&itrm_ns->second);
	    itrm_inv->second.pointFanOut.push_back(&itrm_gate->second);
	            
	    itrm_ns->second.pointFanOut.push_back(&itrm_inv->second);  
	            
	    itrm_gate->second.listFanIn.insert(addedNode_inv->lineNumber);
	    itrm_gate->second.pointFanIn.push_back(&itrm_inv->second); 
         
	    delete addedNode_inv; 	        
    }
    else
    {
        itrm_ns->second.pointFanOut.push_back(&itrm_gate->second);  
        itrm_gate->second.listFanIn.insert(addedNode_ns->lineNumber);
	    itrm_gate->second.pointFanIn.push_back(&itrm_ns->second); 
    }
    
    delete addedNode_ns, addedNode_out, addedNode_gate;

}
