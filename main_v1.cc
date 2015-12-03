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
#include "function/run_fault_simulation.h"
#include "podem/podem_v2.h"


#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;


//global variables
int numPI, numPO, totalInputs;
vector<int> inputList;             //The list to store the primary inputs to the original circuit.
vector<int> outputList;            //The list to store the primary outputs to the original circuit.
HashTable opl_hash;
int PO_line;
vector<string> masterInputVector;
ofstream redFILE0, redFILE1, record_hash, timeRecord, logFile;


void rem_max_red(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &masterNodeList, vector <FaultList> &original_sf, multimap<int, FaultList> &cost_sf)
{
    if(cost_sf.empty())
        return;

    multimap<int, FaultList>::reverse_iterator itrmf_r;
	itrmf_r = cost_sf.rbegin(); 
	cout << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;
	redFILE1 << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;
	redFILE0 << endl << "("<<itrmf_r->second.lineNumber<<", "<<itrmf_r->second.stuckAtValue<<"): "<<endl;
	redFILE1 << "cost of current fault: "<<itrmf_r->first<<endl;

			
    struct timeb startTime, endTime;  
    ftime(&startTime);
    FaultList objFault(itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
    TestList test(itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
    int isTestable = podem(masterNodeList, objFault, test);
    ftime(&endTime);
	cout << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
	//logFile << "Time for podem: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl<<endl;
    if(isTestable)
    {
        redFILE1 << "Current fault is testable!"<<endl;
        multimap<int, FaultList>::iterator itrmf = cost_sf.end();
        itrmf--;
        cost_sf.erase(itrmf);
        //testPool.push_back(test.testVector);
        rem_max_red(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf);
    }        
    else
    {
        cout << "Untestable!"<<endl;
        redFILE1 << "Untestable!"<<endl;
		HashTable implicationNodeList;
		implicationNodeList.Init_HashTable();
		/*redundancy forward deletion */
		red_forward_deletion_v2(orNodeList, implicationNodeList, itrmf_r->second.lineNumber, itrmf_r->second.stuckAtValue);
		implicationNodeList.Destroy_HashTable();
		collapse_circuit(orNodeList);					
		redFILE1 << endl<< "After forward deletion, circuit size is "<<orNodeList.size()<<endl;
						
		/*redundancy backward deletion*/
		red_backward_deletion(orNodeList);
		collapse_circuit(orNodeList);
		redFILE1 << endl <<"After backward deletion, circuit size is "<<orNodeList.size()<<endl;	
		
		merge_circuit(masterNodeList, orNodeList, brNodeList);
	    //print_circuit(orNodeList, redFILE1);	
	    /*redFILE1 << "Run logic simulation on the whole circuit:"<<endl;
	    cout << "Run logic simulation on the whole circuit:"<<endl;
	    RunLogicSimulation(orNodeList, brNodeList, masterNodeList, totalInputs, masterInputVector);*/
		
		cost_sf.clear();
		compute_cost(orNodeList, masterNodeList, original_sf, cost_sf);		
		rem_max_red(orNodeList, brNodeList, masterNodeList, original_sf, cost_sf);			
    }
}



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
    if (argc != 3) 
	{
       cerr << "ERROR: Usage: " << argv[0] << " <Circuit1 Filename> " << " <Circuit1 Filename> " << endl;
       exit(1);
    }

	redFILE0.open ("red_addition.log", ofstream::app);
	redFILE1.open ("red_addition1.log", ofstream::app);
	timeRecord.open("time.log", ofstream::app);
	//record_hash.open("hash_file.log", ofstream::out);
	logFile.open("record.log", ofstream::app);
	   
	//Read the original circuit and the BR circuit
    ifstream inFile1, inFile2;
	string fileName1 = argv[1];
	inFile1.open(fileName1.c_str(), ios::out);
	if(!inFile1)
	{
		cerr << "Cannot open file "<<fileName1<<endl;
		exit(1);
	}
    ReadCircuit(inFile1, orNodeList, 1);
	inFile1.close();

	string fileName2 = argv[2];
	inFile2.open(fileName2.c_str(), ios::out);
	if(!inFile2)
	{
		cerr << "Cannot open file "<<fileName2<<endl;
		exit(1);
	}
    ReadCircuit(inFile2, brNodeList, 2);
	inFile2.close();

    
	//Record the number of primary inputs and primary outputs.
	//Put lineNumber of PIs and POs into inputList and outputList.
	numPI = 0; numPO = 0;
	for(int i=0; i<orNodeList.size(); i++)
	{
		if(orNodeList[i].nodeType == 1)             //PI
		{
			numPI+=1; 
			inputList.push_back(orNodeList[i].lineNumber);
		}
		if(orNodeList[i].nodeType == 3)             //PO
		{
			numPO+=1;
			outputList.push_back(orNodeList[i].lineNumber);
		}
	}
	redFILE0 <<"# PI: "<<numPI<<endl;
	redFILE0 <<"# PO: "<<numPO<<endl;

	redFILE0<< "Input list:"<<endl;
	for(int i=0; i<inputList.size(); i++)
		redFILE0<<inputList[i]<<" ";
	redFILE0<<endl;
	redFILE0<< "Output list:"<<endl;
	//HashTable opl_hash;
	opl_hash.Init_HashTable();
	for(int i=0; i<outputList.size(); i++)
	{
		redFILE0<<outputList[i]<<" ";
		int res = opl_hash.Insert_Hash(outputList[i]);
		while(res == 0)
			res = opl_hash.Insert_Hash(outputList[i]);
	}
	redFILE0<<endl<<endl;
	

	//Copy circuit in vector to map and print them in file "red_addition.log"
	copy_point_vector2(orNodeList, orNodeList_m);
	copy_point_vector2(brNodeList, brNodeList_m);
	
	itrm = orNodeList_m.end();
	itrm--;
	int maxLineNumber = itrm->second.lineNumber;
	itrm = brNodeList_m.end();
	itrm--;
	maxLineNumber += itrm->second.lineNumber;
	maxLineNumber += 50;
	//redFILE1 << "maxlinenumber = "<<maxLineNumber<<endl;
	
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
	
	//Backup the three initial circuits.
	map<int, CircuitNode> buOrNodeList_m, buBrNodeList_m, buMasterNodeList_m;	
	copy_point_vector1(orNodeList_m, buOrNodeList_m);
	copy_point_vector1(brNodeList_m, buBrNodeList_m);
	copy_point_vector1(masterNodeList_m, buMasterNodeList_m);
	
	//Print original circuit.
	redFILE0<<"Original circuit in map: "<<orNodeList_m.size()<<" nodes"<<endl;
	print_circuit(orNodeList_m, redFILE0);
	
	//Adjust line numbers of BR circuit and print the circuit.
	adjust_ln(orNodeList_m, brNodeList_m, inputList, outputList);
	redFILE0<< endl << "BR circuit in map: "<<brNodeList_m.size()<<" nodes"<<endl;
	print_circuit(brNodeList_m, redFILE0);
	
	
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
	
	totalInputs = inputList.size();
	GenerateMasterInputVectors(masterInputVector, totalInputs);	
	//for(int i = 0; i < masterInputVector.size(); i++)
	//    redFILE1 << masterInputVector[i]<<endl;
	    
	for(itrm = masterNodeList_m.begin(); itrm != masterNodeList_m.end(); itrm++)
		masterNodeList.push_back(itrm->second);
	copy_point_vector3(masterNodeList_m, masterNodeList);
	
	//Generate all input vectors for logic simulation.
	totalInputs = inputList.size();
	redFILE0 << "totalInputs = "<<totalInputs<<endl;
	
	
	//Variables for time record
	struct timeb startTime, endTime;                         //Record the computing time.
	struct timeb startTime1, endTime1;                         //Record the computing time.
	
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
		int true_ans = podem(masterNodeList_m, *(itrv_fault), test);
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
	
	/*optimize the circuit by redundancy identification and removal*/
	ftime(&startTime1);
	
	map <int, CircuitLine> masterLineList_m;
	map <int, CircuitLine>::iterator itrml;
	vector <FaultList> orFaultList;
	masterLineList_m.clear();
	CreateFaultObjects(masterLineList_m, masterNodeList_m);
	CreateFaultList(masterLineList_m, orFaultList, masterNodeList_m);
	redFILE0 << "before collapse, #fault: "<<2*masterLineList_m.size()<<endl;
	redFILE0 << "Faults in original_sf: "<<orFaultList.size()<<endl;
	for(int j=0; j<orFaultList.size(); j++)
		redFILE0 << "("<<orFaultList[j].lineNumber <<","<<orFaultList[j].stuckAtValue<<"), ";
	redFILE0 <<endl; 
	
	CollapseFaults(masterLineList_m, masterNodeList_m);
	int count = 0;
	for(itrml = masterLineList_m.begin(); itrml != masterLineList_m.end(); itrml++)
	{
		if(itrml->second.isStuckAt_0) count++;
		if(itrml->second.isStuckAt_1) count++;
	}
	redFILE0 << "after collapse, #fault: "<<count<<endl;
	orFaultList.clear();
	CreateFaultList(masterLineList_m, orFaultList,masterNodeList_m);
	
	redFILE0 << "Faults in original_sf: "<<orFaultList.size()<<endl;
	for(int j=0; j<orFaultList.size(); j++)
		redFILE0 << "("<<orFaultList[j].lineNumber <<","<<orFaultList[j].stuckAtValue<<"), ";
	redFILE0 <<endl; 
	
	multimap<int, FaultList> cost_sf;
	compute_cost(orNodeList_m, masterNodeList_m, orFaultList, cost_sf);
	
	int ini_size = orNodeList_m.size();
	rem_max_red(orNodeList_m, brNodeList_m, masterNodeList_m, orFaultList, cost_sf);
	
	merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
	print_circuit(orNodeList_m, redFILE1);	
	redFILE1 << "Run logic simulation on the whole circuit:"<<endl;
	cout << "Run logic simulation on the whole circuit:"<<endl;
	RunLogicSimulation(orNodeList_m, brNodeList_m, masterNodeList_m, totalInputs, masterInputVector);

    redFILE1 << "ini_size = "<<ini_size<<endl;
	cout << "ini_size = "<<ini_size<<endl;	
	int new_size = 	orNodeList_m.size();
	redFILE1 << "new_size = "<<new_size<<endl;	
	logFile << "new_size = "<<new_size<<endl;	
	cout << "new_size = "<<new_size<<endl;	
	
	ftime(&endTime1);
	cout << "Time for optimize: "<<((endTime1.time - startTime1.time)*1000 + (endTime1.millitm - startTime1.millitm))/1000.0<<endl<<endl;

	
	redFILE0.close();
	redFILE1.close();
	logFile.close();
	return 0;
	
}

	
