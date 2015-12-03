/*
 * =====================================================================================
 *
 *       Filename:  forward_implication.cc
 *
 *    Description:  This function performs forward implication on a 1-3 input gate.
 *                  The inputs and output are integer sets. It reads the values from
 *                  logic tables.
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
#include <cassert>

#include "../function/helper.h"

using namespace std;

extern ofstream redFILE0, redFILE1;

// #####   HEADER FILE INCLUDES   ################################################### 


#ifndef LOGIC_TABLES_H_
#include "../include/logic_tables.h"
#endif


// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "../include/global_defines.h"
#endif


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  forwardImlpication
 *  Description:  The function takes in an STL set as input and returns a set as output.
 *                The version defined below is a single input version.
 *                Set is used since multiple inputs may give same result.
 * =====================================================================================
 */
set <int>   forwardImplication (unsigned int gateType, set <int> & input_a) {

    set <int> returnSet;
    set <int>::iterator i;
	

	//cout << "In forwardImplication:"<<endl;
    for (i = input_a.begin(); i != input_a.end(); i++) {
	//	cout << "input_a = "<<*i<<endl;
        switch (gateType) {
            case INV:
            case NAND:
            case NOR:
                returnSet.insert(INVTABLE[*i]);
                break;
            case BRNCH:
            case BUFF:
            case AND:
            case OR:
                returnSet.insert(BRNCHTABLE[*i]);
                break;
            case XOR:
                returnSet.insert(ONEXORTABLE[*i]);
                break;
            default:
                cerr << "Unknown gate type encountered for a gate with only 1 input." << endl;
                cerr << "Possible values are 0-7. Current value is " << gateType << endl;
                exit (0);
                break;
        }
    }

	
	//for(i = returnSet.begin(); i!=returnSet.end(); i++)
	//	cout << "returnSet = "<<*i<<endl<<endl;
    return (returnSet);

}

vector <int>   forwardImplication (unsigned int gateType, vector <int> & input_a) {

    vector <int> returnVector;
    vector <int>::iterator i;

    for (i = input_a.begin(); i != input_a.end(); i++) {
        switch (gateType) {
            case INV:
            case NAND:
            case NOR:
                returnVector.push_back(INVTABLE[*i]);
                break;
            case BRNCH:
            case AND:
            case OR:
                returnVector.push_back(BRNCHTABLE[*i]);
                break;
            case XOR:
                returnVector.push_back(ONEXORTABLE[*i]);
                break;
            default:
                cerr << "Unknown gate type encountered for a gate with only 1 input." << endl;
                cerr << "Possible values are 0-7. Current value is " << gateType << endl;
                exit (0);
                break;
        }
    }
    return (returnVector);
}

