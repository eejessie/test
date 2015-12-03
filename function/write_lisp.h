#ifndef WRITE_LISP_H
#define WRITE_LISP_H 
 
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


	void write_lisp(map<int, CircuitNode> &nodeList, vector<string> &br_lisp_log, ofstream & oufile);

#endif 

