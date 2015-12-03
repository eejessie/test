/*
 * =====================================================================================
 *
 *       Filename:  red_backward_deletion.cc
 *
 *    Description:  This function performs backward deletion/*
 * =====================================================================================
 *
 *       Filename:  red_backward_deletion.cc
 *
 *    Description:  This function performs forward implication on a 1-3 input gate.
 *                  The inputs and output are integer sets. It reads the values from
 *                  logic tables.
 *
 *        Version:  1.0
 *        Created:  16/07/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author: Yi Wu <eejessie@sjtu.edu.cn>
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
#include <utility>

#include "class/CircuitNode.h"   
#include "function/helper.h"   
 
using namespace std;

extern ofstream redFILE0, redFILE1;


void red_backward_deletion(map <int, CircuitNode> &simNodeList)
{
	/*redFILE0 <<endl;
	redFILE0 << "********************************"<<endl;  	
	redFILE0 <<"Enter into red_backward_deletion!"<<endl;
	redFILE0 << "********************************"<<endl; */


	int i, input, lineNumber;
	vector<CircuitNode>::iterator itrv;
	vector<CircuitNode*>::iterator itrv1;
	map<int, CircuitNode>::iterator itrm0, itrm;
	set<int>::iterator itrs, itrs1, itrs2;
	vector<int> impendingNodeList;
	vector<int>::iterator itrv0;

	for(itrm=simNodeList.begin(); itrm!=simNodeList.end(); itrm++)
		if((itrm->second.pointFanOut.empty()) && (itrm->second.nodeType != 3) && (itrm->second.nodeType != 1))
		{
			impendingNodeList.push_back(itrm->second.lineNumber);
		//	redFILE0 << "Node"<<" "<<itrm->second.lineNumber<<" "<<"is impending!"<<endl;
		}
	
    vector<CircuitNode> FanIn;
	while(!impendingNodeList.empty())
	{
		FanIn.clear();
		itrv0 = impendingNodeList.begin();
		lineNumber = *itrv0;
		//cout << "Current node is "<<lineNumber<<endl;
		itrm0 = simNodeList.find(lineNumber);

		for(itrv1 = itrm0->second.pointFanIn.begin(); itrv1 != itrm0->second.pointFanIn.end(); itrv1++)
		{
			itrm = simNodeList.find((*itrv1)->lineNumber);
			
			if(itrm->second.pointFanOut.size() == 1)
			{
				if(itrm->second.nodeType != 1)
					impendingNodeList.push_back(itrm->second.lineNumber);
				itrm->second.pointFanOut.clear();
			}
			else if(itrm->second.pointFanOut.size() > 1)
				vector_erase(itrm->second, lineNumber, 2);
			
		//	redFILE0 << "Node"<<" "<<itrm->second.lineNumber<<" "<<"is pushed into impendingNodeList!"<<endl;
		}	
	
		itrv0 = impendingNodeList.begin();
		impendingNodeList.erase(itrv0);
		simNodeList.erase(lineNumber);
		//redFILE1 << "Node"<<" "<<lineNumber<<" "<<"is erased from simNodeList!"<<endl;
		
	}

	
}



