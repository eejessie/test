/*
 * =====================================================================================
 *
 *       Filename:  file_operation.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16/07/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yi Wu
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
#include <cassert>
#include "../class/CircuitNode.h"

using namespace std;

#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

	
	void openInFile (char *fileName, ifstream &inFile, ofstream &logFile);
	void openOutFile (char *fileName, ofstream &inFile, ofstream &logFile);


#endif 

