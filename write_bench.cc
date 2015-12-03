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
#include "function/merge_circuit.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

extern int PO_line;
extern vector<int> inputList;
extern vector<int> outputList;
extern ofstream redFILE1;

using namespace std;

void write_bench_basic(map<int, CircuitNode> &nodeList, map<int, int> &const_pair, ofstream &oufile)
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
        //inputs
        for(int i = 0; i < inputList.size(); i++)
            oufile << "INPUT(n"<<inputList[i]<<")"<<endl; 
               
        //gates
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
		            const_pair.insert(pair<int, int>(cnode, 0));
		            break;
		        case 10:
		            const_pair.insert(pair<int, int>(cnode, 1));
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
		            oufile << "n"<<cnode<<" = BUF(n"<<fanin_node<<")"<<endl;
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
		            oufile << "n"<<cnode<<" = NOT(n"<<fanin_node<<")"<<endl;
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
		            oufile << "n"<<cnode<<" = AND(n"<<fanin0<<", n"<<fanin1<<")"<<endl;
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
		            oufile << "n"<<cnode<<" = OR(n"<<fanin0<<", n"<<fanin1<<")"<<endl;
		            break;
		        }		       
		    }//switch()	    		    		    		    
        }//for()
        
}

void write_bench(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, vector<string> &br_bench_log, ofstream &oufile)
{
    map<int, CircuitNode>::iterator itrm, itrm0, itrm1, itrm2;
    vector<CircuitNode*>::iterator itrv, itrv0, itrv1;
    map<int, int>::iterator itrmi;
    
    int flag_const = 0;
    for(int i = 0; i < outputList.size(); i++)
    {
        itrm = orNodeList.find(outputList[i]);
        if(itrm->second.gateType == 9 || itrm->second.gateType == 10)
        {
            flag_const = 1;
            break;
        }        
    }
    
    oufile.open("bench.log", ofstream::out); 
    
    map<int, int> const_pair;
    
    if(flag_const == 0)
    {
   //     redFILE1 << "flag_const = " << flag_const<<endl;
        write_bench_basic(orNodeList, const_pair, oufile);  
               		    
		for(int i = 0; i < br_bench_log.size(); i++)
        {
            oufile << br_bench_log[i] << endl;
        }   
    }

    else if(flag_const == 1)
    {
   //     redFILE1 << "flag_const = " << flag_const<<endl;
        map<int, CircuitNode> tmp_masterNodeList;
        merge_circuit(tmp_masterNodeList, orNodeList, brNodeList);
        for(itrmi = const_pair.begin(); itrmi != const_pair.end(); itrmi++)
        {
            itrm = tmp_masterNodeList.find(itrmi->first);
            if(itrm != tmp_masterNodeList.end())
                simplify_circuit(tmp_masterNodeList, itrmi->first, itrmi->second);
        }
        write_bench_basic(tmp_masterNodeList, const_pair, oufile);        
    }
    
    int last_node = PO_line + 1;                   
    oufile << "n"<<last_node<<" = NOT(n"<<PO_line<<")"<<endl<<endl;
    //outputs   
    oufile << "OUTPUT(n" << last_node << ")" << endl;  	 
 
    oufile.close();
      
}


void write_bench_or(map<int, CircuitNode> &orNodeList, ofstream &oufile, string &truename)
{
    map<int, CircuitNode>::iterator itrm, itrm0, itrm1, itrm2;
    vector<CircuitNode*>::iterator itrv, itrv0, itrv1;
    map<int, int>::iterator itrmi;
   
    string path = "./result_bench/";
    path = path.append(truename);
    path = path.append(".bench");
    oufile.open(path.c_str(), ofstream::out); 
    
    map<int, int> const_pair;
    write_bench_basic(orNodeList, const_pair, oufile); 
    
    //outputs
    for(int i = 0; i < outputList.size(); i++)
        oufile << "OUTPUT(n"<<outputList[i]<<")"<<endl;     
               		    
    oufile.close();
      
}
