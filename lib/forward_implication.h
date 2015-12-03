/*
 * =====================================================================================
 *
 *       Filename:  forward_implication.h
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


#ifndef FORWARD_IMPLICATION_H
#define FORWARD_IMPLICATION_H

	
	set <int>   forwardImplication (unsigned int gateType, set <int> & input_a);
	set <int>   forwardImplication (unsigned int gateType, set <int> &input_a, set <int> &input_b);
	set <int>   forwardImplication (unsigned int gateType, set <int> &input_a, set <int> &input_b, set <int> &input_c);
	vector <int>   forwardImplication (unsigned int gateType, vector <int> & input_a);
	vector <int>   forwardImplication (unsigned int gateType, vector<int> &input_a, vector<int> &input_b);
	void backwardImplication1 (unsigned int gateType, set <int> &output_a, set <int> &input_b);
	void backwardImplication2 (unsigned int gateType, set <int> &output_a, set <int> &input_b, set <int> &input_c);
	vector <int>   forwardImplication (unsigned int gateType, vector <int> & input_a, int length);
	vector <int>   forwardImplication (unsigned int gateType, vector <int> &input_a, vector <int> &input_b, int length);
	int check_consistency(unsigned int gateType, set <int> &output, set <int> &input);
	int check_consistency(unsigned int gateType, set <int> &output, set <int> &input_a, set <int> &input_b, int num_known);
	

#endif 

