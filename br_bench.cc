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
#include "lib/radix_convert.h"       

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

extern int PO_line;
extern vector<int> inputList;
extern vector<int> brInputList;
extern ofstream redFILE1;

using namespace std;
//using namespace nameSpace_ATPG;

void br_bench(map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector<string> &br_bench_log)
{
    map<int, CircuitNode>::iterator itrm, itrm0, itrm1, itrm2;
    vector<CircuitNode*>::iterator itrv, itrv0, itrv1;
    map<int, int>::iterator itrmi;

	map<int, int> rep_set;
    redFILE1 << "Compute rep_set:"<<endl;   
    cout << "Compute rep_set:"<<endl;  
    for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
    {
        if(itrm->second.nodeType == 2)
        {
            int node_type = itrm->second.nodeType;
            int cnode = itrm->second.lineNumber;
            while(node_type == 2)
            {
		        itrm1 = masterNodeList.find(cnode);
		        itrv1 = itrm1->second.pointFanIn.begin();
		        itrm2 = masterNodeList.find((*itrv1)->lineNumber);
		        node_type = itrm2->second.nodeType;
		        cnode = itrm2->second.lineNumber;
		    }
		    rep_set.insert(pair<int, int>(itrm->second.lineNumber, cnode));	
		//    redFILE1 << "("<<    itrm->second.lineNumber<<","<<cnode<<")"<<endl;
        } 
    }

    vector<int> sort_list;
    top_sort(brNodeList, sort_list);
    for(int i = 0; i < sort_list.size(); i++)
    {
        itrm = brNodeList.find(sort_list[i]);        
		if(itrm->second.nodeType == 1 || itrm->second.nodeType == 2)
		{
		    continue;	    
		}
		int gate_type = itrm->second.gateType;
		int node_type = itrm->second.nodeType;
		int cnode = itrm->second.lineNumber;
	//	redFILE1 << "current node = "<<cnode<<endl;
		    string comm;
		    switch(gate_type)
		    {
		        case 11:
		        {
		            itrv = itrm->second.pointFanIn.begin();
		            int fanin_node = (*itrv)->lineNumber;
		            itrm1 = brNodeList.find(fanin_node);
		            if(itrm1->second.nodeType == 2)
		            {
		                itrmi = rep_set.find(itrm1->second.lineNumber);
		                fanin_node = itrmi->second;
		            }		       		            
		            comm = "n";
		            string node = RadixConvert(cnode, 10);
		            comm = comm.append(node);
		            comm = comm.append(" = BUF(n");
		            string fanin = RadixConvert(fanin_node, 10);
		            comm = comm.append(fanin);
		            comm = comm.append(")");
		            break;
		        }
		        case 5:
		        {
		            itrv = itrm->second.pointFanIn.begin();
		            int fanin_node = (*itrv)->lineNumber;
		       //     redFILE1 << "0. fanin_node = "<<fanin_node<<endl;
		            itrm1 = brNodeList.find(fanin_node);
		            if(itrm1->second.nodeType == 2)
		            {
		                itrmi = rep_set.find(itrm1->second.lineNumber);
		                fanin_node = itrmi->second;
		            }	
		        //    redFILE1 << "1. fanin_node = "<<fanin_node<<endl;	
		            comm = "n";
		            string node = RadixConvert(cnode, 10);
		            comm = comm.append(node);
		            comm = comm.append(" = NOT(n");
		            string fanin = RadixConvert(fanin_node, 10);
		            comm = comm.append(fanin);
		            comm = comm.append(")");
		      //      redFILE1 << "comm = "<<comm<<endl;		                  
		            break;
		        }
		        case 7:
		        {
		            itrv0 = itrm->second.pointFanIn.begin();
		            itrv1 = itrm->second.pointFanIn.end();
		            itrv1--;
		            int fanin0 = (*itrv0)->lineNumber;
		            int fanin1 = (*itrv1)->lineNumber;
		            itrm0 = brNodeList.find(fanin0);
		            itrm1 = brNodeList.find(fanin1);
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
		            {
		                comm = "n";
		                string node = RadixConvert(cnode, 10);
		                comm = comm.append(node);
		                comm = comm.append(" = AND(n");
		                string fanin0_c = RadixConvert(fanin0, 10);  
		                comm = comm.append(fanin0_c);
		                comm = comm.append(", n");
		                string fanin1_c = RadixConvert(fanin1, 10);
		                comm = comm.append(fanin1_c);
		                comm = comm.append(")");
		            }    
		            break;
		        }
		        case 3:
		        {
		            itrv0 = itrm->second.pointFanIn.begin();
		            itrv1 = itrm->second.pointFanIn.end();
		            itrv1--;
		            int fanin0 = (*itrv0)->lineNumber;
		            int fanin1 = (*itrv1)->lineNumber;
		            itrm0 = brNodeList.find(fanin0);
		            itrm1 = brNodeList.find(fanin1);
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
		            {
		                comm = "n";
		                string node = RadixConvert(cnode, 10);
		                comm = comm.append(node);
		                comm = comm.append(" = OR(n");
		                string fanin0_c = RadixConvert(fanin0, 10);  
		                comm = comm.append(fanin0_c);
		                comm = comm.append(", n");
		                string fanin1_c = RadixConvert(fanin1, 10);
		                comm = comm.append(fanin1_c);
		                comm = comm.append(")");
		            }    
		            break;
		        }
		    }//switch()	
		    
		    br_bench_log.push_back(comm);
		    	    
		}//for()
        
}
