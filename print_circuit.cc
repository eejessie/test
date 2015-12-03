/*
 * =====================================================================================
 *
 *       Filename:  print_circuit.cc
 *
 *    Description:  This function merges the original circuit and the boolean relation 
 *					circuit
 *
 *        Version:  1.0
 *        Created:  16/07/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yi Wu <eejessie@sjtu.edu.cn>
 *
 * =====================================================================================
 */

// #####   HEADER FILE INCLUDES   ################################################### 

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
  

using namespace std;

extern ofstream redFILE0, redFILE1;

void print_circuit(map <int, CircuitNode> &masterNodeList_m, ofstream &redFILE)
{
	map<int, CircuitNode>::iterator itrm;
	set<int>::iterator itrs;
	vector<CircuitNode*>::iterator itrv;

	redFILE1 << "Circuit size = "<<masterNodeList_m.size()<<endl;
	redFILE0 << "Circuit size = "<<masterNodeList_m.size()<<endl;
	//redFILE << "nT\tn\tgT\t#FI\t#FO\tFI"<<endl;
	redFILE << "*********************************************"<<endl;
	redFILE << "Val\tlevel\tci\tn\tnT\tgT\tFI\t\tFO"<<endl;
	for (itrm = masterNodeList_m.begin(); itrm != masterNodeList_m.end(); itrm++)
	{
	  
		if(!itrm->second.lineValue.empty())
		{
			itrs = itrm->second.lineValue.begin();
			redFILE <<*itrs<<"\t";
		}
		else 
			redFILE << "\t";
		redFILE << itrm->second.lineLevel<<"\t";
		redFILE <<itrm->second.circuitIndex<<"\t";
		redFILE << itrm->second.lineNumber<<"\t";
		redFILE <<itrm->second.nodeType<<"\t";
		redFILE <<itrm->second.gateType<<"\t";
		for(itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++)
			redFILE <<(*itrv)->lineNumber<<" ";
		redFILE << "\t\t";
		/*for(itrs = itrm->second.listFanIn.begin(); itrs != itrm->second.listFanIn.end(); itrs++)
			redFILE <<*itrs<<" ";
		redFILE << endl;*/
		for(itrv = itrm->second.pointFanOut.begin(); itrv != itrm->second.pointFanOut.end(); itrv++)
			redFILE <<(*itrv)->lineNumber<<" ";		
		redFILE << endl;
		
	}
	redFILE << "*********************************************"<<endl;
	redFILE << endl;
		
}

