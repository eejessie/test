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


void upd_circuit_plus(map<int, CircuitNode> &orNodeList, map<int, int> &wa_set, map<int, int> &ndp_set, int maxLineNumber)
{
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;
    map<int, int>::iterator itrmi, itrmi1;
    vector<CircuitNode*>::iterator itrv;
    
    itrmi = wa_set.begin();
    int wa_line = itrmi->first;
    int wa_value = itrmi->second;
    itrmi1 = ndp_set.begin();
    int d_line = itrmi1->first;
    int d_value = itrmi1->second;        
    
    itrm1 = orNodeList.find(wa_line);
    itrm2 = orNodeList.find(d_line);
    
    itrv = itrm1->second.pointFanOut.begin();
    itrm3 = orNodeList.find((*itrv)->lineNumber);
    
    //itrm_dl & itrm_out
    CircuitNode *addedNode_dl;
	addedNode_dl = new CircuitNode(FB);
	addedNode_dl->lineNumber = maxLineNumber + 1;
	addedNode_dl->gateType = 1;         //branch
	addedNode_dl->circuitIndex = 1;	
	orNodeList.insert(pair<int, CircuitNode>(addedNode_dl->lineNumber, *addedNode_dl));
	map<int, CircuitNode>::iterator itrm_dl = orNodeList.find(addedNode_dl->lineNumber);
	itrm_dl->second.listFanIn.insert(d_line);
	itrm_dl->second.pointFanIn.push_back(&itrm2->second);
		
	CircuitNode *addedNode_out;
	addedNode_out = new CircuitNode(FB);
	addedNode_out->lineNumber = maxLineNumber + 2;
	addedNode_out->gateType = 1;         //branch
	addedNode_out->circuitIndex = 1;	
	orNodeList.insert(pair<int, CircuitNode>(addedNode_out->lineNumber, *addedNode_out));
	map<int, CircuitNode>::iterator itrm_out = orNodeList.find(addedNode_out->lineNumber);
	itrm_out->second.listFanIn.insert(d_line);	
	itrm_out->second.pointFanIn.push_back(&itrm2->second);
	
	for(itrv = itrm2->second.pointFanOut.begin(); itrv != itrm2->second.pointFanOut.end(); itrv++)
	{
	    itrm4 = orNodeList.find((*itrv)->lineNumber);
	    itrm4->second.listFanIn.erase(d_line);
	    vector_erase(itrm4->second, d_line, 1);
	    itrm4->second.listFanIn.insert(addedNode_out->lineNumber);
	    itrm4->second.pointFanIn.push_back(&itrm_out->second);	    
	    itrm_out->second.pointFanOut.push_back(&itrm4->second);
	}
	
	itrm2->second.pointFanOut.clear();
	itrm2->second.pointFanOut.push_back(&itrm_dl->second);
	itrm2->second.pointFanOut.push_back(&itrm_out->second);
    
    
    
    //itrm_gate
    CircuitNode *addedNode_gate;
	addedNode_gate = new CircuitNode(GT);
	addedNode_gate->lineNumber = maxLineNumber + 3;
	if(itrm3->second.gateType == 3)
	    addedNode_gate->gateType = 7;  
	else if(itrm3->second.gateType == 7)
	    addedNode_gate->gateType = 3; 
	addedNode_gate->circuitIndex = 1;	
	orNodeList.insert(pair<int, CircuitNode>(addedNode_gate->lineNumber, *addedNode_gate));
	map<int, CircuitNode>::iterator itrm_gate = orNodeList.find(addedNode_gate->lineNumber);
	itrm_gate->second.listFanIn.insert(wa_line);
	itrm_gate->second.pointFanIn.push_back(&itrm1->second);
	
	for(itrv = itrm1->second.pointFanOut.begin(); itrv != itrm1->second.pointFanOut.end(); itrv++)
	{
	    itrm4 = orNodeList.find((*itrv)->lineNumber);
	    itrm4->second.listFanIn.erase(wa_line);
	    vector_erase(itrm4->second, wa_line, 1);
	    itrm4->second.listFanIn.insert(addedNode_gate->lineNumber);
	    itrm4->second.pointFanIn.push_back(&itrm_gate->second);	    
	    itrm_gate->second.pointFanOut.push_back(&itrm4->second);
	}
	
	itrm1->second.pointFanOut.clear();
	itrm1->second.pointFanOut.push_back(&itrm_gate->second);

	
    if(wa_value != d_value)
    {
        CircuitNode *addedNode_inv;
	    addedNode_inv = new CircuitNode(GT);
	    addedNode_inv->lineNumber = maxLineNumber + 4;
	    addedNode_inv->gateType = 5;         //inverter
	    addedNode_inv->circuitIndex = 1;	            	        
	    orNodeList.insert(pair<int, CircuitNode>(addedNode_inv->lineNumber, *addedNode_inv));
	    map<int, CircuitNode>::iterator itrm_inv = orNodeList.find(addedNode_inv->lineNumber);	
	    itrm_inv->second.listFanIn.insert(addedNode_dl->lineNumber);	  
	    itrm_inv->second.pointFanIn.push_back(&itrm_dl->second);
	    itrm_inv->second.pointFanOut.push_back(&itrm_gate->second);
	        
	    itrm_dl->second.pointFanOut.push_back(&itrm_inv->second);  	        
	    itrm_gate->second.listFanIn.insert(addedNode_inv->lineNumber);
	    itrm_gate->second.pointFanIn.push_back(&itrm_inv->second); 
	        
	    delete addedNode_inv; 	        
    }
    else
    {
        itrm_dl->second.pointFanOut.push_back(&itrm_gate->second); 
        itrm_gate->second.listFanIn.insert(addedNode_dl->lineNumber);
	    itrm_gate->second.pointFanIn.push_back(&itrm_dl->second);  
    }
        
    delete addedNode_dl;
    delete addedNode_out;
    delete addedNode_gate;

}
