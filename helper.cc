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
#include <cassert>
#include <ctime>
#include <sys/timeb.h>

#include "class/CircuitNode.h"         
#include "class/CircuitLine.h"         
#include "class/FaultList.h"          
#include "class/TestList.h"            
#include "class/queue.h"
#include "lib/forward_implication.h"
#include "lib/file_operations.h"
#include "lib/radix_convert.h"
#include "class/HashTable.h"
#include "function/copy_point_vector.h"
#include "function/red_forward_deletion_v2.h"
#include "function/red_backward_deletion.h"
#include "function/print_circuit.h"
#include "function/atpg.h"
#include "function/helper.h"
#include "function/write_lisp.h"
#include "function/idtf_unt.h"
#include "function/write_bench.h"
#include "function/call_abc.h"


using namespace std;

extern HashTable opl_hash;
extern int PO_line;
extern ofstream redFILE1, redFILE0;
extern vector<int> inputList;
extern vector<int> outputList;

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

//GenerateMasterInputVectors()
void GenerateMasterInputVectors(vector<string> &masterInputVector, int totalInputs) 
{  
    string resultVector;                   // Input vector.
    vector<string> *tempInputVector;      // We have to sort vectors by number of X in them.
    tempInputVector = new vector <string> [totalInputs];
    vector <string>::iterator itrVector;
    
/*    int num_appr = 20;
    int totalTruth = pow(2, totalInputs);
    HashTable index_appr;
	index_appr.Init_HashTable();
	srand((unsigned)time(0));
	while(index_appr.Data_Size() < num_appr)
	{		
		unsigned int randIndex = rand() % totalTruth + 1;
		int res = index_appr.Insert_Hash(randIndex);
		while(res == 0)
			res = index_appr.Insert_Hash(randIndex);
	}

    for(int i = 0; i < index_appr.Get_Size(); i++)
	{
		int index = index_appr.Get_Data(i);
		if(index == -1 || index == -2)
			continue;*/

    for (int i = 0; i < (pow(2, totalInputs)); i++) 
    {   
        resultVector = RadixConvert (i, 2);     // Go on converting numbers in base 2 system.
        int numberOfZero = 0;                                  // Total X in the vector.
        int positionOfZero = resultVector.find_first_of('0');  // Iterate over the vector.
        while (positionOfZero!=string::npos)
        {
            numberOfZero++;                                    // Count number of 'X's
            positionOfZero = resultVector.find_first_of('0',(positionOfZero + 1));
        }
        tempInputVector[numberOfZero].push_back(resultVector);
    }

    // Now we have to put all the vectors in a large table.
    // There are no leading zeros in the string. We need them
    // in our simulation so padding is also done.
    for (int i = 0; i < totalInputs; i++) {
        for (itrVector = tempInputVector[i].begin(); itrVector != tempInputVector[i].end(); itrVector++) {
            resultVector = *itrVector;
            int vectorLength = resultVector.length();
	        for (int j = 0; j < (totalInputs - vectorLength); j++)
		        resultVector = "0" + resultVector;
            masterInputVector.push_back (resultVector);
        }
    }

    // We start from the case with most 'X's and go to the case with minimum 'X's.
    // For this we have to reverse the vector.
    //reverse (masterInputVector.begin(), masterInputVector.end());
    
    delete []tempInputVector;
}


void GenerateMasterInputVectors_some(vector<string> &masterInputVector, int totalInputs) 
{  
    string resultVector;                   // Input vector.
    vector<string> *tempInputVector;      // We have to sort vectors by number of X in them.
    tempInputVector = new vector <string> [totalInputs];
    vector <string>::iterator itrVector;
    
	for (int i = 0; i < 100; i++) 		
    {   
        resultVector = RadixConvert (i, 2);     // Go on converting numbers in base 2 system.
        int numberOfZero = 0;                                  // Total X in the vector.
        int positionOfZero = resultVector.find_first_of('0');  // Iterate over the vector.
        while (positionOfZero!=string::npos)
        {
            numberOfZero++;                                    // Count number of 'X's
            positionOfZero = resultVector.find_first_of('0',(positionOfZero + 1));
        }
        tempInputVector[numberOfZero].push_back(resultVector);
    }

    // Now we have to put all the vectors in a large table.
    // There are no leading zeros in the string. We need them
    // in our simulation so padding is also done.
    for (int i = 0; i < totalInputs; i++) {
        for (itrVector = tempInputVector[i].begin(); itrVector != tempInputVector[i].end(); itrVector++) {
            resultVector = *itrVector;
            int vectorLength = resultVector.length();
	        for (int j = 0; j < (totalInputs - vectorLength); j++)
		        resultVector = "0" + resultVector;
            masterInputVector.push_back (resultVector);
        }
    }

    // We start from the case with most 'X's and go to the case with minimum 'X's.
    // For this we have to reverse the vector.
    //reverse (masterInputVector.begin(), masterInputVector.end());
    
    delete []tempInputVector;
}




//vector_erase()
void vector_erase(CircuitNode &node, int line, int flag)
{
	vector<CircuitNode*>::iterator itrv;

	if(flag == 1)                     //pointFanIn
	{
		//redFILE0 << "Node "<<node.lineNumber << " erases its input node "<<line<<endl;
		for(itrv = node.pointFanIn.begin(); itrv != node.pointFanIn.end(); itrv++)
			if((*itrv)->lineNumber == line)
			{
				node.pointFanIn.erase(itrv);
				break;
			}
	}
	else if(flag == 2)                //pointFanOut
	{
		//redFILE0 << "Node "<<node.lineNumber << " erases its output node "<<line<<endl;
		for(itrv = node.pointFanOut.begin(); itrv != node.pointFanOut.end(); itrv++)
			if((*itrv)->lineNumber == line)
			{
				node.pointFanOut.erase(itrv);
				break;
			}
	}
}


//vector_insert()
void vector_insert(CircuitNode &node, int line, int flag, map<int, CircuitNode> &nodeList)
{
	vector<CircuitNode*>::iterator itrv;
	map<int, CircuitNode>::iterator itrm;

	if(flag == 1)                     //pointFanIn
	{
		//redFILE0 << "Node "<<node.lineNumber << " inserts an input node "<<line<<endl;
		itrm = nodeList.find(line);
		node.pointFanIn.push_back(&itrm->second);
	}
	else if(flag == 2)                //pointFanOut
	{
		//redFILE0 << "Node "<<node.lineNumber << " inserts an output node "<<line<<endl;
		itrm = nodeList.find(line);
		node.pointFanOut.push_back(&itrm->second);
	}
}


//search_vector_int()
int search_vector_int(vector<int> &NodeList, int node)
{
	int i;
	for(i=0; i<NodeList.size(); i++)
		if(NodeList[i] == node)
		{
			return i;
			break;
		}
	return -1;
}


//search_set_int()
int search_set_int(set<int> &NodeList, int node)
{
	set<int>::iterator itrs;
	int i = 0;
	for(itrs = NodeList.begin(); itrs != NodeList.end(); itrs++, i++)
		if(*itrs == node)
		{
			return i;
			break;
		}
	return -1;
}

