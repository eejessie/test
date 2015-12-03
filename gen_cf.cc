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
#include "class/ConnFault.h"
#include "function/print_circuit.h"
#include "function/run_logic_simulation.h"
#include "function/copy_point_vector.h"
#include "function/helper.h"

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif


using namespace std;

extern ofstream redFILE0;


//gen_cf
int gen_cf(map<int, CircuitNode> &orNodeList_m, vector<ConnFault> &cf, int totalInputs)
{
	int i, count_cf=0, flag=0;
	map <int, CircuitNode> tempNodeList;
	map <int, CircuitNode>::iterator itrm, itrm1;
	set <int>::iterator itrs, itrs1;
	vector <int> tranFanOut;
	vector <int> tranFanIn;
	vector <int> tranFanOutIn;
	vector <int> safeNodeList;
	

	// Copy the nodes information in orNodeList_m to tempNodeList.
	copy_point_vector1(orNodeList_m, tempNodeList);
	ofstream cfFILE;
	cfFILE.open ("CF.log", ofstream::out);

	//Put the nodes in orNodeList_m but not in tranFanOut and tranFanIn into safeNodeList.
	for(itrm=orNodeList_m.begin(); itrm!=orNodeList_m.end(); itrm++)
	{
		//cfFILE <<"current node is "<<itrm->second.lineNumber<<endl;
		if(itrm->second.nodeType == 2 ) //Ignore the branches
			continue;
		if(itrm->second.nodeType == 3 ) //Ignore the outputs
			continue;
	    if(itrm->second.gateType == 11)
	        continue;
				
		CircuitNode currentNode(itrm->second);
		tranFanOut.clear();
		tranFanIn.clear();
		tranFanOutIn.clear();
		safeNodeList.clear();

		tranFanOut.push_back(currentNode.lineNumber);  //Put the node itself into its transistive fanout list.
		find_tranfanout(orNodeList_m, tranFanOut, currentNode.lineNumber);
		find_tranfanin(orNodeList_m, tranFanIn, currentNode.lineNumber);
		
		for(i=0; i<tranFanOut.size(); i++)
			tranFanOutIn.push_back(tranFanOut[i]);
		for(i=0; i<tranFanIn.size(); i++)
			tranFanOutIn.push_back(tranFanIn[i]);

		for(itrm1=tempNodeList.begin(); itrm1!=tempNodeList.end(); itrm1++)
		{
			int flag = 0;
			for(i=0; i<tranFanOutIn.size(); i++)
				if (itrm1->second.lineNumber == tranFanOutIn[i])
				{
					flag = 1;
					break;
				}
			if (flag == 0)
				//The only destination can be GT or PO and the lineNumber of destination gate should not exceed the boundary.
				if (itrm1->second.nodeType == 0 || itrm1->second.nodeType == 3)  
					if((itrm1->second.gateType != 0) && (itrm1->second.gateType != 1) && (itrm1->second.gateType != 5) && (itrm1->second.gateType != 11))    
						safeNodeList.push_back(itrm1->second.lineNumber);
		}
		
		//test for each node in orNodeList_m, whether safeNodeList contains the right nodes	
		//cout<<"For node "<<itrm->second.lineNumber<<":"<<endl;	
		for(i=0; i<safeNodeList.size(); i++)
		{
			//cout<<safeNodeList[i].lineNumber<<" ";
			ConnFault *CF;
			CF = new ConnFault[2];
			CF[0].src = currentNode.lineNumber;
			CF[0].dest = safeNodeList[i];
			CF[0].pola = 0;
			CF[1].src = currentNode.lineNumber;
			CF[1].dest = safeNodeList[i];
			CF[1].pola = 1;
			cf.push_back(CF[0]);
			cf.push_back(CF[1]);
		}
		//cout << endl;
		count_cf = count_cf + 2*safeNodeList.size();					
	}	

	
	#ifdef DEBUG 
	cfFILE<<"----------------------------------------------"<<endl;
	cfFILE << "total counts of connection faults is: "<<count_cf<<endl;
	cfFILE << cf.size()<<endl;
	for(i=0; i<cf.size(); i++)
	{
		cfFILE << "("<<cf[i].src<<", "<<cf[i].dest<<", "<<cf[i].pola<<")"<<endl;
	}
	#endif
	
	return count_cf;
}




