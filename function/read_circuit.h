#ifndef _READ_CIRCUIT_H
#define _READ_CIRCUIT_H


#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "../class/CircuitNode.h"
#include "../class/FaultList.h"

void ReadCircuit(ifstream &inFile, vector<CircuitNode> &NodeList, int circuitIndex);

void ReadFaultList(ifstream &inFile, map<int, CircuitNode> &masterNodeList, vector<FaultList> &providedFaultList);

#endif