//search_vector_node()
int search_vector_node(vector<CircuitNode> &masterNodeList, int line)
{
	vector<CircuitNode>::iterator itrv;
	int i = 0;
	for(itrv = masterNodeList.begin(); itrv != masterNodeList.end(); itrv++, i++)
		if((*itrv).lineNumber == line)
			return i;
	return -1;
}



//search_index_fault()
void search_vector_fault(vector<FaultList> &NodeList, int node, vector<int> &flagVec)
{
	int i;
	for(i=0; i<NodeList.size(); i++)
		if(NodeList[i].lineNumber == node)
		{
			flagVec[i] = 1;
		}
}


//collapse_circuit()
void collapse_circuit(map<int, CircuitNode> &masterNodeList)
{
	//redFILE1 << "Enter into collapse_circuit!"<<endl;

	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3;
	set <int>::iterator itrs, itrs1;
	vector <CircuitNode*>::iterator itrv, itrv1, itrv2, itrv3;

	vector <int> del_list, del_list_inv;
	int num_fanout = 0;
	for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
		if(search_vector_int(outputList, itrm->second.lineNumber) != -1)
			continue;
		if(itrm->second.nodeType == 2 || (itrm->second.nodeType == 0 && itrm->second.gateType == 1))                        //Branch
		{
			itrv = itrm->second.pointFanIn.begin();
			itrm1 = masterNodeList.find((*itrv)->lineNumber);
			num_fanout = itrm1->second.pointFanOut.size();
			if(num_fanout == 1)
			{				
				del_list.push_back(itrm->first);
												
				itrm1->second.pointFanOut.clear();
				itrm1->second.pointFanOut = itrm->second.pointFanOut;
		
				for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++)
				{
					itrm2 = masterNodeList.find((*itrv)->lineNumber);
					itrm2->second.listFanIn.erase(itrm->second.lineNumber);
					itrm2->second.listFanIn.insert(itrm1->second.lineNumber);
					vector_erase(itrm2->second, itrm->second.lineNumber, 1);
					vector_insert(itrm2->second, itrm1->second.lineNumber, 1, masterNodeList);
				}
			}			
		}
	}
	
    for(int i = 0; i < del_list.size(); i++)
	{
		//redFILE1 << "Node "<<del_list[i] <<" is deleted due to collapse."<<endl;
		masterNodeList.erase(del_list[i]);
	}

/*    for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
    	if(itrm->second.gateType == 5)
		{
		    if(itrm->second.pointFanOut.size() == 1)
			{
			   // redFILE1 << "current node is "<<itrm->second.lineNumber<<endl;
			    itrv = itrm->second.pointFanOut.begin();
			    itrm1 = masterNodeList.find((*itrv)->lineNumber);
			    if(itrm1->second.gateType == 5 )
			    {			        
			        itrv1 = itrm->second.pointFanIn.begin();
			        itrm2 = masterNodeList.find((*itrv1)->lineNumber);
			        del_list_inv.push_back(itrm2->second.lineNumber);
			        redFILE1 << "first inverter: "<<itrm->second.lineNumber<<endl;
			        redFILE1 << "second inverter: "<<itrm1->second.lineNumber<<endl;
			        redFILE1 << "Node "<<itrm2->second.lineNumber<<" is pushed into delete list."<<endl;
			       
			    }
			}
		}
    }

	for(int i = 0; i < del_list_inv.size(); i++)
	{
		itrm = masterNodeList.find(del_list_inv[i]);
        itrv = itrm->second.pointFanOut.begin();
		itrm1 = masterNodeList.find((*itrv)->lineNumber);
		itrv1 = itrm1->second.pointFanOut.begin();
		itrm2 = masterNodeList.find((*itrv1)->lineNumber);
		if(itrm1->second.gateType == 5 && itrm2->second.gateType == 5)
        {
            itrm->second.pointFanOut.clear();
			for(itrv3 = itrm2->second.pointFanOut.begin(); itrv3 != itrm2->second.pointFanOut.end(); itrv3++)
			{
			    itrm3 = masterNodeList.find((*itrv3)->lineNumber);
				itrm3->second.listFanIn.erase(itrm2->second.lineNumber);
				itrm3->second.listFanIn.insert(itrm->second.lineNumber);
				vector_erase(itrm3->second, itrm2->second.lineNumber, 1);
				vector_insert(itrm3->second, itrm->second.lineNumber, 1, masterNodeList);
				itrm->second.pointFanOut.push_back(&itrm3->second);
			}
			masterNodeList.erase(itrm1);
			masterNodeList.erase(itrm2);
			redFILE1 << "Node "<<itrm1->second.lineNumber<<" and node "<<itrm2->second.lineNumber<<" is deleted."<<endl;
        }
	}
	*/

}

void collapse_circuit_v2(map<int, CircuitNode> &masterNodeList, int node)
{
	map<int, CircuitNode>::iterator itrm, itrm1;
	set <int>::iterator itrs, itrs1;
	vector <CircuitNode*>::iterator itrv, itrv1;

	
	itrm = masterNodeList.find(node);
		for(itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++)
		{
			itrm1 = masterNodeList.find((*itrv)->lineNumber);
			if(itrm1 == masterNodeList.end())
				continue;
			vector_erase(itrm1->second, itrm->second.lineNumber, 2);
			masterNodeList.erase(itrm);
			if(itrm1->second.pointFanOut.empty() && itrm1->second.nodeType != PO && itrm->second.nodeType != PI)
				collapse_circuit_v2(masterNodeList, itrm1->second.lineNumber);
			else continue;		
		}
}

void collapse_circuit_v2_main(map<int, CircuitNode> &masterNodeList)
{
	map<int, CircuitNode>::iterator itrm;
	
	vector<int> start_list;
	for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
		if(itrm->second.pointFanOut.empty() && itrm->second.nodeType != PO && itrm->second.nodeType != PI)                        //has no fanout
		{
			start_list.push_back(itrm->second.lineNumber);
		}
	}
	for(int i = 0; i < start_list.size(); i++)
		collapse_circuit_v2(masterNodeList, start_list[i]);
}

//insertion_sort_asc()
void insertion_sort_asc(vector <int> &highestNodeList, int count)
{
	for (int i = 1; i < count; i++)
    {
   		if (highestNodeList[i - 1] > highestNodeList[i])
        {
        	int temp = highestNodeList[i];
            int j = i;
            while (j > 0 && highestNodeList[j - 1] > temp)
            {
           		highestNodeList[j] = highestNodeList[j - 1];
                j--;
            }
            highestNodeList[j] = temp;
        }
    }
}



