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


#include "class/CircuitNode.h"  
#include "class/FaultList.h"       

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ReadCircuit
 *  Description:  This function, once it can successfully open the circuit file, will 
 *                then populate the structure/class with the proper values from the file.
 * =====================================================================================
 */
void ReadCircuit_v2(ifstream &inFile, map<int, CircuitNode> &NodeList, vector<int> &inputList, vector<int> &outputList, int circuitIndex) {

    unsigned int nodeType;
    unsigned int fanInListMember;

    while (true) {
        inFile >> nodeType;                         // This is the type of current node.
        if (inFile.eof())
            break;
        CircuitNode *thisNode;
        thisNode = new CircuitNode (nodeType);      // Constructor checks for inputs and outputs.
        

        switch (nodeType) {
            case PI:                                // Check if the current node is a primary input.
                inFile >> thisNode->lineNumber;     // Unique ID for the line.
                inFile >> thisNode->gateType;       // This will be always 0 for primary inputs.
                inFile >> thisNode->numberFanOut;   // Number of lines connected to this input.
                inFile >> thisNode->numberFanIn;    // This will always be 0 for primary inputs.
                
                break;

            case FB:
                inFile >> thisNode->lineNumber;     // Unique ID for the line.
                inFile >> thisNode->gateType;       // This will be always 1 for branches.
                thisNode->numberFanIn = 1;          // Branch always has only one input.
                thisNode->numberFanOut = 100;       // Branch can have multiple outputs but only one per line.
                //TODO: Figure this out - 100 is incorrect. Needs to be proper value.
                inFile >> fanInListMember;          // This is the fan out for branch.
                thisNode->listFanIn.insert(fanInListMember);
                break;

            case GT:
                inFile >> thisNode->lineNumber;     // Unique ID for the line.
                inFile >> thisNode->gateType;       // Gate type.
                inFile >> thisNode->numberFanOut;   // Number of lines connected to this node.
                inFile >> thisNode->numberFanIn;    // Number of lines connected at this node.
                for (int i = 0; i < thisNode->numberFanIn; i++) {
                    inFile >> fanInListMember;
                    thisNode->listFanIn.insert(fanInListMember);
                }
                break;

            case PO:
                inFile >> thisNode->lineNumber;     // Unique ID for the line.
                inFile >> thisNode->gateType;       // Type of the gate output is connected to.
                inFile >> thisNode->numberFanOut;   // Always zero for outputs.
                inFile >> thisNode->numberFanIn;    // Number of lines connected at this node.
                for (int i = 0; i < thisNode->numberFanIn; i++) {
                    inFile >> fanInListMember;
                    thisNode->listFanIn.insert(fanInListMember);
                }
                break;

            default:                                // Undefined node type.
                cerr << "ERROR: Undefined Node Type." << endl
                     << "Valid values are 0, 1, 2 and 3." << endl
                     << "Current value is " << nodeType << endl;
                exit (1);
                break;                              // No necessary since exit (1) will end program.
        }

		thisNode->circuitIndex = circuitIndex;
		thisNode->rectFlag = 0;
        NodeList.insert(pair<int, CircuitNode>(thisNode->lineNumber, *thisNode));

        delete thisNode;

    }

    //complete the pointFanIn of each node
	set <int>::iterator it, ij;
	vector <CircuitNode*>::iterator itrv;
	map<int, CircuitNode>::iterator itrm, itrm1;
	for(itrm = NodeList.begin(); itrm != NodeList.end(); itrm++)
	{
	    if(itrm->second.nodeType == 1)
	        inputList.push_back(itrm->second.lineNumber);
	    else if(itrm->second.nodeType == 3)
	        outputList.push_back(itrm->second.lineNumber);
	    for(it = itrm->second.listFanIn.begin(); it != itrm->second.listFanIn.end(); it++)
	    {
	        itrm1 = NodeList.find(*it);
	        itrm->second.pointFanIn.push_back(&itrm1->second);
	    }
	    for(itrv = itrm->second.pointFanIn.begin(); itrv != itrm->second.pointFanIn.end(); itrv++)
	    {
	        (*itrv)->pointFanOut.push_back(&itrm->second);
	    }
	}
 
}




