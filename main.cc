#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <math.h>
#include <cassert>
#include <ctime>
#include <sys/timeb.h>

#include "class/CircuitNode.h"         
#include "class/CircuitLine.h"        
#include "class/FaultList.h"           
#include "class/TestList.h"  
#include "class/HashTable.h"          
#include "lib/file_operations.h"
#include "lib/radix_convert.h"
#include "function/read_circuit_v2.h"
#include "function/print_circuit.h"
#include "function/copy_point_vector.h"
#include "function/atpg.h"
#include "function/adjust_ln.h"
#include "function/merge_circuit.h"
#include "function/helper.h"
#include "function/run_logic_simulation.h"
#include "function/red_forward_deletion_v2.h"
#include "function/red_backward_deletion.h"
#include "function/rem_red_sat.h"
#include "function/opt_rar_sat.h"
#include "function/br_lisp.h"
#include "function/write_lisp.h"
#include "function/br_bench.h"
#include "function/write_bench.h"
#include "function/call_abc.h"

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

#include "../../CUDD/cudd-2.4.1/cudd/cudd.h"
#include "../../CUDD/cudd-2.4.1/cudd/cuddInt.h"
#include "cudd/cudd_build.h"
#include "cudd/cudd_comp.h"
#include "cudd/cudd_dst.h"

using namespace std;


//global variables
vector<int> cut_nodes;
vector<string> testPool;
int numPI, numPO, totalInputs;
vector<int> inputList;//The list to store the primary inputs to the original circuit.
vector <int> brInputList; 
vector<int> outputList;            //The list to store the primary outputs to the original circuit.
HashTable opl_hash;
int PO_line;
vector<string> masterInputVector, masterInputVector1;
ofstream redFILE0, redFILE1, record_hash, timeRecord, logFile, untFile, oufile;


int check_inv_chain(map<int, CircuitNode> &orNodeList)
{
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;
    vector<CircuitNode*>::iterator itrv, itrv1, itrv2, itrv3, itrv4;
    
    int count_minus = 0;
    vector<int> delete_set;
    for(itrm = orNodeList.begin(); itrm != orNodeList.end(); itrm++)
	{
	    int count_inv = 0;
    	if(itrm->second.gateType == 5)
		{
		    int fanout_not_inv = 0;
		    if(itrm->second.pointFanOut.size() == 1)
		    {
		        itrv1 = itrm->second.pointFanOut.begin();
			    itrm1 = orNodeList.find((*itrv1)->lineNumber);
			    if(itrm1->second.gateType != 5)
			        fanout_not_inv = 1;
		    }
		    else if(itrm->second.pointFanOut.size() > 1)
		        fanout_not_inv = 1;
		      
		    if(fanout_not_inv == 0)
		        continue;
		    
		    count_inv++; 
		    vector<int> inv_set;   
		    inv_set.push_back(itrm->second.lineNumber);
		     
	        itrv2 = itrm->second.pointFanIn.begin();
			itrm2 = orNodeList.find((*itrv2)->lineNumber);  
			int  gate_type = itrm2->second.gateType;
			int current_node = itrm2->second.lineNumber;			
	        while(gate_type == 5)
			{
			//    redFILE1 << current_node << " ";
			    inv_set.push_back(current_node);
			    itrm3 = orNodeList.find(current_node);
			    itrv4 = itrm3->second.pointFanIn.begin();
			    itrm4 = orNodeList.find((*itrv4)->lineNumber);
			    gate_type = itrm4->second.gateType;
			    current_node = itrm4->second.lineNumber;
			    count_inv++;
			}
            
			if(count_inv % 2 == 0)
			    count_minus += count_inv;
			else
			    count_minus = count_minus + count_inv - 1;
			    
			if(inv_set.size() >= 2)
			{
			    if(inv_set.size() % 2 == 0)
			        delete_set.insert(delete_set.begin(), inv_set.begin(), inv_set.end());   
			    else
			    {
			        vector<int>::iterator iv;
			        iv = inv_set.begin();
			        inv_set.erase(iv);
			        delete_set.insert(delete_set.begin(), inv_set.begin(), inv_set.end());   
			    }
			}     
		}
    }
    
    for(int i = 0; i < delete_set.size(); i++)
    {
        int node = delete_set[i];
        itrm = orNodeList.find(node);
        if(itrm != orNodeList.end())
        {
            remove_inv(orNodeList, node);
            redFILE1 << "remove inv: "<<node<<endl;
        }
    }
    
    int final_size = orNodeList.size();
 //   int final_size = orNodeList.size() - count_minus;
    return final_size;
}