//decimal to binary
void dtob(unsigned int d, vector<int> &bstr, int wordSize)
{
	if(d < 0)
	{   
		//redFILE0 << "Input d is less than 0!"<<endl;
		exit(1);
	}
		
    int mod = 0;
    vector <int> tmpstr;
    
    int i = 0;
    while (d > 0)
    {
        mod = d % 2;
        d /= 2;
        tmpstr.push_back(mod);
        i++;
    }
    unsigned int len = tmpstr.size();
    //redFILE0 << "tmpstr size = "<<tmpstr<<endl;
    //redFILE0 << "word size = "<<wordSize<<endl;
    if(len < wordSize)
    	for(i = 0; i < wordSize - len; i++)
    		tmpstr.push_back(0);
 
    //copy the string
   // redFILE0 << "returnVector: "<<endl;
    for (i = wordSize-1; i >= 0; i--)
    {
        bstr.push_back(tmpstr[i]);
    }
 	/*for(i = 0; i < bstr.size(); i++)
 		redFILE0 << bstr[i];
 	redFILE0 << endl;*/
}

void find_tranfanout(map<int, CircuitNode> &orNodeList_m, vector<int> &tranFanOut, int lineNumber)
{
	
	map <int,CircuitNode>::iterator itrm, itrm1;
	set <int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv;

	itrm = orNodeList_m.find(lineNumber);
	if (itrm->second.nodeType == 3)         //If corrent line is a PO, then return;
		return;
	
	for(itrv=itrm->second.pointFanOut.begin(); itrv!=itrm->second.pointFanOut.end(); itrv++)
	{	
		//redFILE1 << "Current node "<<itrm->second.lineNumber << "has fanout "<<(*itrv)->lineNumber<<endl;
		itrm1 = orNodeList_m.find((*itrv)->lineNumber);
		if(itrm1 != orNodeList_m.end())
		{
			if(search_vector_int(tranFanOut, itrm1->second.lineNumber) == -1)
				tranFanOut.push_back(itrm1->second.lineNumber);
			find_tranfanout(orNodeList_m, tranFanOut, itrm1->second.lineNumber);
		}
	}

}


void find_tranfanout_hash(map<int, CircuitNode> &orNodeList_m, HashTable &tranFanOut, int lineNumber)
{
    
	map <int,CircuitNode>::iterator itrm, itrm1;
	set <int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv;

//    redFILE1 << "In find_tranfanout_hash, current node = "<<lineNumber<<endl;
	itrm = orNodeList_m.find(lineNumber);	
	if(itrm == orNodeList_m.end())
	{
	    cerr<<"no this node!"<<endl;
	    return;
	}
	//If corrent line is a PO, then return;
	if (itrm->second.nodeType == 3 || itrm->second.pointFanOut.empty()) 
		return;
		
	for(itrv=itrm->second.pointFanOut.begin(); itrv!=itrm->second.pointFanOut.end(); itrv++)
	{	
//		redFILE1 << "Current node "<<itrm->second.lineNumber << "has fanout "<<(*itrv)->lineNumber<<endl;
		itrm1 = orNodeList_m.find((*itrv)->lineNumber);
		int fanout = itrm1->second.lineNumber;
		if(itrm1 != orNodeList_m.end() && itrm1->second.visit == 0)
		{
			int result = tranFanOut.Insert_Hash(fanout);
			while(result == 0)
				result = tranFanOut.Insert_Hash(fanout);
			itrm1->second.visit = 1;	
			find_tranfanout_hash(orNodeList_m, tranFanOut, fanout);
		}
	}
}

void find_tranfanout_hash_output(map<int, CircuitNode> &orNodeList_m, HashTable &tranFanOut, int lineNumber)
{
    
	map <int,CircuitNode>::iterator itrm, itrm1;
	set <int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv;

	itrm = orNodeList_m.find(lineNumber);	
	if(itrm == orNodeList_m.end())
	{
	    cerr<<"no this node!"<<endl;
	    return;
	}
	//If corrent line is a PO, then return;
	if (itrm->second.nodeType == 3 || itrm->second.pointFanOut.empty()) 
		return;
	
	for(itrv=itrm->second.pointFanOut.begin(); itrv!=itrm->second.pointFanOut.end(); itrv++)
	{	
		itrm1 = orNodeList_m.find((*itrv)->lineNumber);
		int fanout = itrm1->second.lineNumber;
		int p;
		if(opl_hash.Search_Hash(fanout, p) == 1)
		{
			int result = tranFanOut.Insert_Hash(fanout);
			while(result == 0)
				result = tranFanOut.Insert_Hash(fanout);			
		}
		find_tranfanout_hash(orNodeList_m, tranFanOut, fanout);
	}
}

void find_tranfanin(map <int, CircuitNode> &orNodeList_m, vector<int> &tranFanIn, int lineNumber)
{
	map<int,CircuitNode>::iterator itrm, itrm1;
	set<int>::iterator itrs;
	//vector<CircuitNode*>::iterator itrv;

	//redFILE1 << "In find_tranfanin, current node = "<<lineNumber<<endl;

	itrm = orNodeList_m.find(lineNumber);
	if (itrm->second.nodeType == 1 || itrm->second.pointFanIn.empty())         //If corrent line is a PI, then return;
		return;
	
	for(itrs=itrm->second.listFanIn.begin(); itrs!=itrm->second.listFanIn.end(); itrs++)
	{
		itrm1 = orNodeList_m.find(*itrs);
		if(itrm1 != orNodeList_m.end())
		{
			if (search_vector_int(tranFanIn, itrm1->second.lineNumber) == -1)
				tranFanIn.push_back(itrm1->second.lineNumber);
			find_tranfanin(orNodeList_m, tranFanIn, *itrs);
		}
	}

}

void find_tranfanin_hash(map <int, CircuitNode> &orNodeList_m, HashTable &tranFanIn, int lineNumber)
{

	map<int,CircuitNode>::iterator itrm, itrm1;
	set<int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv;

	itrm = orNodeList_m.find(lineNumber);
	//If corrent line is a PI, then return;
	if (itrm->second.nodeType == 1 || itrm->second.pointFanIn.empty())         
		return;
		
	
	for(itrv =itrm->second.pointFanIn.begin(); itrv!=itrm->second.pointFanIn.end(); itrv++)
	{
		itrm1 = orNodeList_m.find((*itrv)->lineNumber);
	//	redFILE0 <<  "Current node is "<<itrm1->second.lineNumber<<endl;
		if(itrm1 != orNodeList_m.end() && itrm1->second.visit == 0)
		{
			
			int result = tranFanIn.Insert_Hash((*itrv)->lineNumber);
			while(result == 0)
				result = tranFanIn.Insert_Hash((*itrv)->lineNumber);

            itrm1->second.visit = 1;
			find_tranfanin_hash(orNodeList_m, tranFanIn, (*itrv)->lineNumber);
		}
	}

}


void classify_lines(map <int, CircuitNode> &masterNodeList, vector<int> inputList, vector<int> &free_lines, vector<int> &bound_lines, vector<int> &head_lines)
{
	map<int,CircuitNode>::iterator itrm;
	head_lines.clear();
	head_lines = inputList;
	for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
	{
		if(itrm->second.nodeType != 1)
			bound_lines.push_back(itrm->second.lineNumber);
	}
}