vector <int>   forwardImplication (unsigned int gateType, vector <int> & input_a, int wordSize) {

    vector <int> returnVector;
    vector <int>::iterator i;
    
  //  redFILE1 << "One input!"<<endl;
 //   redFILE1 << "gate type: "<<gateType<<endl;

	unsigned int value = 0, result;
	int len = input_a.size();
	//redFILE0 << "input_a.size =" << len <<  endl;
	for(int i = 0; i < wordSize; i++)
	{
//	    redFILE1 << input_a[i];
		value = value + input_a[i] * pow(2, wordSize-1-i);		
	}
//	redFILE1 << endl << "value = "<<value<<endl;
    switch (gateType) {
        case INV:
            result = ~value;
            break;
        case BRNCH:
        case AND:
        case OR:
        case BUFF:
            result = value;
            break;
        default:
            cerr << "Unknown gate type encountered for a gate with only 1 input." << endl;
            cerr << "Possible values are 0-7. Current value is " << gateType << endl;
            exit (0);
            break;
    }
    dtob(result, returnVector, wordSize);
    return (returnVector);
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  forwardImlpication
 *  Description:  The function takes in an STL set as input and returns a set as output.
 *                The version defined below is a two input version.
 *                Set is used since multiple inputs may give same result.
 * =====================================================================================
 */
set <int>   forwardImplication (unsigned int gateType, set <int> &input_a, set <int> &input_b) {

	//redFILE1 << "forwardImplication: 2 inputs."<<endl;
    set <int> returnSet;
    set <int>::iterator i, j;

	//cout << "In forwardImplication:"<<endl;
    for (i = input_a.begin(); i != input_a.end(); i++) {
        for (j = input_b.begin(); j != input_b.end(); j++) {
            switch (gateType) {
                case AND:
                    returnSet.insert(ANDTABLE[*i][*j]);
                    break;
                case NAND:
                    returnSet.insert(NANDTABLE[*i][*j]);
                    break;
                case OR:
                    returnSet.insert(ORTABLE[*i][*j]);
                    break;
                case NOR:
                    returnSet.insert(NORTABLE[*i][*j]);
                    break;
                case XOR:
                    returnSet.insert(XORTABLE[*i][*j]);
                    break;
                case XNOR:
                    returnSet.insert(XNORTABLE[*i][*j]);
                    break;
                default:
                    cerr << "Unknown gate type encountered for a gate with two inputs." << endl;
                    cerr << "Possible values are 2,3,4,6,7. Current value is " << gateType << endl;
                    exit (0);
                    break;
            }
        }
    }
	//for(i = returnSet.begin(); i!=returnSet.end(); i++)
	//	cout << "returnSet = "<<*i<<endl<<endl;

    return returnSet;

}

vector <int>   forwardImplication (unsigned int gateType, vector<int> &input_a, vector<int> &input_b) {
	
	vector<int> returnSet;
    for (int i = 0; i < input_a.size(); i++) {
        for (int j = 0; j < input_b.size(); j++){
            switch (gateType) {
                case AND:
                    returnSet.push_back(ANDTABLE[input_a[i]][input_b[j]]);
                    break;
                case NAND:
                    returnSet.push_back(NANDTABLE[input_a[i]][input_b[j]]);
                    break;
                case OR:
                    returnSet.push_back(ORTABLE[input_a[i]][input_b[j]]);
                    break;
                case NOR:
                    returnSet.push_back(NORTABLE[input_a[i]][input_b[j]]);
                    break;
                case XOR:
                    returnSet.push_back(XORTABLE[input_a[i]][input_b[j]]);
                    break;
                case XNOR:
                    returnSet.push_back(XNORTABLE[input_a[i]][input_b[j]]);
                    break;
                default:
                    cerr << "Unknown gate type encountered for a gate with two inputs." << endl;
                    cerr << "Possible values are 2,3,4,6,7. Current value is " << gateType << endl;
                    exit (0);
                    break;
            }
        }
    }
	//for(i = returnSet.begin(); i!=returnSet.end(); i++)
	//	cout << "returnSet = "<<*i<<endl<<endl;

    return returnSet;

}

vector <int>   forwardImplication (unsigned int gateType, vector <int> &input_a, vector <int> &input_b, int wordSize) {

    vector <int> returnVector;
    vector <int>::iterator i, j;
    
   // redFILE1 << "Two inputs!"<<endl;
   // redFILE1 << "gate type: "<<gateType<<endl;

	//cout << "In forwardImplication:"<<endl;
    unsigned int value_a = 0 , value_b = 0, result;
	unsigned int len_a = input_a.size();
	unsigned int len_b = input_b.size();
	for(int i = 0; i < len_a; i++)
	{
	   // redFILE1 << input_a[i];
		value_a = value_a + input_a[i] * pow(2, len_a-1-i);
		if(value_a > pow(2, 64) - 1)
		{
		    cout << "a. exceeds the maximum bound."<<endl;
		    exit(1);
		}
	}
	//redFILE1 << "value_a = "<<value_a<<endl;
	for(int i = 0; i < len_b; i++)
	{
	   // redFILE1 << input_b[i];
		value_b = value_b + input_b[i] * pow(2, len_b-1-i);
		if(value_a > pow(2, 64) - 1)
		{
		    cout << "b. exceeds the maximum bound."<<endl;
		    exit(1);
		}
	}
	//redFILE1 << "value_b = "<<value_b<<endl;
		
            switch (gateType) {
                case AND:
                    result = value_a & value_b;
                    break;
                case OR:
                    result = value_a | value_b;
                    break;
                default:
                    cerr << "Unknown gate type encountered." << endl;
                    cerr << "Possible values are 2,3,4,6,7. Current value is " << gateType << endl;
                    exit (0);
                    break;
            }
    
   // redFILE1 << "result = "<<result<<endl;
    
    dtob(result, returnVector, wordSize);
    /*for(int i = 0; i < returnVector.size(); i++)
	{
	    redFILE1 << returnVector[i];
	}
	redFILE1 << endl;*/
    return returnVector;
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  forwardImlpication
 *  Description:  The function takes in an STL set as input and returns a set as output.
 *                The version defined below is a three input version.
 *                Set is used since multiple inputs may give same result.
 * =====================================================================================
 */
set <int>   forwardImplication (unsigned int gateType, set <int> &input_a, set <int> &input_b, set <int> &input_c) {

	//redFILE1 << "forwardImplication: 3 inputs."<<endl;
    set <int> returnSet;
    set <int>::iterator i, j;

    for (i = input_a.begin(); i != input_a.end(); i++) {
        for (j = input_b.begin(); j != input_b.end(); j++) {
            switch (gateType) {
                case AND:
                case NAND:
                    returnSet.insert(ANDTABLE[*i][*j]);
                    break;
                case OR:
                case NOR:
                    returnSet.insert(ORTABLE[*i][*j]);
                    break;
                case XOR:
                case XNOR:
                    returnSet.insert(XORTABLE[*i][*j]);
                    break;
                default:
                    cerr << "Unknown gate type encountered." << endl;
                    cerr << "Possible values are 2,3,4,6,7. Current value is " << gateType << endl;
                    exit (0);
                    break;
            }
        }
    }

    return (forwardImplication (gateType, input_c, returnSet));

}


//backwardImplication: one input
void  backwardImplication1 (unsigned int gateType, set <int> &output_a, set <int> &input_b) 
{

	//set<int> returnSet;
    set <int>::iterator i;

    for (i = output_a.begin(); i != output_a.end(); i++) {
        switch (gateType) {
            case INV:
            case NAND:
            case NOR:
            	input_b.clear();
            	input_b.insert(INVTABLE[*i]);
                break;
            case BRNCH:
            case BUFF:
            case AND:
            case OR:
            	input_b.clear();
            	input_b.insert(BRNCHTABLE[*i]);
                break;
            default:
                cerr << "Unknown gate type encountered for a gate with only 1 input." << endl;
                cerr << "Possible values are 0-7. Current value is " << gateType << endl;
                exit (0);
                break;
        }
    }
   //	return returnSet;
}

//backwardImplication: two inputs
void backwardImplication2(unsigned int gateType, set <int> &output_a, set <int> &input_b, set <int> &input_c) 
{
	//redFILE1 << "backwardImplication2: 2 inputs!"<<endl;
	//set<int> returnSet;
    set <int>::iterator itrs, itrs1, itrs2;

    for (itrs = output_a.begin(); itrs != output_a.end(); itrs++) 
    {
    	itrs1 = input_b.begin();
        itrs2 = input_c.begin();
        int num_X = 0;
            switch (gateType) 
            {
                case AND:
                	if(*itrs1 == X)
                		num_X++;
                	if(*itrs2 == X)
                		num_X++;
                	if(*itrs == 1)                    //output value = 1
                	{
                		input_b.clear();
                		input_b.insert(1);
                		input_c.clear();
                		input_c.insert(1);
                	
                	}
                	else if(*itrs == 0)                    //output value = 0
                	{
                		if(num_X == 2)
                		{
                			//do nothing
                		}
                		else if(num_X == 1)
                		{
                			if(*itrs1 == X && *itrs2 == 1)
                			{
                				input_b.clear();
                				input_b.insert(0);
                			}
                			else if(*itrs1 == 1 && *itrs2 == X)
                			{
                				input_c.clear();
                				input_c.insert(0);
                			}
                		}
                	}
                	else if(*itrs == D || *itrs == B)                 //output value = D/B
                	{
                		if(num_X == 2)
                		{
                			//do nothing
                		}
                		else if(num_X == 1)
                		{
                			if(*itrs1 == X && *itrs2 == 1)
                			{
                				input_b.clear();
                				input_b.insert(*itrs);
                			}
                			else if(*itrs1 == 1 && *itrs2 == X)
                			{
                				input_c.clear();
                				input_c.insert(*itrs);
                			}
                		}
                	}
                    break;
              
                case OR:
                    if(*itrs1 == X)
                		num_X++;
                	if(*itrs2 == X)
                		num_X++;
                	if(*itrs == 0)                                               //output value = 0
                	{
                		input_b.clear();
                		input_b.insert(0);
                		input_c.clear();
                		input_c.insert(0);
                	}
                	else if(*itrs == 1)                                         //output value = 1
                	{
                		if(num_X == 2)
                		{
                			//do nothing
                		}
                		else if(num_X == 1)
                		{
	                		if(*itrs1 == X && *itrs2 == 0)
	                		{
	                			input_b.clear();
	                			input_b.insert(1);
	                		}
	                		else if(*itrs1 == 0 && *itrs2 == X)
	                		{
	                			input_c.clear();
	                			input_c.insert(1);
	                		}
	                	}
                	}
                	else if(*itrs == D || *itrs == B)                 //output value = D/B
                	{
                		if(num_X == 2)
                		{
                			//do nothing
                		}
                		else if(num_X == 1)
                		{
                			if(*itrs1 == X && *itrs2 == 0)
                			{
                				input_b.clear();
                				input_b.insert(*itrs);
                			}
                			else if(*itrs1 == 0 && *itrs2 == X)
                			{
                				input_c.clear();
                				input_c.insert(*itrs);
                			}
                		}
                	}
                    break;
                    
                default:
                    cerr << "Unknown gate type encountered." << endl;
                    cerr << "Possible values are 2,3,4,6,7. Current value is " << gateType << endl;
                    exit (0);
                    break;
            }
    }

	//return returnSet;
}

int check_consistency(unsigned int gateType, set <int> &output, set <int> &input)
{
	set <int>::iterator itrs, itrs1;
	itrs = output.begin();
	itrs1 = input.begin();
	int flag = -1;
        switch (gateType) {
            case INV:
            	if(*itrs1 == X)
            		flag = 1;
            	else
	            {
	            	int correct = INVTABLE[*itrs];
	            	if(*itrs1 == correct)
	            		flag = 1;
	            	else flag = 0;
	            }
                break;
            case BRNCH:
            case BUFF:
            case AND:
            case OR:
            	if(*itrs1 == X)
            		flag = 1;
            	else
	            {
	            	int correct = BRNCHTABLE[*itrs];
	            	if(*itrs1 == correct)
	            		flag = 1;
	            	else flag = 0;
	            }
                break;
        }
    return flag;
}

int check_consistency(unsigned int gateType, set <int> &output, set <int> &input_a, set <int> &input_b, int num_known)
{
	set <int>::iterator itrs, itrs1, itrs2;
	itrs = output.begin();
	itrs1 = input_a.begin();
	itrs2 = input_b.begin();
	int flag = -1;
        switch (gateType) {
            case AND:
            	if(num_known == 0)
            		flag = 1;
            	else if(num_known == 2)
            	{
					int correct_output = ANDTABLE[*itrs1][*itrs2];
            		if(*itrs == correct_output)
            			flag = 1;
            		else flag = 0;
            	}
            	else if(num_known == 1)
            	{
            		if(*itrs1 == X)
            		{
            			for(int i = 0; i < 5; i++)
            				if(ANDTABLE[i][*itrs2] == *itrs)
            				{
            					flag = 1;
            					break;
            				}
            			if(flag != 1)
            				flag = 0;
            		}
            		else if(*itrs2 == X)
            		{
            			for(int i = 0; i < 5; i++)
            				if(ANDTABLE[*itrs1][i] == *itrs)
            				{
            					flag = 1;
            					break;
            				}
            			if(flag != 1)
            				flag = 0;
            		}
            	}
                break;
            case OR:
            	if(num_known == 0)
            		flag = 1;
            	else if(num_known == 2)
            	{
            		int correct_output = ORTABLE[*itrs1][*itrs2];
            		if(*itrs == correct_output)
            			flag = 1;
            		else flag = 0;
            	}
 				else if(num_known == 1)
            	{
            		if(*itrs1 == X)
            		{
            			for(int i = 0; i < 5; i++)
            				if(ORTABLE[i][*itrs2] == *itrs)
            				{
            					flag = 1;
            					break;
            				}
            			if(flag != 1)
            				flag = 0;
            		}
            		else if(*itrs2 == X)
            		{
            			for(int i = 0; i < 5; i++)
            				if(ORTABLE[*itrs1][i] == *itrs)
            				{
            					flag = 1;
            					break;
            				}
            			if(flag != 1)
            				flag = 0;
            		}
            	}
                break;
           
        }
    return flag;
}