int main(int argc, char *argv[])
{
	//Containers for circuits
	map<int, CircuitNode> orNodeList_m;
	map<int, CircuitNode> brNodeList_m;
	map<int, CircuitNode> masterNodeList_m;
	
	//Iterators
	vector<CircuitNode>::iterator itrv;
	vector<CircuitNode*>::iterator itrv1;
	set<int>::iterator itrs;
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;

	//Check the number of arguments
    if (argc != 4) 
	{
       cerr << "ERROR: Usage: " << argv[0] << " <Circuit1 Filename> " << " <Circuit2 Filename> " << " <Circuit3 Filename> "<< endl;
       exit(1);
    }
    
    
    
//	redFILE0.open ("red_addition_v0.log", ofstream::app);
	redFILE1.open ("red_addition_v1.log", ofstream::app);
	logFile.open ("record.log", ofstream::app);
	
/*	string filename = argv[2];
	size_t pos = filename.find("brc");
	string subname = filename.substr(pos);
	int size = subname.size();
	int name_size = size - 4;
	string truename = subname.substr(0, name_size);
	cout << truename << endl;
	string type_name = argv[1];
    string subname1 = type_name.substr(15);
    size = subname1.size();
    name_size = size - 4;
    string truename1 = subname1.substr(0, name_size);
	cout << truename1 << endl;
	string catname = truename1;
	catname = catname.append("_");
	catname = catname.append(truename);
	cout << catname << endl;
	logFile << endl << catname<<endl;*/

    //Variables for time record
	struct timeb startTime, endTime;                         //Record the computing time.
	struct timeb startTime1, endTime1;                         //Record the computing time.
	struct timeb startTime2, endTime2;                         //Record the computing time.
	
	ftime(&startTime);
	   
	ftime(&startTime1);
	//Read the original circuit, the BR circuit and the BR pla file.
    ifstream inFile1, inFile2, inFile3, inFile4;
	string fileName1 = argv[1];
	inFile1.open(fileName1.c_str(), ios::in);
	if(!inFile1)
	{
		cerr << "Cannot open file "<<fileName1<<endl;
		exit(1);
	}
    ReadCircuit_v2(inFile1, orNodeList_m, inputList, outputList, 1);
	inFile1.close();
	int num_or = 0, num_and = 0, num_inv = 0;
	for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
	    if(itrm->second.gateType == 3)
	        num_or++;
	    else if(itrm->second.gateType == 7)
	        num_and++;
	    else if(itrm->second.gateType == 5)
	        num_inv++;
    redFILE1 << "0. gate number: "<<endl;	
	redFILE1 << "#AND: "<<num_and<<", #OR: "<<num_or<<", #INV: "<<num_inv<<endl;
	logFile << "0. gate number: "<<endl;	
	logFile << "#AND: "<<num_and<<", #OR: "<<num_or<<", #INV: "<<num_inv<<endl;
	cout << "#AND: "<<num_and<<", #OR: "<<num_or<<", #INV: "<<num_inv<<endl;
	
	print_circuit(orNodeList_m, redFILE1);
	
	string fileName2 = argv[2];
	inFile2.open(fileName2.c_str(), ios::in);
	if(!inFile2)
	{
		cerr << "Cannot open file "<<fileName2<<endl;
		exit(1);
	}
	vector<int> outputList_br;
    ReadCircuit_v2(inFile2, brNodeList_m, brInputList, outputList_br, 2);
	inFile2.close();
	
	ftime(&endTime1);
	double runtime_rc = ((endTime1.time - startTime1.time)*1000 + (endTime1.millitm - startTime1.millitm))/1000.0;


	numPI = inputList.size();
	numPO = outputList.size();

	redFILE1 <<"# PI: "<<numPI<<endl;
	redFILE1 <<"# PO: "<<numPO<<endl;

	redFILE1<< "Input list:"<<endl;
	for(int i=0; i<inputList.size(); i++)
		redFILE1<<inputList[i]<<" ";
	redFILE1<<endl;
	redFILE1<< "Output list:"<<endl;
	//HashTable opl_hash;
	opl_hash.Init_HashTable();
	for(int i=0; i<outputList.size(); i++)
	{
		redFILE1<<outputList[i]<<" ";
		int res = opl_hash.Insert_Hash(outputList[i]);
		while(res == 0)
			res = opl_hash.Insert_Hash(outputList[i]);
	}
	redFILE1<<endl<<endl;
	
	
	ftime(&startTime2);
	string br_file = argv[3];
	inFile3.open(br_file.c_str(), ios::in);
	if(!inFile3)
	{
		cerr << "Cannot open file "<<br_file<<endl;
		exit(1);
	}	
	
/*	DdManager *dd = NULL;
	DdNode *dnode = NULL;
	string dot_file = "dot.file";
	cout << "start building BDD:"<<endl;
	cudd_build(&dd, &dnode, br_file.c_str(), dot_file.c_str());
	cout << "print minterms:"<<endl;
	Cudd_PrintMinterm(dd, dnode);
    cudd_dst(dd);
	return 0;*/
		

	itrm = orNodeList_m.end();
	itrm--;
	int maxLineNumber = itrm->second.lineNumber;
	itrm = brNodeList_m.end();
	itrm--;
	maxLineNumber += itrm->second.lineNumber;
	maxLineNumber += 50;
	//redFILE1 << "maxlinenumber = "<<maxLineNumber<<endl;
	
	
	//add buffers
	for(int i = 0; i < outputList.size(); i++)
	{		
		itrm = orNodeList_m.find(outputList[i]);
		CircuitNode *addedNode;
		addedNode = new CircuitNode;
		if(itrm->second.gateType == 1)
			addedNode->nodeType = 2;   //FB
		else addedNode->nodeType = 0;   //GT
		addedNode->gateType = itrm->second.gateType;
		addedNode->lineNumber = maxLineNumber;
		addedNode->circuitIndex = 1;
		addedNode->rectFlag = 0;
		maxLineNumber += 1;
		orNodeList_m.insert(pair<int, CircuitNode>(addedNode->lineNumber, *addedNode));
		itrm1 = orNodeList_m.find(addedNode->lineNumber);
		for(itrv1 = itrm->second.pointFanIn.begin(); itrv1 != itrm->second.pointFanIn.end(); itrv1++)
		{
			itrm2 = orNodeList_m.find((*itrv1)->lineNumber);
			vector_erase(itrm2->second, itrm->second.lineNumber, 2);
			itrm2->second.pointFanOut.push_back(&itrm1->second);
			itrm1->second.listFanIn.insert(itrm2->second.lineNumber);
			itrm1->second.pointFanIn.push_back(&itrm2->second);			
		}
		
		orNodeList_m.erase(itrm);
		CircuitNode *addedNode_buf;
		addedNode_buf = new CircuitNode;
		addedNode_buf->nodeType = PO;
		addedNode_buf->gateType = 11;
		addedNode_buf->lineNumber = outputList[i];
		addedNode_buf->circuitIndex = 1;
		addedNode_buf->rectFlag = 0;
	    orNodeList_m.insert(pair<int, CircuitNode>(addedNode_buf->lineNumber, *addedNode_buf));
	    itrm3 = orNodeList_m.find(addedNode_buf->lineNumber);
	    itrm3->second.listFanIn.insert(itrm1->second.lineNumber);
	    itrm3->second.pointFanIn.push_back(&itrm1->second);
	    
	    itrm1->second.pointFanOut.push_back(&itrm3->second);
	    
	    delete addedNode;
	    delete addedNode_buf;
	}
	
	//Print original circuit.
	redFILE0<<"Original circuit in map: "<<orNodeList_m.size()<<" nodes"<<endl;
	print_circuit(orNodeList_m, redFILE0);	
	
	//Adjust line numbers of BR circuit and print the circuit.	
	adjust_ln(orNodeList_m, brNodeList_m, inputList, outputList);
	redFILE0<< endl << "BR circuit in map: "<<brNodeList_m.size()<<" nodes"<<endl;
	print_circuit(brNodeList_m, redFILE0);

	//obtain the input node of BR circuit.
	brInputList.clear();
	redFILE1 << "brInputList:"<<endl;
	for(itrm = brNodeList_m.begin(); itrm != brNodeList_m.end(); itrm++)
	{
		if(itrm->second.nodeType == 1)             //PI
		{ 
			brInputList.push_back(itrm->second.lineNumber);
			redFILE1 << itrm->second.lineNumber<< " ";
		}
	}
	redFILE1 << endl;
	
	//Merge the original circuit and the BR circuit into a whole circuit and print it.
	merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
	map<int, CircuitNode>::reverse_iterator itrm_rev;
	for(itrm_rev = masterNodeList_m.rbegin(); itrm_rev != masterNodeList_m.rend(); itrm_rev++)
	{
		if(itrm_rev->second.nodeType == 3)
		{
			PO_line = itrm_rev->second.lineNumber;
			break;
		}
	}
	redFILE1 << "PO_line = "<<PO_line<<endl;
	redFILE0<<"The whole circuit in map: "<<masterNodeList_m.size()<<" nodes"<<endl;
	print_circuit(masterNodeList_m, redFILE0);
	
/*	map<int, int>::iterator itrmi;
	for(int i = 0; i < outputList.size(); i++)
	{
	    int cnode = outputList[i];
	    map<int, int> cdom;
        find_dominator_main(brNodeList_m, cdom, cnode);
        redFILE1 << "dom for node "<<cnode<<":"<<endl;
        for(itrmi = cdom.begin(); itrmi != cdom.end(); itrmi++)
            redFILE1 << itrmi->first<<" ";
        redFILE1 << endl;
    }
    
    return 0;*/
    
	//create fault list
	 vector <FaultList> orFaultList;
	 vector <FaultList>::iterator itrvf;
	 create_fault_list(orNodeList_m, orFaultList);
	 
/*	//br_lisp
	vector<string> br_lisp_log;
	redFILE1 << "start br_lisp:"<<endl;
	br_lisp(brNodeList_m, masterNodeList_m, br_lisp_log);
	redFILE1 << "end of br_lisp"<<endl;*/
	
	//br_bench
	vector<string> br_bench_log;
	br_bench(brNodeList_m, masterNodeList_m, br_bench_log);
/*    write_bench(orNodeList_m, brNodeList_m, br_bench_log, oufile);
    call_abc();
    char tmp[100];
	sprintf(tmp, "minisat_static bench.cnf bench.out");
	system(tmp);*/
	
/*	map<int, CircuitNode> tmp_orNodeList;
	copy_point_vector1(orNodeList_m, tmp_orNodeList);
	simplify_circuit(tmp_orNodeList, 22, 1);
	check_red_sat_abc(tmp_orNodeList, brNodeList_m, br_bench_log, oufile);
	
	return 0;*/

	//step2. optimize the circuit by remove redundancies
	cout << "start rem_red_sat:"<<endl;
	ftime(&startTime1);	
	rem_red_sat(orNodeList_m, brNodeList_m, masterNodeList_m, orFaultList, br_bench_log);
	collapse_circuit(orNodeList_m);
    merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
    
    print_circuit(orNodeList_m, redFILE1);
    int inter_size = check_inv_chain(orNodeList_m);	
    print_circuit(orNodeList_m, redFILE1);
		
//	int inter_size = orNodeList_m.size();
	redFILE1 << "inter size = "<<inter_size<<endl;
	cout << "#inter size = "<<inter_size<<endl;
	logFile << "inter size = "<<inter_size<<endl;
	
	ftime(&endTime1);
	double runtime_rr = ((endTime1.time - startTime1.time)*1000 + (endTime1.millitm - startTime1.millitm))/1000.0;
	cout << "Time for remove redundancies: "<<runtime_rr<<endl<<endl;
	redFILE1 << "Time for remove redundancies: "<<runtime_rr<<endl<<endl;
	print_circuit(orNodeList_m, redFILE1);
	
	int isTestable_inter = check_red_sat_abc(orNodeList_m,brNodeList_m, br_bench_log, oufile); 
    if(!isTestable_inter)
    {
        redFILE1 << "The first stage has no error!"<<endl;
        cout << "The first stage has no error!"<<endl;
        logFile << "The first stage has no error!"<<endl;
    }
    else
    {
        redFILE1 << "The first stage has error!"<<endl;
        cout << "The first stage has error!"<<endl;
        logFile << "The first stage has error!"<<endl;
    }
	
//	return 0;
	

	//step3. optimize the circuit by rewiring
	redFILE1 << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;	
	redFILE1 << "Starting rar!"<<endl;
	ftime(&startTime1);
	opt_rar_sat(orNodeList_m, brNodeList_m, masterNodeList_m, orFaultList, br_bench_log);
	ftime(&endTime1);
	double runtime2 = ((endTime1.time - startTime1.time)*1000 + (endTime1.millitm - startTime1.millitm))/1000.0;
	collapse_circuit(orNodeList_m);
	
	
	cout << "Time for remove redundancies: "<<runtime_rr<<endl;
	cout << "Time for rewiring: "<<runtime2<<endl;
	redFILE1 << "Time for remove redundancies: "<<runtime_rr<<endl;
    redFILE1 << "Time for rewiring: "<<runtime2<<endl;
    logFile << "Time for remove redundancies: "<<runtime_rr<<endl;
    logFile << "Time for rewiring: "<<runtime2<<endl;

	
	ftime(&endTime);
	double runtime = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	logFile << "whole runtime = "<<runtime<<endl;	
	cout << "whole runtime = "<<runtime<<endl;
	
	int isTestable = check_red_sat_abc(orNodeList_m,brNodeList_m, br_bench_log, oufile); 
    if(!isTestable)
    {
        redFILE1 << "The whole precess has no error!"<<endl;
        cout << "The whole precess has no error!"<<endl;
        logFile << "The whole precess has no error!"<<endl;
    }
    else
    {
        redFILE1 << "The whole precess has error!"<<endl;
        cout << "The whole precess has error!"<<endl;
        logFile << "The whole precess has no error!"<<endl;
    }	
	
	
	int final_size = check_inv_chain(orNodeList_m);	
	
	redFILE1 << "Inter circuit size = "<<inter_size<<endl;
	redFILE1 << "Final circuit size = "<<final_size<<endl;	
	logFile << "Inter circuit size = "<<inter_size<<endl;
	logFile << "Final circuit size = "<<final_size<<endl;	
	cout << "Inter circuit size = "<<inter_size<<endl;
	cout << "Final circuit size = "<<final_size<<endl;
	
	
	
	print_circuit(orNodeList_m, redFILE1);
	num_or = 0, num_and = 0, num_inv = 0;
	for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
	    if(itrm->second.gateType == 3)
	        num_or++;
	    else if(itrm->second.gateType == 7)
	        num_and++;
	    else if(itrm->second.gateType == 5)
	        num_inv++;
	redFILE1 << "1. gate number:"<<endl;
	redFILE1 << "#AND: "<<num_and<<", #OR: "<<num_or<<", #INV: "<<num_inv<<endl;
	logFile << "1. gate number:"<<endl;
	logFile << "#AND: "<<num_and<<", #OR: "<<num_or<<", #INV: "<<num_inv<<endl;
	cout << "#AND: "<<num_and<<", #OR: "<<num_or<<", #INV: "<<num_inv<<endl;
	
    	
	write_bench_or(orNodeList_m, oufile, catname);

	
	
	redFILE0.close();
	redFILE1.close();
	record_hash.close();
	timeRecord.close();
	logFile.close();
	untFile.close();
	return 0;
	
}