int comp_map(map<int, int> m1, map<int, int> m2)
{
	map<int, int>::iterator itrmi1, itrmi2;
	itrmi1 = m1.begin();
	itrmi2 = m2.begin();
	if(itrmi1->first == itrmi2->first && itrmi1->second == itrmi2->second)
		return 1;
	return 0;
}

void local_implication_helper(map <int, CircuitNode> &masterNodeList, int inLineNumber, vector<int> &result_node) 
{

	map<int, CircuitNode>::iterator itrm, itrm1, itrm2;
	vector<CircuitNode*>::iterator itrv;
    set <int> gateVector[3];
    //gateVector = new set <int> [3];
    set <int>::iterator itrSet, itrs, itrs1, itrs2;
	int i, j;
	

    set <int> tempSet;  // To store the result of implication.

    itrm = masterNodeList.find(inLineNumber);
    itrs = itrm->second.lineValue.begin();
    
	//redFILE1 << endl <<"Current line = "<<inLineNumber<<endl;
	//redFILE1 << "stop_gate = "<<stop_gate<<endl;
	
	if(*itrs == X)
    	return;
	if(itrm->second.numberFanOut == 0)
		return;

    for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++)
	{
		itrm1 = masterNodeList.find((*itrv)->lineNumber);
		int num = 0;
		for(itrs1 = itrm1->second.listFanIn.begin(); itrs1 != itrm1->second.listFanIn.end(); itrs1++)
		{
			itrm2 = masterNodeList.find(*itrs1);
			gateVector[num++] = itrm2->second.lineValue;
		}
		
        //Call proper implication function depending on the number of the inputs to the gate.
        // Overloaded function, please refer to logic_implication.cc for details.
        switch (itrm1->second.numberFanIn) {
             case 1:
                 tempSet = forwardImplication (itrm1->second.gateType, gateVector[0]);
                 break;
             case 2:
                 tempSet = forwardImplication (itrm1->second.gateType, gateVector[0], gateVector[1]);
                 break;
             case 3:
                 tempSet = forwardImplication (itrm1->second.gateType, gateVector[0], gateVector[1], gateVector[2]);
                 break;
             default:
                 cerr << "This ATPG generator only works if the number of inputs to a gate is less than 4." << endl;
                 cerr << "Currently there are " << itrm->second.numberFanIn << " inputs to the gate." << endl;
                 exit (0);
                 break;
        }
		itrm1->second.lineValue.clear();
        itrm1->second.lineValue = tempSet;
		itrs1 = itrm1->second.lineValue.begin();
		if(*itrs1 != X)
			result_node.push_back(itrm1->second.lineNumber);
			
		local_implication_helper(masterNodeList, (*itrv)->lineNumber, result_node);
		
	}
	//delete []gateVector;

}


void local_implication(map <int, CircuitNode> &masterNodeList, int inLineNumber, bool inStuckAtValue, vector<int> &result_node)
{	
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2;
	vector<CircuitNode*>::iterator itrv;

	itrm = masterNodeList.find(inLineNumber);
	itrm->second.lineValue.clear();
	itrm->second.lineValue.insert(inStuckAtValue);
	
	//Perforem forward implication recursively.
    local_implication_helper(masterNodeList, inLineNumber, result_node);
}

bool remove_vector(vector<vector<int> > & NumVec, vector<int> & FlagVec)
{
	  int iSize = (int)NumVec.size();  
	  if (iSize != (int)FlagVec.size())  
	  	return false;

	  int iWritePos = 0;  
	  for (int iIterPos = 0; iIterPos < iSize; iIterPos++)
	  {
	  	if (!FlagVec.at(iIterPos))  
	    {
	      if (iWritePos != iIterPos)  
	        NumVec.at (iWritePos) = NumVec.at (iIterPos);
	      iWritePos++;
	    }
	  }

	if (iSize != iWritePos)  
		 NumVec.resize(iWritePos);

  return true;
}


void compute_cost(map<int, CircuitNode> &orNodeList_m, vector <FaultList> &original_sf, multimap<int, FaultList> &cost_sf, int first_cost)
{
    map<int, CircuitNode>::iterator itrm;
    vector <FaultList>::iterator itrvf;
    
	/*Backup the orNodeList_m and masterNodeList_m.*/
	map<int, CircuitNode> tempNodeList_m;
//	redFILE1 << "set line level: "<<endl;
	SetLineLevel_new(orNodeList_m, inputList);
	copy_point_vector1(orNodeList_m, tempNodeList_m);

	int ini_size = orNodeList_m.size();
	
	
	/*Computing costs for all faults.*/
	multimap<int, FaultList>::iterator itrmf;
	for(itrvf = original_sf.begin(); itrvf != original_sf.end(); itrvf++)
	{    						
		int red_line = (*itrvf).lineNumber;
		bool red_stuck = (*itrvf).stuckAtValue;
	//	redFILE1 << "compute cost for node "<<red_line<<", sa "<<red_stuck<<endl;
		
		itrm = orNodeList_m.find(red_line);
		if(itrm == orNodeList_m.end())
			continue;
		
		if(itrvf != original_sf.begin())
		    copy_point_vector1(orNodeList_m, tempNodeList_m);
		
//		

		/*redundancy forward deletion*/
		HashTable implicationNodeList;
		implicationNodeList.Init_HashTable();
	//	redFILE1 << "forward deletion: "<<endl;
		red_forward_deletion_v2(tempNodeList_m, implicationNodeList, red_line, red_stuck);
		implicationNodeList.Destroy_HashTable();				
		
		/*redundancy backward deletion*/
	//	redFILE1 << "backward deletion: "<<endl;
		red_backward_deletion(tempNodeList_m);
	//	redFILE1 << "collase:"<<endl;
		collapse_circuit(tempNodeList_m);
		
		
		/*Compute the saved number of connections.*/
		int new_size = tempNodeList_m.size();
		int save_size =  ini_size - new_size;
		//redFILE0 << "ini_size =" <<ini_size<<", ";
		//redFILE0 << "new_size =" <<new_size<<", ";
		//redFILE0 << "save_size = "<<save_size<<endl;	
		if(save_size <= first_cost)	
		    cost_sf.insert(pair<int, FaultList>(save_size, (*itrvf)));	
	//	redFILE1 << "fault "<<"("<<red_line<<", "<<red_stuck<<") is pushed into cost_sf!"<<endl;
				
	}
	
	/*redFILE1 << "Costs of all faults: "<<endl;
	for(itrmf = cost_sf.begin(); itrmf != cost_sf.end(); itrmf++)
		redFILE1 << "("<<itrmf->second.lineNumber<<", "<<itrmf->second.stuckAtValue<<"): "<<itrmf->first<<endl;
	redFILE1 << endl;*/
		
}


