#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <ctime>
#include <sys/timeb.h>

#include "lib/forward_implication.h"   
#include "lib/string_convert.h"
#include "class/CircuitNode.h"         
#include "class/CircuitLine.h"         
#include "class/FaultList.h"           
#include "class/TestList.h"  
#include "class/HashTable.h"        
#include "function/helper.h"
#include "function/print_circuit.h"
#include "function/copy_point_vector.h"
#include "function/recursive_learning_new.h"

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

extern ofstream redFILE1, redFILE0;

int compute_MA(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &masterNodeList, FaultList & wt_error, map<int, int> &MA_act, map<int, int> &MA_obs, map<int, int> &MA_forced)
{
    //Iterators
	vector<CircuitNode*>::iterator itrv, itrv1;
	map<int, int>::iterator itrmi, itrmi1, itrmi2;
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;

    int line = wt_error.lineNumber;
    int sa_value = wt_error.stuckAtValue;
    
    redFILE1 << "sa_line = "<<line<<endl;
    
 //   redFILE1 << "1. find implications for the activating MA ("<<line<<", "<<1-sa_value<<")"<<endl;
    redFILE0 << "1. find implications for the activating MA ("<<line<<", "<<1-sa_value<<")"<<endl;
    for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
    {
        itrm->second.lineValue.clear();
        if(itrm->second.gateType == 9)
            itrm->second.lineValue.insert(0);
        else if(itrm->second.gateType == 10)
            itrm->second.lineValue.insert(1);
        else itrm->second.lineValue.insert(X);
    }
    itrm = masterNodeList.find(line);
    if(itrm == masterNodeList.end())
    {
        cerr <<"In compute_MA, this line doesn't exist!"<<endl;
        exit(1);
    }
    itrm->second.lineValue.clear();
    if(sa_value == 0)
    {
        itrm->second.lineValue.insert(D);
        MA_act.insert(pair<int, int>(line, D));
        MA_forced.insert(pair<int, int>(line, D));
    }
    else
    {
        itrm->second.lineValue.insert(B);
        MA_act.insert(pair<int, int>(line, B));
        MA_forced.insert(pair<int, int>(line, B));
    }
 //   print_circuit(masterNodeList, redFILE0);
 
    struct timeb startTime, endTime;
    
    ftime(&startTime);
    
    int r = 0, rmax = 3;
    int consist_flag = -1;
    int flag_ao = 0;  //Type: for activating MA
    recursive_learning_new(masterNodeList, wt_error, line, r, rmax, MA_act, MA_obs, MA_forced, consist_flag, flag_ao);
    if(consist_flag == 0)
    {
 //       redFILE1 << "recursive learning for activating fails"<<endl;
        return 1;
    }
//    redFILE1 << "end of activation!"<<endl;
    redFILE0 << "end of activation!"<<endl;
    ftime(&endTime);
	redFILE1 << "runtime for activation: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;
    
    ftime(&startTime);

    redFILE1 << "In compute_MA, "<<endl;
    HashTable tranfanout;
    tranfanout.Init_HashTable();
    for(itrm1 = masterNodeList.begin(); itrm1 != masterNodeList.end(); itrm1++)
	    itrm1->second.visit = 0;		
    find_tranfanout_hash(masterNodeList, tranfanout, line);

    ftime(&endTime);
//	redFILE1 << "runtime for find tranfanout: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;
    
    ftime(&startTime);

    /*find dominators*/
    map<int, int> dom;
//    find_dominator_main(orNodeList, dom, line);
    find_dominator_main(masterNodeList, dom, line);
    redFILE1 << "2. dominators of the current node "<<line<<":"<<endl;
    redFILE0 << "2. dominators of the current node "<<line<<":"<<endl;
    
    ftime(&endTime);
	redFILE1 << "runtime for find dominator: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;

    map<int, int> start_MA;
    for(itrmi = dom.begin(); itrmi != dom.end(); itrmi++)
    {
  //      redFILE1 << "current node = "<<itrmi->first<<endl;
        if(itrmi->first == line)
            continue;
        redFILE0 << itrmi->first<<" ";  
        itrm1 = masterNodeList.find(itrmi->first);    
    /*    itrm1 = orNodeList.find(itrmi->first);
        if(itrm1 == orNodeList.end())
            continue;*/
        if(itrm1->second.nodeType == 2 || (itrm1->second.nodeType == 0 && itrm1->second.gateType == 1)) //branch
            continue;
        else if(itrm1->second.nodeType == 0 && itrm1->second.gateType == 5)   //inverter
            continue;
        else if(itrm1->second.nodeType == 0 && (itrm1->second.gateType == 3 || itrm1->second.gateType == 7))
        {
            redFILE1 << "current dominator: "<<itrmi->first<<" ci = "<<itrm1->second.circuitIndex << endl;
 
            for(itrv1 = itrm1->second.pointFanIn.begin(); itrv1 != itrm1->second.pointFanIn.
            end(); itrv1++)
            {
                int p;
                if(tranfanout.Search_Hash((*itrv1)->lineNumber, p) == 0 && (*itrv1)->lineNumber != line)
                {
                   // redFILE1 << "side input = "<<(*itrv1)->lineNumber<<endl;
                    if(itrm1->second.gateType == 3)
                        start_MA.insert(pair<int, int>((*itrv1)->lineNumber, 0));
                    else
                        start_MA.insert(pair<int, int>((*itrv1)->lineNumber, 1));
                }               
            }
        } 
        
    }
    redFILE1 << endl;
    
    tranfanout.Destroy_HashTable();
    
//    redFILE1 << endl << "3. start_MAs: "<<endl;
    redFILE0 << endl << "3. start_MAs: "<<endl;
    for(itrmi = start_MA.begin(); itrmi != start_MA.end(); itrmi++)
    {
  //      redFILE1 << "("<<itrmi->first<<", "<<itrmi->second<<")"<<endl;
        redFILE0 << "("<<itrmi->first<<", "<<itrmi->second<<")"<<endl;
    }
        
    ftime(&startTime);
    
    int flag_untestable = 0;
    for(itrmi = start_MA.begin(); itrmi != start_MA.end(); itrmi++)
    {
      //  redFILE1 << "3. find implications for ("<<itrmi->first<<", "<<itrmi->second<<")"<<endl;
        redFILE0 << "3. find implications for ("<<itrmi->first<<", "<<itrmi->second<<")"<<endl;

        itrm = masterNodeList.find(itrmi->first);
        itrm->second.lineValue.clear();
        itrm->second.lineValue.insert(itrmi->second);
        MA_obs.insert(pair<int, int>(itrmi->first, itrmi->second));
        MA_forced.insert(pair<int, int>(itrmi->first, itrmi->second));
        
        int flag = 0, r = 0, rmax = 3;
      //  FaultList wt_error(-1, 0);
        map<int, int> current_MA;
        map<int, int> record_values;
        int consist_flag;
     //   redFILE1 << "start recursive learning for node "<<itrmi->first<<endl;
     //   redFILE0 << "start recursive learning for node "<<itrmi->first<<endl;
        flag_ao = 1;  //Type: for observability MA
        r = 0; rmax = 3; consist_flag = -1;
        recursive_learning_new(masterNodeList, wt_error, itrmi->first, r, rmax, MA_act, MA_obs, MA_forced, consist_flag, flag_ao);
        if(consist_flag == 0)
        {
       //     redFILE1 << "recursive learning for propagating fails"<<endl;
            return 1;
        }

    } 
    ftime(&endTime);
	redFILE1 << "runtime for propagation: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;
    
    for(itrmi = dom.begin(); itrmi != dom.end(); itrmi++)
    {
        itrmi1 = MA_act.find(itrmi->first);
        if(itrmi1 != MA_act.end())
        {
           // redFILE1 << "1. node "<<itrmi1->first<<" is inserted into MA_forced"<<endl;
            MA_forced.insert(pair<int, int>(itrmi1->first, itrmi1->second));
        }
        else
        {
            itrmi1 = MA_obs.find(itrmi->first);
            if(itrmi1 != MA_obs.end())
            {
              //  redFILE1 << "2. node "<<itrmi1->first<<" is inserted into MA_forced"<<endl;
                MA_forced.insert(pair<int, int>(itrmi1->first, itrmi1->second));
            }
        }
    }
    
    
    
    return 0;
            
}






