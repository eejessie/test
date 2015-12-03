/*
 * =====================================================================================
 *
 *       Filename:  CircuitNode.cc
 *
 *    Description:  This file defines the methods in the CircuitNode class.
 *                  This class stores information about every node in the circuit.
 *
 *        Version:  1.0
 *        Created:  12/02/2011
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aditya Shevade <aditya.shevade@gmail.com>,
 *                  Amey Marathe <marathe.amey@gmail.com>,
 *                  Samik Biswas <samiksb@gmail.com>,
 *                  Viraj Bhogle <viraj.bhogle@gmail.com>
 *
 * =====================================================================================
 */
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
#include "CircuitNode.h"

// Global constants are defined in this file.
#ifndef GLOBAL_DEFINES_H_
#include "../include/global_defines.h"
#endif


using namespace std;
extern ofstream redFILE0;
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CircuitNode
 *      Method:  CircuitNode :: CircuitNode
 * Description:  The circuit file is read and an object of this class is created for 
 *               each line in the circuit file. Then this constructor is called which 
 *               initializes some members and remaining members are directly written to 
 *               by the values read from the file.
 *--------------------------------------------------------------------------------------
 */
CircuitNode::CircuitNode() {
    //this->nodeType = inNodeType;    // Set the type of current object.
    totalNodes++;                   // Total number of nodes.
    /*if (inNodeType == PI)
        totalInputs ++;             // Total number of inputs to circuit.
    if (inNodeType == PO)
        totalOutputs ++;            // Total number of outputs from circuit.*/

    this->nodeIndex = totalNodes;   // Set the node ID of current object.
} 
 
CircuitNode::CircuitNode(int inNodeType) {
    this->nodeType = inNodeType;    // Set the type of current object.
    totalNodes++;                   // Total number of nodes.
    if (inNodeType == PI)
        totalInputs ++;             // Total number of inputs to circuit.
    if (inNodeType == PO)
        totalOutputs ++;            // Total number of outputs from circuit.

    this->nodeIndex = totalNodes;   // Set the node ID of current object.
}

CircuitNode &CircuitNode::operator =(CircuitNode &node)
//void CircuitNode::operator =(CircuitNode &node)
{
	redFILE0 << "Enter into operator = "<<endl;
	vector<CircuitNode*>::iterator itrv;
	if(this != &node)
	{
		nodeIndex = node.nodeIndex;    
	    nodeType = node.nodeType;
	    lineNumber =  node.lineNumber;
	    gateType =  node.gateType;
	    numberFanIn = node.numberFanIn;
	    numberFanOut = node.numberFanOut;
		circuitIndex = node.circuitIndex;
		lineLevel = node.lineLevel;
		internal = node.internal;
		listFanIn = node.listFanIn;
	    lineValue = node.lineValue;
	    rectFlag = node.rectFlag;
	    visit = node.visit;
		//pointFanIn = node.pointFanIn;
		//pointFanOut =  node.pointFanOut;
		//for(itrv = node.pointFanOut.begin(); itrv != node.pointFanOut.end(); itrv++)
		//	redFILE0 << "Node "<<node.lineNumber<<" has fanout "<<(*itrv)->lineNumber<<endl;
	}
	return *this;
}

CircuitNode::CircuitNode(const CircuitNode &node)
{
	//redFILE0 << "Enter into copy constructor "<<endl;
	nodeIndex = node.nodeIndex;    
	    nodeType = node.nodeType;
	    lineNumber =  node.lineNumber;
	    gateType =  node.gateType;
	    numberFanIn = node.numberFanIn;
	    numberFanOut = node.numberFanOut;
		circuitIndex = node.circuitIndex;
		lineLevel = node.lineLevel;
		internal = node.internal;
		listFanIn = node.listFanIn;
	    lineValue = node.lineValue;
	    rectFlag = node.rectFlag;
		//CircuitNode *p1, *p2;
		//p1 = new CircuitNode(GT);
		//pointFanIn.push_back(p1);
		//pointFanIn.clear();
		//p2 = new CircuitNode(GT);
		//pointFanOut.push_back(p2);
		//pointFanOut.clear();
		//delete p1, p2;
		
}

/*
 * Static variables must be declared globally in order to work properly.
 */
int CircuitNode::totalNodes;     // Number of nodes, global.
int CircuitNode::totalInputs;    // Number of primary inputs, global.
int CircuitNode::totalOutputs;   // Number of primary outputs, global.