void compute_cost_cone(map<int, CircuitNode> &orNodeList_m, vector <FaultList> &original_sf, multimap<int, FaultList> &cost_sf, int first_cost)
{
    map<int, CircuitNode>::iterator itrm;
    vector <FaultList>::iterator itrvf;
    
	/*Backup the orNodeList_m and masterNodeList_m.*/
	map<int, CircuitNode> tempNodeList_m;
	copy_point_vector1(orNodeList_m, tempNodeList_m);

	int ini_size = orNodeList_m.size();
	
	/*Computing costs for all faults.*/
	//redFILE1 << endl << "Computing costs for all faults!"<<endl;
	multimap<int, FaultList>::iterator itrmf;
	for(itrvf = original_sf.begin(); itrvf != original_sf.end(); itrvf++)
	{    						
		int red_line = (*itrvf).lineNumber;
		bool red_stuck = (*itrvf).stuckAtValue;
		
		itrm = orNodeList_m.find(red_line);
		if(itrm == orNodeList_m.end())
			continue;
		
		if(itrvf != original_sf.begin())
		    copy_point_vector1(orNodeList_m, tempNodeList_m);

		/*redundancy forward deletion*/
		HashTable implicationNodeList;
		implicationNodeList.Init_HashTable();
		red_forward_deletion_v2(tempNodeList_m, implicationNodeList, red_line, red_stuck);
		implicationNodeList.Destroy_HashTable();				
		
		/*redundancy backward deletion*/
		red_backward_deletion(tempNodeList_m);
		collapse_circuit(tempNodeList_m);
		
		
		/*Compute the saved number of connections.*/
		int new_size = tempNodeList_m.size();
		int save_size =  ini_size - new_size;
		//redFILE0 << "ini_size =" <<ini_size<<", ";
		//redFILE0 << "new_size =" <<new_size<<", ";
		//redFILE0 << "save_size = "<<save_size<<endl;	
		if(save_size <= first_cost)	
		    cost_sf.insert(pair<int, FaultList>(save_size, (*itrvf)));	
		redFILE1 << "fault "<<"("<<red_line<<", "<<red_stuck<<") is pushed into cost_sf!"<<endl;
				
	}
	
	/*redFILE1 << "Costs of all faults: "<<endl;
	for(itrmf = cost_sf.begin(); itrmf != cost_sf.end(); itrmf++)
		redFILE1 << "("<<itrmf->second.lineNumber<<", "<<itrmf->second.stuckAtValue<<"): "<<itrmf->first<<endl;
	redFILE1 << endl;*/
}



void create_fault_list(map <int, CircuitNode> &masterNodeList_m, vector<FaultList> &orFaultList)
{
     map <int, CircuitLine> masterLineList_m;
	 map <int, CircuitLine>::iterator itrml;
		        
	 masterLineList_m.clear();
	 CreateFaultObjects(masterLineList_m, masterNodeList_m);
	 CreateFaultList(masterLineList_m, orFaultList, masterNodeList_m);
	 redFILE0 << "before collapse, #fault: "<<2*masterLineList_m.size()<<endl;
	 CollapseFaults(masterLineList_m, masterNodeList_m);
	 orFaultList.clear();
	 CreateFaultList(masterLineList_m, orFaultList,masterNodeList_m);	
	 redFILE0 << "Faults in orFaultList: "<<orFaultList.size()<<endl;
	 for(int j=0; j<orFaultList.size(); j++)
		 redFILE0 << "("<<orFaultList[j].lineNumber <<","<<orFaultList[j].stuckAtValue<<"), ";
	 redFILE1 <<endl; 

}


void simplify_circuit(map<int, CircuitNode> &orNodeList, int sa_line, int sa_value)
{
    struct timeb startTime, endTime;
    ftime(&startTime);
    SetLineLevel_new(orNodeList, inputList);
    ftime(&endTime);
//    redFILE1 <<"runtime for set line level: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
//    print_circuit(orNodeList, redFILE1);

    ftime(&startTime);
    HashTable implicationNodeList;
	implicationNodeList.Init_HashTable();
	red_forward_deletion_v2(orNodeList, implicationNodeList, sa_line, sa_value);
	implicationNodeList.Destroy_HashTable();
	ftime(&endTime);
//	redFILE1 <<"runtime for red_forward_deletion: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
		
	red_backward_deletion(orNodeList);
    collapse_circuit(orNodeList);
    
}

void update_testpool(vector<string> &testPool, string testVector)
{
    int flag_exist = 0;
    for(int k = 0; k < testPool.size(); k++)
        if(testPool[k] == testVector)
        {
            flag_exist = 1;
            break;
        }
    if(flag_exist == 0)
        testPool.push_back(testVector); 
}

void vector_fault_insert(vector<FaultList> &record, FaultList &fault)
{
    int flag_exist = 0;
    for(int i = 0; i < record.size(); i++)
    {
        if(record[i].lineNumber == fault.lineNumber && record[i].stuckAtValue == fault.stuckAtValue)
        {
            flag_exist = 1;
            break;
        }
    }
    if(flag_exist == 0)
        record.push_back(fault);
}

//form_cone_circuit
void form_cone_circuit(map<int, CircuitNode> &masterNodeList, map<int, CircuitNode> &cone_masterNodeList, int node)
{
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2;

    HashTable tfi;
    tfi.Init_HashTable();
    find_tranfanin_hash(masterNodeList, tfi, node);
    vector<int> delete_set;
    for(itrm = masterNodeList.begin(); itrm != masterNodeList.end(); itrm++)
    {
        int current_node = itrm->second.lineNumber;
        if(current_node == node)
            continue;
        int p;
        if(tfi.Search_Hash(current_node, p) == 0)
            delete_set.push_back(current_node);        
    }
    tfi.Destroy_HashTable();
    copy_point_vector1(masterNodeList, cone_masterNodeList);
    for(int i = 0; i < delete_set.size(); i++)
    {
        itrm = cone_masterNodeList.find(delete_set[i]);
        cone_masterNodeList.erase(itrm);
    }
    itrm = cone_masterNodeList.find(node);
    itrm->second.nodeType = 3;
    itrm->second.pointFanOut.clear();
    
    map<int, int> delete_fanout;
    map<int, int>::iterator itrmi;
    vector<CircuitNode*>::iterator itrv;
    for(itrm = cone_masterNodeList.begin(); itrm != cone_masterNodeList.end(); itrm++)
    {
        itrm1 = masterNodeList.find(itrm->second.lineNumber);
        for(itrv = itrm1->second.pointFanOut.begin(); itrv != itrm1->second.pointFanOut.end(); itrv++)
        {
            int fanout_node = (*itrv)->lineNumber;
            itrm2 = cone_masterNodeList.find(fanout_node);
            if(itrm2 == cone_masterNodeList.end())
            {
                delete_fanout.insert(pair<int, int>(fanout_node, itrm->second.lineNumber));
       //         redFILE1 << "node "<<itrm->second.lineNumber<<" 's fanout "<<fanout_node<<endl;   
            }
        }
    }
    for(itrmi = delete_fanout.begin(); itrmi != delete_fanout.end(); itrmi++)
    {
        itrm = cone_masterNodeList.find(itrmi->second);
        if(itrm != cone_masterNodeList.end())
        {
            vector_erase(itrm->second, itrmi->first, 2);
    //        redFILE1 << itrm->second.lineNumber<<" 's fanout "<<itrmi->first<<" is deleted."<<endl;
        }
    }    
}


