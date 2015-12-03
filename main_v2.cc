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
#include "function/read_circuit.h"
#include "function/print_circuit.h"
#include "function/copy_point_vector.h"
#include "function/atpg.h"
#include "function/adjust_ln.h"
#include "function/merge_circuit.h"
#include "function/helper.h"
#include "function/run_logic_simulation.h"
#include "function/red_forward_deletion_v2.h"
#include "function/red_backward_deletion.h"
#include "function/ATPG_checker.h"
#include "function/parallel_test_simulation.h"
#include "function/run_fault_simulation.h"
#include "function/rem_red.h"
#include "podem/podem_v2.h"
#include "dac/opt_dac.h"
#include "dac/find_MA.h"


#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;


//global variables
int numPI, numPO, totalInputs;
vector<int> inputList, inputList_br;             //The list to store the primary inputs to the original circuit.
vector<int> outputList;            //The list to store the primary outputs to the original circuit.
HashTable opl_hash;
int PO_line;
vector<string> masterInputVector;
ofstream redFILE0, redFILE1, record_hash, timeRecord, logFile;



int main(int argc, char *argv[])
{
	//Containers for circuits
	vector<CircuitNode> orNodeList;
	map<int, CircuitNode> orNodeList_m;
	vector<CircuitNode> brNodeList;
	map<int, CircuitNode> brNodeList_m;
	vector<CircuitNode> masterNodeList;
	map<int, CircuitNode> masterNodeList_m;
	
	//Iterators
	vector<CircuitNode>::iterator itrv;
	vector<CircuitNode*>::iterator itrv1;
	set<int>::iterator itrs;
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;

	//Check the number of arguments
    if (argc != 4) 
	{
       cerr << "ERROR: Usage: " << argv[0] << " <Circuit1 Filename> " << " <Circuit2 Filename> " << " <Circuit3 Filename> "<<endl;
       exit(1);
    }

	redFILE0.open ("red_addition.log", ofstream::app);
	redFILE1.open ("red_addition1.log", ofstream::app);
	timeRecord.open("time.log", ofstream::app);
	//record_hash.open("hash_file.log", ofstream::out);
	logFile.open("record.log", ofstream::app);
	
    //Variables for time record
	struct timeb startTime, endTime;                         //Record the computing time.
	struct timeb startTime1, endTime1;                         //Record the computing time.
	struct timeb startTime2, endTime2;                         //Record the computing time.
	
	ftime(&startTime);
	   
	ftime(&startTime1);
	//Read the original circuit, the BR circuit and the BR pla file.
    ifstream inFile1, inFile2, inFile3;
	string fileName1 = argv[1];
	inFile1.open(fileName1.c_str(), ios::in);
	if(!inFile1)
	{
		cerr << "Cannot open file "<<fileName1<<endl;
		exit(1);
	}
    ReadCircuit(inFile1, orNodeList, 1);
	inFile1.close();

	string fileName2 = argv[2];
	inFile2.open(fileName2.c_str(), ios::in);
	if(!inFile2)
	{
		cerr << "Cannot open file "<<fileName2<<endl;
		exit(1);
	}
    ReadCircuit(inFile2, brNodeList, 2);
	inFile2.close();
	
	ftime(&startTime1);
	double runtime_rc = ((endTime1.time - startTime1.time)*1000 + (endTime1.millitm - startTime1.millitm))/1000.0;
	cout << "Time for reading circuit files: "<<runtime_rc<<endl<<endl;
	logFile << "runtime for reading circuit files = "<<runtime_rc<<endl;
	
	ftime(&startTime1);
	
	//Copy circuit in vector to map and print them in file "red_addition.log"
	copy_point_vector2(orNodeList, orNodeList_m);
	copy_point_vector2(brNodeList, brNodeList_m);
	
    //Record the number of primary inputs and primary outputs.
	//Put lineNumber of PIs and POs into inputList and outputList.
	numPI = 0; numPO = 0;
	for(itrm = orNodeList_m.begin(); itrm != orNodeList_m.end(); itrm++)
	{
		if(itrm->second.nodeType == 1)             //PI
		{
			numPI+=1; 
			inputList.push_back(itrm->second.lineNumber);
		}
		if(itrm->second.nodeType == 3)             //PO
		{
			numPO+=1;
			outputList.push_back(itrm->second.lineNumber);
		}
	}
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
	double runtime_record = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
	cout << "Time for recording: "<<runtime_record<<endl<<endl;
	logFile << "runtime for recording = "<<runtime_record<<endl;
	
	
	ftime(&startTime2);
	
	map<int, int> truthTable;
	string br_file = argv[3];
	inFile3.open(br_file.c_str(), ios::in);
	if(!inFile3)
	{
		cerr << "Cannot open file "<<br_file<<endl;
		exit(1);
	}	
	string s;
	while(inFile3 >> s)
	{
	    //redFILE1 << "s = "<<s<<":"<<s.size()<<endl;
		if(s.size() < (numPI+numPO))
			continue;
		else
		{
		    //redFILE1 << "s = "<<s<<endl;
		    int int_value = 0;
		    for(int j = 0; j < s.size(); j++)
			{
			    int_value = int_value + (s[j] - 48) * pow(2, s.size()-1-j);

			}	
			//redFILE1 << "truthtable's index: "<<int_value<<" ";
			truthTable.insert(pair<int, int>(int_value, 1));
		}
		inFile3>>s;                                   //read the last "1"
	}
	redFILE1 << endl;

	ftime(&endTime2);
	double runtime0 = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
	cout << "Time for reading truth table: "<<runtime0<<endl<<endl;
	logFile << "runtime for reading truth table = "<<runtime0<<endl;
	
	itrm = orNodeList_m.end();
	itrm--;
	int maxLineNumber = itrm->second.lineNumber;
	itrm = brNodeList_m.end();
	itrm--;
	maxLineNumber += itrm->second.lineNumber;
	maxLineNumber += 50;
	//redFILE1 << "maxlinenumber = "<<maxLineNumber<<endl;
	
	ftime(&startTime2);
	
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
	    
	    delete addedNode, addedNode_buf;
	}
	
	ftime(&endTime2);
	double runtime_add = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
	cout << "Time for add buffers: "<<runtime_add<<endl<<endl;
	logFile << "runtime for add buffers = "<<runtime_add<<endl;
	
	//Print original circuit.
	redFILE0<<"Original circuit in map: "<<orNodeList_m.size()<<" nodes"<<endl;
	print_circuit(orNodeList_m, redFILE0);
	
	
	ftime(&startTime2);
	
	//Adjust line numbers of BR circuit and print the circuit.
	adjust_ln(orNodeList_m, brNodeList_m, inputList, outputList);
	redFILE0<< endl << "BR circuit in map: "<<brNodeList_m.size()<<" nodes"<<endl;
	print_circuit(brNodeList_m, redFILE0);
	
	ftime(&endTime2);
	double runtime_adjust = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
	cout << "Time for adjust: "<<runtime_adjust<<endl<<endl;
	logFile << "runtime for adjust = "<<runtime_adjust<<endl;
	
	ftime(&startTime2);
	
	//obtain the input node of BR circuit.
	for(itrm = brNodeList_m.begin(); itrm != brNodeList_m.end(); itrm++)
	{
		if(itrm->second.nodeType == 1)             //PI
		{ 
			inputList_br.push_back(itrm->second.lineNumber);
		}
	}
	
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
	//redFILE1 << "PO_line = "<<PO_line<<endl;
	redFILE0<<"The whole circuit in map: "<<masterNodeList_m.size()<<" nodes"<<endl;
	print_circuit(masterNodeList_m, redFILE0);
	
	ftime(&endTime2);
	double runtime_merge = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
	cout << "Time for merge: "<<runtime_merge<<endl<<endl;
	logFile << "runtime for merge = "<<runtime_merge<<endl;
	
	ftime(&startTime2);
	
	//Generate all input vectors for logic simulation.
	totalInputs = inputList.size();
	redFILE0 << "totalInputs = "<<totalInputs<<endl;
	GenerateMasterInputVectors(masterInputVector, totalInputs);	
	
	ftime(&endTime2);
	double runtime_gen = ((endTime2.time - startTime2.time)*1000 + (endTime2.millitm - startTime2.millitm))/1000.0;
	cout << "Time for gen: "<<runtime_gen<<endl<<endl;
	logFile << "runtime for gen = "<<runtime_gen<<endl;

	for(itrm = masterNodeList_m.begin(); itrm != masterNodeList_m.end(); itrm++)
		masterNodeList.push_back(itrm->second);
	copy_point_vector3(masterNodeList_m, masterNodeList);
	

	
	
	/**********************************/
	/*redundancy identification starts*/
	/**********************************/
