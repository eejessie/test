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
#include <ctime>
#include <sys/timeb.h>

#include "class/CircuitNode.h"         
#include "class/CircuitLine.h"        
#include "class/FaultList.h"           
#include "class/TestList.h"            
#include "class/ConnFault.h"
#include "lib/file_operations.h"
#include "lib/radix_convert.h"


#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

using namespace std;

extern ofstream redFILE0, redFILE1;

int sim_br(map<int, int> &truthTable, vector<int> &inputVector)
{
    int int_value = 0;
    
    for(int i = 0; i < inputVector.size(); i++)
        int_value += inputVector[i] * pow(2, inputVector.size()-1-i);	
        
    //redFILE1 <<"int_value = "<<int_value<<endl;
      
    map<int, int>::iterator itrmi;
    itrmi = truthTable.find(int_value);
    if(itrmi != truthTable.end())  
        return 1;
    else 
        return 0;
}