//form_cone_circuit
void form_fanout_cone_circuit(map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &fanout_cone_circuit, int output_node)
{
    map<int, CircuitNode>::iterator itrm, itrm1, itrm2;

    HashTable tfo;
    tfo.Init_HashTable();
    find_tranfanout_hash(brNodeList, tfo, output_node);
    vector<int> delete_set;
    for(itrm = brNodeList.begin(); itrm != brNodeList.end(); itrm++)
    {
        int current_node = itrm->second.lineNumber;
        if(current_node == output_node)
            continue;
        int p;
        if(tfo.Search_Hash(current_node, p) == 0)
            delete_set.push_back(current_node);        
    }
    tfo.Destroy_HashTable();
    copy_point_vector1(brNodeList, fanout_cone_circuit);
    for(int i = 0; i < delete_set.size(); i++)
    {
        itrm = fanout_cone_circuit.find(delete_set[i]);
        fanout_cone_circuit.erase(itrm);
    }
 
    map<int, int> delete_fanin;
    map<int, int>::iterator itrmi;
    vector<CircuitNode*>::iterator itrv;
    for(itrm = fanout_cone_circuit.begin(); itrm != fanout_cone_circuit.end(); itrm++)
    {
        int cnode = itrm->second.lineNumber;
        itrm1 = brNodeList.find(cnode);
        for(itrv = itrm1->second.pointFanIn.begin(); itrv != itrm1->second.pointFanIn.end(); itrv++)
        {
            int fanin_node = (*itrv)->lineNumber;
            itrm2 = fanout_cone_circuit.find(fanin_node);
            if(itrm2 == fanout_cone_circuit.end())
            {
                delete_fanin.insert(pair<int, int>(fanin_node, cnode));                
            }
        }
    }
    for(itrmi = delete_fanin.begin(); itrmi != delete_fanin.end(); itrmi++)
    {
        itrm = fanout_cone_circuit.find(itrmi->second);
        if(itrm != fanout_cone_circuit.end())
        {
            vector_erase(itrm->second, itrmi->first, 1);
    //        redFILE1 << itrm->second.lineNumber<<" 's fanin "<<itrmi->first<<" is deleted."<<endl;
        }
    }  
    tfo.Destroy_HashTable();  
}



void find_dominator(map<int, CircuitNode> &masterNodeList, map<int, int> &dom, int target_node, map<int, map<int, int> > &dom_record) 
{
    //Iterators
	vector<CircuitNode*>::iterator itrv, itrv1;
	map<int, int>::iterator itrmi, itrmi1;
	map<int, CircuitNode>::iterator itrm, itrm1;
	typedef map<int, map<int, int> >::iterator PAT;

    dom.insert(pair<int, int>(target_node, target_node));
    itrm = masterNodeList.find(target_node);
    if(itrm->second.nodeType == 3)  //PO
        return;
 
    int num_prev = itrm->second.pointFanOut.size();
    map<int, int> *current_dom;
    current_dom = new map<int, int>[num_prev];
    if(current_dom == NULL)
    {
        redFILE1 << "malloc fails!"<<endl;
        exit(1);
    }
    int i = 0;
    for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++, i++)
    {
        int prev = (*itrv)->lineNumber;
        PAT ite = dom_record.find(prev);
        if(ite != dom_record.end())
            current_dom[i] = ite->second;
        else
            find_dominator(masterNodeList, current_dom[i], prev, dom_record);
    }
    if(num_prev > 1)
    {
        for(itrmi = current_dom[0].begin(); itrmi != current_dom[0].end(); itrmi++)
        {
            int pivot = itrmi->first;
            int num_pivot = 1;
            for(i = 1; i < num_prev; i++)
            {
                itrmi1 = current_dom[i].find(pivot);
                if(itrmi1 != current_dom[i].end())
                    num_pivot++;
            }
            if(num_pivot == num_prev)
                dom.insert(pair<int, int>(itrmi->first, target_node));
        }
    }
    else
    {
        if(!current_dom[0].empty())
        {
        //    redFILE1 << "size = "<<current_dom[0].size()<<endl;
            for(itrmi = current_dom[0].begin(); itrmi != current_dom[0].end(); itrmi++)
                dom.insert(pair<int, int>(itrmi->first, target_node));
        }
    }
    dom_record.insert(pair<int, map<int, int> >(target_node, dom));

//    redFILE1 << "delete current_dom"<<endl;
    delete []current_dom;
}


void find_dominator_main(map<int, CircuitNode> &masterNodeList, map<int, int> &dom, int target_node)
{
    //Iterators
	vector<CircuitNode*>::iterator itrv, itrv1;
	map<int, int>::iterator itrmi, itrmi1;
	map<int, CircuitNode>::iterator itrm, itrm1;

    map<int, map<int, int> > dom_record;
    find_dominator(masterNodeList, dom, target_node, dom_record);
}



void find_dominator_new_helper(map<int, CircuitNode> &orNodeList, map<int, int> &dom, int target_node, int output_node)
{
    //Iterators
	vector<CircuitNode*>::iterator itrv, itrv1;
	map<int, int>::iterator itrmi, itrmi1;
	map<int, CircuitNode>::iterator itrm, itrm1;

    dom.insert(pair<int, int>(target_node, target_node));   
//    redFILE1 << "target node = "<<target_node<<endl;
    
    if(target_node == output_node)  //PO
        return;
 
    itrm = orNodeList.find(target_node);
    int num_prev = itrm->second.pointFanOut.size();
    map<int, int> *current_dom;
    current_dom = new map<int, int>[num_prev];
    if(current_dom == NULL)
    {
        redFILE1 << "malloc fails!"<<endl;
        exit(1);
    }
    int i = 0;
    for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++, i++)
    {
        int prev = (*itrv)->lineNumber;
        find_dominator_new_helper(orNodeList, current_dom[i], prev, output_node);
    }
    if(num_prev > 1)
    {
        for(itrmi = current_dom[0].begin(); itrmi != current_dom[0].end(); itrmi++)
        {
            int pivot = itrmi->first;
            int num_pivot = 1;
            for(i = 1; i < num_prev; i++)
            {
                itrmi1 = current_dom[i].find(pivot);
                if(itrmi1 != current_dom[i].end())
                    num_pivot++;
            }
            if(num_pivot == num_prev)
                dom.insert(pair<int, int>(itrmi->first, target_node));
        }
    }
    else
    {
        if(!current_dom[0].empty())
        {
            for(itrmi = current_dom[0].begin(); itrmi != current_dom[0].end(); itrmi++)
                dom.insert(pair<int, int>(itrmi->first, target_node));
        }
    }

    delete []current_dom;
}



