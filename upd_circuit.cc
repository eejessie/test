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


int upd_circuit(map<int, CircuitNode> &orNodeList, map<int, int> &ns_set, map<int, int> &nd_set, int maxLineNumber)
{
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3;
    map<int, int>::iterator itrmi, itrmi1;
    vector<CircuitNode*>::iterator itrv;
    
    itrmi = ns_set.begin();
    int ns = itrmi->first;
    int ns_value = itrmi->second;
    itrmi1 = nd_set.begin();
    int nd = itrmi1->first;
    int nd_value = itrmi1->second;
    
    //redFILE1 << "In upd_circuit, ns = "<<ns<<", ns_value = "<<ns_value<<", nd = "<<nd<<", nd_value = "<<nd_value<<endl;
    
    //print_circuit(orNodeList, redFILE1);
    
    int wa_line;
    
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
	addedNode_gate->listFanIn.insert(nd);	
	orNodeList.insert(pair<int, CircuitNode>(addedNode_gate->lineNumber, *addedNode_gate));
	map<int, CircuitNode>::iterator itrm_gate = orNodeList.find(addedNode_gate->lineNumber);
	itrm_gate->second.pointFanIn.push_back(&itrm2->second);
	
	for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++)
	{
	    itrm3 = orNodeList.find((*itrv)->lineNumber);
	    itrm3->second.listFanIn.erase(nd);
	    vector_erase(itrm3->second, nd, 1);
	    itrm3->second.listFanIn.insert(addedNode_gate->lineNumber);
	    itrm3->second.pointFanIn.push_back(&itrm_gate->second);	    
	    itrm_gate->second.pointFanOut.push_back(&itrm3->second);
	}
	
	itrm2->second.pointFanOut.clear();
	itrm2->second.pointFanOut.push_back(&itrm_gate->second);
    
    if(nd_value == 0 || nd_value == D)
    {
        itrm_gate->second.gateType = 3;  
        if(ns_value == 0)
        {
            CircuitNode *addedNode_inv;
	        addedNode_inv = new CircuitNode(GT);
	        addedNode_inv->lineNumber = maxLineNumber + 4;
	        addedNode_inv->gateType = 5;         //inverter
	        addedNode_inv->circuitIndex = 1;
	        addedNode_inv->listFanIn.insert(addedNode_ns->lineNumber);	        	        
	        orNodeList.insert(pair<int, CircuitNode>(addedNode_inv->lineNumber, *addedNode_inv));
	        map<int, CircuitNode>::iterator itrm_inv = orNodeList.find(addedNode_inv->lineNumber);	  
	        itrm_inv->second.pointFanIn.push_back(&itrm_ns->second);
	        itrm_inv->second.pointFanOut.push_back(&itrm_gate->second);
	        
	        itrm_ns->second.pointFanOut.push_back(&itrm_inv->second);  
	        
	        itrm_gate->second.listFanIn.insert(addedNode_inv->lineNumber);
	        itrm_gate->second.pointFanIn.push_back(&itrm_inv->second); 
	        
	        wa_line = itrm_inv->second.lineNumber;
	        
	        delete addedNode_inv; 	        
        }
        else if(ns_value == 1)
        {
            itrm_ns->second.pointFanOut.push_back(&itrm_gate->second);
            itrm_gate->second.listFanIn.insert(addedNode_ns->lineNumber);
	        itrm_gate->second.pointFanIn.push_back(&itrm_ns->second);  	
	        
	        wa_line = itrm_ns->second.lineNumber;  	        
        }        
    }
    else if(nd_value == 1 || nd_value == B)
    {
        itrm_gate->second.gateType = 7; 
        if(ns_value == 1)
        {
            CircuitNode *addedNode_inv;
	        addedNode_inv = new CircuitNode(GT);
	        addedNode_inv->lineNumber = maxLineNumber + 4;
	        addedNode_inv->gateType = 5;         //inverter
	        addedNode_inv->circuitIndex = 1;
	        addedNode_inv->listFanIn.insert(addedNode_ns->lineNumber);	               
	        orNodeList.insert(pair<int, CircuitNode>(addedNode_inv->lineNumber, *addedNode_inv));
	        map<int, CircuitNode>::iterator itrm_inv = orNodeList.find(addedNode_inv->lineNumber);	
	        itrm_inv->second.pointFanIn.push_back(&itrm_ns->second);
	        itrm_inv->second.pointFanOut.push_back(&itrm_gate->second);	 
	          
	        itrm_ns->second.pointFanOut.push_back(&itrm_inv->second);  
	        
	        itrm_gate->second.listFanIn.insert(addedNode_inv->lineNumber);
	        itrm_gate->second.pointFanIn.push_back(&itrm_inv->second); 
	        
	        wa_line = itrm_inv->second.lineNumber; 
	        
	        delete addedNode_inv; 	  	        
        }
        else if(ns_value == 0)
        {
            itrm_ns->second.pointFanOut.push_back(&itrm_gate->second);
            itrm_gate->second.listFanIn.insert(addedNode_ns->lineNumber);
	        itrm_gate->second.pointFanIn.push_back(&itrm_ns->second);  
	        
	        wa_line = itrm_ns->second.lineNumber;	  	        
        }         
    }
    
    delete addedNode_ns;
    delete addedNode_out;
    delete addedNode_gate;
    
    return wa_line;

}
