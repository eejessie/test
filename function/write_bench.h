#ifndef WRITE_BENCH_H
#define WRITE_BENCH_H 
 
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

    void write_bench_basic(map<int, CircuitNode> &nodeList, map<int, int> &const_pair, ofstream &oufile);

	void write_bench(map<int, CircuitNode> &orNodeList, map<int, CircuitNode> &brNodeList, vector<string> &br_bench_log, ofstream &oufile);
	
	void write_bench_or(map<int, CircuitNode> &orNodeList, ofstream &oufile, string &truename);

#endif 