void find_dominator_new(map<int, CircuitNode> &orNodeList, map<int, int> &dom, int target_node, HashTable &tfo)
{
    //Iterators
	vector<CircuitNode*>::iterator itrv, itrv1;
	map<int, int>::iterator itrmi, itrmi1;
	map<int, CircuitNode>::iterator itrm, itrm1;
	struct timeb startTime, endTime;
	
	
	int j = 0;
	for(int i = 0; i < outputList.size(); i++)
	{
	    int output_node = outputList[i];
	    int p;
	    ftime(&startTime);
	    if(tfo.Search_Hash(output_node, p) == 0)
	        continue;
	    ftime(&endTime);
	    redFILE1 << "runtime for search hash: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;
	    map<int, int> this_dom;
	    map<int, CircuitNode> cone_orNodeList;
	    ftime(&startTime);
	    form_cone_circuit(orNodeList, cone_orNodeList, output_node);
	    ftime(&endTime);
	    redFILE1 << "runtime for form_cone_circuit: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;
	    redFILE1 << "find dominators wrt to output node "<<output_node<<endl;
	    ftime(&startTime);
	    find_dominator_new_helper(cone_orNodeList, this_dom, target_node, output_node);
	    ftime(&endTime);
	    redFILE1 << "runtime for find_dominator_new_helper: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;
	/*    redFILE1 << "this_dom: "<<endl;
	    for(itrmi = this_dom.begin(); itrmi != this_dom.end(); itrmi++)
	        redFILE1 << itrmi->first<<"  ";
	    redFILE1 << endl;*/
	    ftime(&startTime);
	    if(j++ == 0)
	        dom = this_dom;
	    else
	    {
	        map<int, int> new_dom;
	        for(itrmi = dom.begin(); itrmi != dom.end(); itrmi++)
	        {
	         //   redFILE1 << "current dom node: "<<itrmi->first;
	            itrmi1 = this_dom.find(itrmi->first);
	            if(itrmi1 != this_dom.end())
	            {   
	          //      redFILE1 << ", yes!"<<endl;
	                new_dom.insert(pair<int, int>(itrmi->first, itrmi->second));
	            }
	        }
	        dom = new_dom;
	    }
	    ftime(&endTime);
	    redFILE1 << "runtime for update_dom: " << ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0 << endl;
	}
}


void find_dominator_br(map<int, CircuitNode> &brNodeList, map<int, map<int, int> > &br_dom)
{
    //Iterators
	vector<CircuitNode*>::iterator itrv, itrv1;
	map<int, int>::iterator itrmi, itrmi1;
	map<int, CircuitNode>::iterator itrm, itrm1;
	
	
	int j = 0;
	for(int i = 0; i < outputList.size(); i++)
	{
	    int output_node = outputList[i];
	    redFILE1 << "output node = "<<output_node<<endl;
	    map<int, CircuitNode> fanout_cone_circuit;
	    redFILE1 << "form fanout cone circuit:"<<endl;
	    form_fanout_cone_circuit(brNodeList, fanout_cone_circuit, output_node);
	    map<int, int> this_dom;
	    redFILE1 << "find dominator for this node:"<<endl;
	    find_dominator_new_helper(fanout_cone_circuit, this_dom, output_node, PO_line);
	    br_dom.insert(pair<int, map<int, int> >(output_node, this_dom));
	/*    redFILE1 << "this_dom: "<<endl;
	    for(itrmi = this_dom.begin(); itrmi != this_dom.end(); itrmi++)
	        redFILE1 << itrmi->first<<"  ";
	    redFILE1 << endl;*/
	    
	}
}



int cal_cost(map<int, CircuitNode> &tmp_orNodeList, int sa_line, int sa_value)
{
    int old_size = tmp_orNodeList.size();    
    simplify_circuit(tmp_orNodeList, sa_line, sa_value);    
    int new_size = tmp_orNodeList.size();
    int cost = old_size - new_size;

    return cost;
    
}

int check_red_sat_minisat(map<int, CircuitNode> &tmp_orNodeList, vector<string> &br_lisp_log, ofstream &oufile)
{
    int isTestable;

    struct timeb startTime, endTime;
    ftime(&startTime);
    write_lisp(tmp_orNodeList, br_lisp_log, oufile);
    ftime(&endTime);
    redFILE1 << "runtime for write_lisp: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
    char tmp[100];
	sprintf(tmp, "bat lisp.log");
	system(tmp);
	
	char tmp_minisat[200];
	sprintf(tmp_minisat, "minisat_static -no-luby -rinc=1.5 -phase-saving=0 -rnd-freq=0.02 lisp.log.cnf lisp.log.out");
	system(tmp_minisat);
	
	string str;
	vector<string> result;
	ifstream resFile;
	resFile.open("lisp.log.out", ifstream::in);
	getline(resFile, str);
	redFILE1 << "str = "<<str<<endl;
	cout << "str = "<<str<<endl;
    if(str.size() == 5)
		isTestable = 0;             //untestable
	else if(str.size() == 3)
		isTestable = 1;             //testable
	else 
	{
		cerr << "error in sat!"<<endl;
		isTestable = -1;
	}
	return isTestable;
}



int check_red_sat(map<int, CircuitNode> &tmp_orNodeList, vector<string> &br_lisp_log, ofstream &oufile)
{
    int isTestable;

    struct timeb startTime, endTime;
    ftime(&startTime);
    write_lisp(tmp_orNodeList, br_lisp_log, oufile);
    ftime(&endTime);
    redFILE1 << "runtime for write_lisp: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
    ftime(&startTime);
    char tmp[100];
	sprintf(tmp, "bat -solver zchaff lisp.log");
	system(tmp);
	ftime(&endTime);
    redFILE1 << "runtime for sat: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
	
	string str;
	vector<string> result;
	ifstream resFile;
	resFile.open("lisp.log.out", ifstream::in);
	while(getline(resFile, str))
		result.push_back(str);
	int size = result.size();
	string res = result[size-1];
	int len = res.size();
	if(len == 13)
		isTestable = 0;             //untestable
	else if(len == 11)
		isTestable = 1;             //testable
	else 
	{
		cerr << "error in sat!"<<endl;
		isTestable = -1;
	}
	return isTestable;
}