/*	ftime(&startTime);
	
    map<int, CircuitLine> masterLineList_m;
	map<int, CircuitLine>::iterator itrml;
	vector <FaultList> orFaultList;
	masterLineList_m.clear();
	CreateFaultObjects(masterLineList_m, masterNodeList_m);
	CreateFaultList(masterLineList_m, orFaultList, masterNodeList_m);
	redFILE1 << "before collapse, #fault: "<<2*masterLineList_m.size()<<endl;
	redFILE1 << "Faults in original_sf: "<<orFaultList.size()<<endl;
	for(int j=0; j<orFaultList.size(); j++)
		redFILE1 << "("<<orFaultList[j].lineNumber <<","<<orFaultList[j].stuckAtValue<<"), ";
	redFILE1 <<endl; 
	
	CollapseFaults(masterLineList_m, masterNodeList_m);
	int count = 0;
	for(itrml = masterLineList_m.begin(); itrml != masterLineList_m.end(); itrml++)
	{
		if(itrml->second.isStuckAt_0) count++;
		if(itrml->second.isStuckAt_1) count++;
	}
	redFILE1 << "after collapse, #fault: "<<count<<endl;
	orFaultList.clear();
	CreateFaultList(masterLineList_m, orFaultList,masterNodeList_m);
	redFILE1 << "Faults in original_sf: "<<orFaultList.size()<<endl;
	for(int j=0; j<orFaultList.size(); j++)
		redFILE1 << "("<<orFaultList[j].lineNumber <<","<<orFaultList[j].stuckAtValue<<"), ";
	redFILE1 <<endl; 
	
	int num_unt = 0;
	vector<FaultList>::iterator itrv_fault;
	while(!orFaultList.empty())
	{
	    itrv_fault = orFaultList.begin();
	    int line = (*itrv_fault).lineNumber;
	    int sa_value = (*itrv_fault).stuckAtValue;
	    cout << "("<<line<<", "<<sa_value<<")"<<endl;
	    redFILE1 << "---------------------------------"<<endl;
		redFILE1 << "("<<line<<", "<<sa_value<<")"<<endl;
		TestList test(line, sa_value);
		int true_ans = podem(masterNodeList_m, *(itrv_fault), test, truthTable);
		//bool true_ans = TestAllVectorsATPG (masterNodeList_m, totalInputs, line, sa_value, masterInputVector);
	    
	    orFaultList.erase(itrv_fault);
		if(true_ans == true)
		{
		    cout << "*current fault is testable!"<<endl;
		    redFILE1 << "*current fault is testable!"<<endl;
		    redFILE1 << "test vector = "<<test.testVector<<endl;

		    vector<FaultList> leftFaultList;
		    run_fault_simulation(masterNodeList_m, orFaultList, leftFaultList, test.testVector); 
		    redFILE1 << "Faults in leftFaultList: "<<leftFaultList.size()<<endl;
	        for(int j=0; j<leftFaultList.size(); j++)
		        redFILE1 << "("<<leftFaultList[j].lineNumber <<","<<leftFaultList[j].stuckAtValue<<"), ";
	        redFILE1 <<endl; 
	        orFaultList.clear();
	        orFaultList = leftFaultList;
		}
		else
		{
		    cout << "*current fault is untestable!"<<endl;
		    //redFILE1 << "("<<itrm->second.lineNumber<<", 0)"<<endl;
		    redFILE1 << "*current fault is untestable!"<<endl;
		    num_unt++;
		}
    }
    cout << "#unt = "<<num_unt<<endl;
      
    ftime(&endTime);
	cout << "Time for redundancy identification: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;*/
	
	
	
	/*step1. optimize the circuit by remove redundancies*/
	ftime(&startTime1);
	
	rem_red(orNodeList_m, brNodeList_m, masterNodeList_m, truthTable);
	collapse_circuit(orNodeList_m);
	merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
		
	int inter_size = orNodeList_m.size();
	redFILE1 << "#line = "<<inter_size<<endl;
	cout << "#line = "<<inter_size<<endl;
	
	//redFILE1 << "circuit after first step: "<<inter_size<<endl;
	//print_circuit(orNodeList_m, redFILE1);	
		
	ftime(&endTime1);
	double runtime1 = ((endTime1.time - startTime1.time)*1000 + (endTime1.millitm - startTime1.millitm))/1000.0;
	cout << "Time for remove redundancies: "<<runtime1<<endl<<endl;
	

	
	/*step2. optimize the circuit by rewiring*/
	redFILE1 << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;	
	redFILE1 << "Starting dac!"<<endl;
	ftime(&startTime1);
	//opt_dac(orNodeList_m, brNodeList_m, masterNodeList_m, truthTable);
	ftime(&endTime1);
	collapse_circuit(orNodeList_m);
	double runtime2 = ((endTime1.time - startTime1.time)*1000 + (endTime1.millitm - startTime1.millitm))/1000.0;
	cout << "Time for rewiring: "<<runtime2<<endl<<endl;
	
	ftime(&endTime);
	double runtime = ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0;
	
	print_circuit(orNodeList_m, redFILE1);	
	redFILE1 << "Run logic simulation on the whole circuit:"<<endl;
	cout << "Run logic simulation on the whole circuit:"<<endl;
	RunLogicSimulation(orNodeList_m, totalInputs, masterInputVector, truthTable);
	
	redFILE1 << "Final circuit size = "<<orNodeList_m.size()<<endl;
	redFILE1 << "Inter circuit size = "<<inter_size<<endl;
	cout << "Inter circuit size = "<<inter_size<<endl;
	cout << "Final circuit size = "<<orNodeList_m.size()<<endl;
	logFile << "Final size = "<<orNodeList_m.size()<<endl;
	logFile << "Inter circuit size = "<<inter_size<<endl;
    
	logFile << "runtime for remove redundanices = "<<runtime1<<endl;
	logFile << "runtime for rewiring = "<<runtime2<<endl;	
	logFile << "whole runtime = "<<runtime<<endl;	
	cout << "whole runtime = "<<runtime<<endl;	
	
	redFILE0.close();
	redFILE1.close();
	logFile.close();
	return 0;
	
}

	
