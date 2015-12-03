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

#include "function/read_circuit.h"
#include "function/merge_circuit.h"
#include "function/atpg.h"
#include "function/gen_cf.h"
#include "function/red_addition.h"
#include "function/red_removal_v2.h"
#include "function/print_circuit.h"
#include "function/run_logic_simulation.h"
#include "function/copy_point_vector.h"
#include "function/adjust_ln.h"
#include "function/helper.h"


using namespace std;

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

extern vector<string> masterInputVector;
extern int totalInputs;
extern ofstream redFILE0, redFILE1, timeRecord;

void opt_rar_v1(map<int, CircuitNode> &orNodeList_m, map<int, CircuitNode> &brNodeList_m, map<int, CircuitNode> &masterNodeList_m, int threshold, map<int, int> &truthTable)
{
    //Iterators
	vector<CircuitNode*>::iterator itrv;
	map<int, CircuitNode>::iterator itrm;
	
	map<int, CircuitNode> buOrNodeList_m;
	copy_point_vector1(orNodeList_m, buOrNodeList_m);
	
	struct timeb startTime, endTime;                         //Record the computing time.
	struct timeb startTime1, endTime1;                         //Record the computing time.
	ftime(&startTime1);


	//Generate testing vectors
	redFILE1 << "totalInputs = "<<totalInputs<<endl;
	vector<string> masterInputVector1;
	GenerateMasterInputVectors(masterInputVector1, totalInputs+1);	 
 
	//Generate a fault list of connection faults and store them in a vector "cf".
	redFILE1 <<"****************************"<<endl;
	redFILE1 << "generating CF phase starts!"<<endl;
	redFILE1 <<"****************************"<<endl;
	
	ftime(&startTime);
	vector<ConnFault> cf;                              //Store the possible connection faults.	
	int count_cf = gen_cf(orNodeList_m, cf, totalInputs);
	redFILE1 << "# total CF: "<<count_cf<<endl;
	
	ftime(&endTime);	
	timeRecord.open ("time_record.log", ofstream::app);
	timeRecord << "generating CF: "<< ((endTime.time - startTime.time)*1000 + (endTime.millitm - startTime.millitm))/1000.0<< endl;
	
    redFILE1 <<"*****************************"<<endl;
	redFILE1 << "generating CF phase is done!"<<endl;
	redFILE1 <<"*****************************"<<endl;
	
	
	
	//Randomly sort the cf array.
	srand((unsigned)time(0));
	/*for(int i = 0;i < cf.size(); i++)
	{ 
		int rand_index = rand() % cf.size();
		ConnFault temp;
		temp = cf[i];
		cf[i] = cf[rand_index];
		cf[rand_index] = temp;
	}*/
	/*redFILE1 <<" connection faults after random sorting:"<<endl;
	for(int i=0; i<cf.size(); i++)
		redFILE1 << "("<<cf[i].src<<","<<cf[i].dest<<","<<cf[i].pola<<")\t";
	redFILE1 << endl;*/


	map<int, CircuitNode> lastOrNodeList_m, lastMasterNodeList_m;	
	copy_point_vector1(orNodeList_m, lastOrNodeList_m);	
	copy_point_vector1(masterNodeList_m, lastMasterNodeList_m);

		
	redFILE1 <<"threshold = "<<threshold<<endl;	
	vector<string> testPool;
	
	//Enter into the big loop.
	static double addTime = 0;
	static double remTime = 0;
	int round = 0;
	while(!cf.empty())
	{
		round++;			
		redFILE1 <<"******************************************************"<<endl;
		redFILE1 << "Round "<<round<<" starts!"<<endl;	
		redFILE1 <<"******************************************************"<<endl;
		
		redFILE1 <<"**********************************"<<endl;
		redFILE1 << "redundancy addition phase starts!"<<endl;
		redFILE1 <<"**********************************"<<endl<<endl;

		//Print the connection faults at the beginning of red_addition.
		redFILE1 << "At the beginning of round"<<round<<",  # cf is "<<cf.size()<<endl<<endl;

		//Print circuits before red_addition.
		redFILE1 << "In round "<<round<<", before red_addition:"<<endl;
		redFILE1 << "The original circuit size is "<<orNodeList_m.size()<<endl;
		//print_circuit(orNodeList_m);
		itrm = masterNodeList_m.end();
		itrm--;
		int maxLineNumber = itrm->second.lineNumber;

		ftime(&startTime);
		red_addition(orNodeList_m, brNodeList_m, cf, threshold, testPool, truthTable, maxLineNumber);
		ftime(&endTime);		
		addTime = addTime + ((endTime.time - startTime.time)*1000 + ((endTime.millitm - startTime.millitm)))/1000.0;
		timeRecord << endl << "##round "<<round<<", red_addition: "<<((endTime.time - startTime.time)*1000 + ((endTime.millitm - startTime.millitm)))/1000.0<<endl;		 
		
		//Print circuits after red_addition.
		redFILE1 <<"In round "<<round<<", after red_addtion:"<<endl;
		redFILE1 << "The original circuit size is "<<orNodeList_m.size()<<endl;
		//print_circuit(orNodeList_m, redFILE1);

		//Run logic simulation on the whole circuit after red_addition.
		RunLogicSimulation(orNodeList_m, totalInputs, masterInputVector, truthTable);
		
		
		redFILE1 <<"*********************************"<<endl;
		redFILE1 << "In round "<<round<<", redundancy removal phase starts!"<<endl;
		redFILE1 <<"*********************************"<<endl<<endl;
        merge_circuit(masterNodeList_m, orNodeList_m, brNodeList_m);
		int last_size = lastOrNodeList_m.size();

		redFILE1 <<"In round "<<round<<", before red_removal:"<<endl;
		redFILE1 << "The original circuit size is "<<orNodeList_m.size()<<endl;
		//print_circuit(orNodeList_m);

		redFILE1 << "Enter into red_removal:"<<endl;	
		ftime(&startTime);
		red_removal_v2(buOrNodeList_m, orNodeList_m, brNodeList_m, masterNodeList_m, truthTable);
		ftime(&endTime);	
		remTime = remTime + ((endTime.time - startTime.time)*1000 + ((endTime.millitm - startTime.millitm)))/1000.0;	
		timeRecord << endl << "##round "<<round<<", red_removal: "<<((endTime.time - startTime.time)*1000 + ((endTime.millitm - startTime.millitm)))/1000.0<<endl;	

		redFILE1 << "In round "<<round<< ", after red_removal, circuit size is "<<orNodeList_m.size()<<endl<<endl;
		//print_circuit(orNodeList_m, redFILE1);
		timeRecord << "##After round "<<round<<", circuit size = "<<orNodeList_m.size()<<endl;

		redFILE1 <<"**********************************"<<endl;
		redFILE1 << "redundancy removal phase is done!"<<endl;
		redFILE1 <<"**********************************"<<endl<<endl;
		
		//RunLogicSimulation(orNodeList_m, totalInputs, masterInputVector, truthTable);

		//After this round, record the current number of connections in the circuit
		redFILE1 << "At the end of round "<<round<<endl;		
		redFILE1 << "last_size = "<<last_size<<endl;
		int new_size = orNodeList_m.size(); 
		redFILE1 << "new_size = "<<new_size<<endl;
			
		if(new_size >= last_size)
		{
			copy_point_vector1(lastOrNodeList_m, orNodeList_m);
			copy_point_vector1(lastMasterNodeList_m, masterNodeList_m);
		}
		else
		{
			copy_point_vector1(orNodeList_m, lastOrNodeList_m);
			copy_point_vector1(masterNodeList_m, lastMasterNodeList_m);
		}

		redFILE1 <<"*******************************************************************"<<endl;	
		redFILE1 << "Round "<<round<<" ends!"<<endl;
		redFILE1 <<"*******************************************************************"<<endl;	

	}//while
	

	redFILE1 << "Final result:"<<endl;
	redFILE1 << "The original circuit size is "<<orNodeList_m.size()<<endl;
	cout << "The original circuit size is "<<orNodeList_m.size()<<endl;
	cout << "round = "<<round<<endl;
	print_circuit(orNodeList_m, redFILE1);

	//RunLogicSimulation(orNodeList_m, totalInputs, masterInputVector, truthTable);	

	ftime(&endTime1);
	timeRecord << endl<<"circuit size = "<<orNodeList_m.size()<<endl;
	timeRecord << "round = "<<round<<endl;
	timeRecord << "total runtime:"<<((endTime1.time - startTime1.time)*1000 + ((endTime1.millitm - startTime1.millitm)))/1000.0<<endl;	
	timeRecord <<"total red_addition time:"<<addTime<<endl;	
	timeRecord <<"total red_removal time:"<<remTime<<endl;	
	
	cout << endl <<"total runtime:"<<((endTime1.time - startTime1.time)*1000 + ((endTime1.millitm - startTime1.millitm)))/1000.0<<endl;	
	cout <<"total red_addition time:"<<addTime<<endl;	
	cout <<"total red_removal time:"<<remTime<<endl;	
	
	return;
}