int check_red_sat_abc(map<int, CircuitNode> &tmp_orNodeList, map<int, CircuitNode> &brNodeList, vector<string> &br_bench_log, ofstream &oufile)
{
    int isTestable;

    struct timeb startTime, endTime;
    ftime(&startTime);
    write_bench(tmp_orNodeList, brNodeList, br_bench_log, oufile);
    ftime(&endTime);
 //   redFILE1 << "runtime for write_bench: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
    
    ftime(&startTime);
    int flag  = call_abc();
    if(flag != 0)
    {
        redFILE1 << "error in call_abc!"<<endl;
        exit(1);
    }
    ftime(&endTime);
    redFILE1 << "runtime for write_cnf: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
    
    ftime(&startTime);
    char tmp[100];
//    sprintf(tmp, "zchaff bench.cnf");
	sprintf(tmp, "minisat_static bench.cnf bench.out");
	system(tmp);
	ftime(&endTime);
    redFILE1 << "runtime for sat: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
	
	string str;
	vector<string> result;
	ifstream resFile;
	resFile.open("bench.out", ifstream::in);
	getline(resFile, str);
	redFILE1 << "str = "<<str<<endl;
	cout << "str = "<<str<<endl;
    if(str.size() == 5)
		isTestable = 0;             //untestable
	else if(str.size() == 3)
		isTestable = 1;             //testable
	else 
	{
		cerr << "error in sat!"<<endl;
		isTestable = -1;
	}
	return isTestable;
}


void rem_new_unt(map<int, CircuitNode> &tmp_orNodeList, map<int, CircuitNode> &brNodeList, map<int, CircuitNode> &tmp_masterNodeList, multimap<int, int> &ICA, int wa_line, int sa_line, int sa_value, int this_cost, vector<string> &br_bench_log, ofstream &oufile)
{
    struct timeb startTime, endTime;
    map<int, CircuitNode>::iterator itrm;    
    redFILE1 << "sa_line = "<<sa_line<<", sa_value = "<<sa_value<<endl;

    multimap<int, FaultList>::reverse_iterator itrmf_r1;
    typedef multimap<int, FaultList>::iterator PAT;
    multimap<int, FaultList> untFaultSet;         
    ftime(&startTime);    	    	    	
    idtf_unt(tmp_orNodeList, tmp_masterNodeList, ICA, untFaultSet);
    ftime(&endTime);  
    redFILE1 << "runtime for idtf_unt: "<<((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<<endl;
    map<int, CircuitNode> bak_tmp_orNodeList;
    copy_point_vector1(tmp_orNodeList, bak_tmp_orNodeList);
    multimap<int, FaultList> newly_unt_faults;        	        
    redFILE1 << "newly untestable wires: "<<untFaultSet.size()<<endl;
    int flag_sa = 0;
    for(PAT ite = untFaultSet.begin(); ite != untFaultSet.end(); ite++)
    {             	               	            
        if(ite->first == wa_line)
            continue;
        redFILE1 << "("<<ite->first<<","<<ite->second.stuckAtValue<<")  ";
        if(ite->first == sa_line && ite->second.stuckAtValue == sa_value)
            flag_sa = 1;
        FaultList fault(ite->first, ite->second.stuckAtValue);
        int current_cost = cal_cost(tmp_orNodeList, ite->first, ite->second.stuckAtValue);
        newly_unt_faults.insert(pair<int, FaultList>(current_cost, fault));
        copy_point_vector1(bak_tmp_orNodeList, tmp_orNodeList);
        redFILE1 << "cost = "<<current_cost<<endl;
    }
    redFILE1 << endl;  	        
    if(flag_sa == 0)
    {
        FaultList fault(sa_line, sa_value);
        newly_unt_faults.insert(pair<int, FaultList>(this_cost, fault));
        redFILE1 << "("<<sa_line<<","<<sa_value<<")  cost = "<<this_cost<<endl;
    }    	  
         
    //start remove redundancies                            
    if(newly_unt_faults.size() == 1)
    {
        redFILE1 << "No other new untestable faults exist!"<<endl;
        simplify_circuit(tmp_orNodeList, sa_line, sa_value);         
        redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;     
    }
    else 
    {
        redFILE1 << "##Check newly untestable faults!"<<endl;
        int k = 0;
        for(itrmf_r1 = newly_unt_faults.rbegin(); itrmf_r1 != newly_unt_faults.rend(); itrmf_r1++, k++)
        {
            int nsa_line = itrmf_r1->second.lineNumber;
            int nsa_value = itrmf_r1->second.stuckAtValue;
            int ncost = itrmf_r1->first;
            redFILE1 << "current fault: ("<<nsa_line<<","<<nsa_value<<"), cost = "<<ncost<<endl;
            if(k == 0)
            {
                simplify_circuit(tmp_orNodeList, nsa_line, nsa_value);
                redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;
                continue;
            } 
            itrm = tmp_orNodeList.find(nsa_line);
            if(itrm == tmp_orNodeList.end())
                continue;          
                
            map<int, CircuitNode> sim_tmp_orNodeList;
            copy_point_vector1(tmp_orNodeList, sim_tmp_orNodeList);
            simplify_circuit(sim_tmp_orNodeList, nsa_line, nsa_value);  
            int isTestable_new_sat = check_red_sat_abc(sim_tmp_orNodeList, brNodeList, br_bench_log, oufile);

            if(!isTestable_new_sat)
            {
                simplify_circuit(tmp_orNodeList, nsa_line, nsa_value);
                redFILE1 << "after remove redundancy, circuit size = "<<tmp_orNodeList.size()<<endl;                 
            }                     
        }//for all faults in newly_unt_faults
    }//else()               
}


void top_sort(map<int, CircuitNode> &nodeList, vector<int> &sort_list)
{
    vector<CircuitNode*>::iterator itrv;
	map<int, int>::iterator itrmi;
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2;

    queue sort_queue;
    map<int, int> indegree_list; 
    for(itrm = nodeList.begin(); itrm != nodeList.end(); itrm++)
    {
        int cnode = itrm->second.lineNumber;
        int num_fanin = itrm->second.pointFanIn.size();
        indegree_list.insert(pair<int, int>(cnode, num_fanin));
        if(num_fanin == 0)
            sort_queue.push(cnode);
    }
    
    while(!sort_queue.empty())
    {
        int current_node = sort_queue.pop();
        sort_list.push_back(current_node);
        itrm = nodeList.find(current_node);
        for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++)
        {
            int cnode = (*itrv)->lineNumber;
            itrmi = indegree_list.find(cnode);
            itrmi->second = itrmi->second - 1;
            if(itrmi->second == 0)
                sort_queue.push(cnode);
        }
    }
    
}


void remove_inv(map<int, CircuitNode> &nodeList, int node)
{
    map<int, CircuitNode>::iterator itrm, itrm_in, itrm_out;
    vector<CircuitNode*>::iterator itrv;

    itrm = nodeList.find(node);
    itrv = itrm->second.pointFanIn.begin();
    itrm_in = nodeList.find((*itrv)->lineNumber);
    itrm_in->second.pointFanOut.clear();
    for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++)
    {
        itrm_out = nodeList.find((*itrv)->lineNumber);
        vector_erase(itrm_out->second, node, 1);
        vector_insert(itrm_out->second, itrm_in->second.lineNumber, 1, nodeList);
        itrm_out->second.listFanIn.insert(itrm_in->second.lineNumber);
   //     itrm_out->second.pointFanIn.push_back(&itrm_in->second);
        itrm_in->second.pointFanOut.push_back(&itrm_out->second);
    }
    nodeList.erase(itrm);
        
}



