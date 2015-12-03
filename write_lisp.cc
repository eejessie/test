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
#include "function/atpg.h"        
#include "function/helper.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

extern int PO_line;
extern vector<int> inputList;
extern ofstream redFILE1;


using namespace std;

void write_lisp(map<int, CircuitNode> &nodeList, vector<string> &br_lisp_log, ofstream &oufile)
{
    map<int, CircuitNode>::iterator itrm, itrm0, itrm1, itrm2;
    vector<CircuitNode*>::iterator itrv, itrv0, itrv1;
    map<int, int>::iterator itrmi;
    
    map<int, int> rep_set; 
    for(itrm = nodeList.begin(); itrm != nodeList.end(); itrm++)
    {
        if(itrm->second.nodeType == 2)
        {
            int node_type = itrm->second.nodeType;
            int cnode = itrm->second.lineNumber;
            while(node_type == 2)
            {
		        itrm1 = nodeList.find(cnode);
		        itrv1 = itrm1->second.pointFanIn.begin();
		        itrm2 = nodeList.find((*itrv1)->lineNumber);
		        node_type = itrm2->second.nodeType;
		        cnode = itrm2->second.lineNumber;
		    }
		    rep_set.insert(pair<int, int>(itrm->second.lineNumber, cnode));	    
        } 
    }
        
    oufile.open ("lisp.log", ofstream::out);
    
    //preamble ":exists"
    oufile << ":exists "<<endl<<endl;
    
    //variables
    oufile << "(";
    for(int i = 0; i < inputList.size(); i++)
        oufile << "(n"<<inputList[i]<<" 1"<<") ";
    oufile << ")"<<endl << endl;
    
    //functions
    oufile << "((set0 (1) ((in 1)) (and 1b0 1b0)) (set1 (1) ((in 1))(and 1b1 1b1)) (buf (1) ((in 1))(in 0)))"<<endl<<endl;
    
    //formula
    oufile << "(local (";
    
    vector<int> sort_list;
    top_sort(nodeList, sort_list);
    for(int i = 0; i < sort_list.size(); i++)
    {
        itrm = nodeList.find(sort_list[i]);        
		if(itrm->second.nodeType == 1 || itrm->second.nodeType == 2)
		    continue;
		int gate_type = itrm->second.gateType;
		int node_type = itrm->second.nodeType;
		int cnode = itrm->second.lineNumber;
		switch(gate_type)
		{
		    case 9:
		        oufile << "(n"<<cnode<<" 1b0) "<<endl;
		        break;
		    case 10:
		        oufile << "(n"<<cnode<<" 1b1) "<<endl;
		        break;
		    case 11:
		    {
		        itrv = itrm->second.pointFanIn.begin();
		        int fanin_node = (*itrv)->lineNumber;
		        itrm1 = nodeList.find(fanin_node);
		        if(itrm1->second.nodeType == 2)
		        {
		            itrmi = rep_set.find(itrm1->second.lineNumber);
		            fanin_node = itrmi->second;
		        }		       		            
		        oufile << "(n"<<cnode<<" (buf n"<<fanin_node<<")) "<<endl;
		        break;
		    }
		    case 5:
		    {
		        itrv = itrm->second.pointFanIn.begin();
		        int fanin_node = (*itrv)->lineNumber;
		        itrm1 = nodeList.find(fanin_node);
		        if(itrm1->second.nodeType == 2)
		        {
		            itrmi = rep_set.find(itrm1->second.lineNumber);
		            fanin_node = itrmi->second;
		        }		       		            
		        oufile << "(n"<<cnode<<" (not n"<<fanin_node<<")) "<<endl;
		        break;
		    }
		    case 7:
		    {
		        itrv0 = itrm->second.pointFanIn.begin();
		        itrv1 = itrm->second.pointFanIn.end();
		        itrv1--;
		        int fanin0 = (*itrv0)->lineNumber;
		        int fanin1 = (*itrv1)->lineNumber;
		        itrm0 = nodeList.find(fanin0);
		        itrm1 = nodeList.find(fanin1);
		        if(itrm0->second.nodeType == 2)
		        {
		            itrmi = rep_set.find(itrm0->second.lineNumber);
		            fanin0 = itrmi->second;
		        }	
		        if(itrm1->second.nodeType == 2)
		        {
		            itrmi = rep_set.find(itrm1->second.lineNumber);
		            fanin1 = itrmi->second;
		        }
		        oufile << "(n"<<cnode<<" (and n"<<fanin0<<" n"<<fanin1<<")) "<<endl;
		        break;
		    }
		    case 3:
		    {
		        itrv0 = itrm->second.pointFanIn.begin();
		        itrv1 = itrm->second.pointFanIn.end();
		        itrv1--;
		        int fanin0 = (*itrv0)->lineNumber;
		        int fanin1 = (*itrv1)->lineNumber;
		        itrm0 = nodeList.find(fanin0);
		        itrm1 = nodeList.find(fanin1);
		        if(itrm0->second.nodeType == 2)
		        {
		            itrmi = rep_set.find(itrm0->second.lineNumber);
		            fanin0 = itrmi->second;
		        }	
		        if(itrm1->second.nodeType == 2)
		        {
		            itrmi = rep_set.find(itrm1->second.lineNumber);
		            fanin1 = itrmi->second;
		        }
		        oufile << "(n"<<cnode<<" (or n"<<fanin0<<" n"<<fanin1<<")) "<<endl;
		        break;
		    }		       
		}//switch()			    		    
    }//for()
    
    oufile << endl;
    
    for(int i = 0; i < br_lisp_log.size(); i++)
    {
    //    redFILE1 << br_lisp_log[i]<<endl;
        oufile << br_lisp_log[i];
    }
        
    oufile << endl;

    oufile.close();
   
    
}
