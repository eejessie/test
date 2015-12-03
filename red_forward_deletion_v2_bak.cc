/*
 * =====================================================================================
 *
 *       Filename:  red_forward_deletion.cc
 *
 *    Description:  This function performs forward implication on a 1-3 input gate.
 *                  The inputs and output are integer sets. It reads the values from
 *                  logic tables.
 *
 *        Version:  1.0
 *        Created:  3/07/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yi Wu
 *
 * =====================================================================================
 */

// #####   HEADER FILE INCLUDES   ################################################### 

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

#include "class/CircuitNode.h" 
#include "class/HashTable.h"        
#include "function/red_forward_implication_v2.h"
#include "function/atpg.h"  
#include "function/print_circuit.h"
#include "function/helper.h"

using namespace std;


// #####   HEADER FILE INCLUDES   ################################################### 

// This file contains the tables which are looked up for determinig the output.

// Global constant definitions.

#ifndef GLOBAL_DEFINES_H_
#include "include/global_defines.h"
#endif

extern ofstream redFILE0, redFILE1;
extern vector<int> outputList;
extern HashTable opl_hash;


void sel_highest(map<int, CircuitNode> &simNodeList, HashTable &implicationNodeList, vector <int> &highestNodeList)
{
	int i, level=0;
	map<int, CircuitNode>::iterator itrm;
	//redFILE1 << "In sel_highest."<<endl;
	for (i=0; i < implicationNodeList.Get_Size(); i++)
	{
		if(implicationNodeList.Get_Data(i) == -1 || implicationNodeList.Get_Data(i) == -2)
			continue;
		itrm = simNodeList.find(implicationNodeList.Get_Data(i));
		if (itrm->second.lineLevel > level)
			level = itrm->second.lineLevel;
	}
	
	//Get the number of nodes on highest level in the implication node list and store them in "highestNodeList".
	for (i=0; i < implicationNodeList.Get_Size(); i++)
	{
		if(implicationNodeList.Get_Data(i) == -1 || implicationNodeList.Get_Data(i) == -2)
			continue;
		itrm = simNodeList.find(implicationNodeList.Get_Data(i));
		//redFILE1 << implicationNodeList.Get_Data(i)<<endl;
		if(itrm->second.lineLevel == level)
			highestNodeList.push_back(itrm->second.lineNumber);
	}
}


void DFS_sim(map <int, CircuitNode> &simNodeList, HashTable &implicationNodeList, CircuitNode &currentNode) 
{
	int i, j, level = 0, count = 0, level1 = 0, count1 = 0;
	int sear=0, flag = 0, rep = 0, con = 0;
	vector <int> tempFanIn;
	int currentNodeValue;
	int linevalue_fanin[2];
	vector<int>::iterator itrv0;
	vector<CircuitNode>::iterator itrv;
	vector<CircuitNode*>::iterator itrv1;
	map<int, CircuitNode>::iterator itrm, itrm1, itrm2, itrm3, itrm4;
	set<int>::iterator itrs, itrs1, itrs2, itrs3;
	vector <CircuitNode> highestNodeList;
	int p;
	
		
	//Get the lineValue of the current node and store it in "currentNodeValue"
	itrs1 = currentNode.lineValue.begin();
	currentNodeValue = *itrs1;

    //Save the input nodes of the current node in tempFanIn. 
	for(itrv1 = currentNode.pointFanIn.begin(); itrv1 !=currentNode.pointFanIn.end(); itrv1++)
		tempFanIn.push_back((*itrv1)->lineNumber);
    

	//A currentNode may be of type "gate", "PO" and "branch". 
	for (i=0; i<tempFanIn.size(); i++)
	{
		redFILE1 << "fanin: "<<tempFanIn[i]<<" ";
		itrm = simNodeList.find(tempFanIn[i]);
		if(itrm == simNodeList.end())
		    redFILE1 << "no this fanin!"<<endl;
		itrs2 = itrm->second.lineValue.begin();
		linevalue_fanin[i] = *itrs2;
	}
	redFILE1 << endl;
        
	if(tempFanIn.size()==1)     
		itrm1 = simNodeList.find(tempFanIn[0]);
	else if(tempFanIn.size()==2)     
	{
		itrm1 = simNodeList.find(tempFanIn[0]);  //The first input node of the current node
		itrm2 = simNodeList.find(tempFanIn[1]);  //The second input node of the current node
	}  
	itrm3 = simNodeList.find(currentNode.lineNumber);       //itrm3: currentNode
	switch(currentNode.nodeType)
	{		
		case PO:                        //nodeType of current node is PO.
			redFILE0 << "Current node is a PO."<<endl;				
			if (currentNodeValue == 0 || currentNodeValue == 1)
			{				
				itrm3->second.gateType =  ( currentNodeValue == 0 ? G_stat0 : G_stat1 );
				itrm3->second.numberFanIn = 0;
				itrm3->second.listFanIn.clear();
				itrm3->second.pointFanIn.clear();
				flag = 1;
			}
			else if ( currentNodeValue == X )
			{
				if (itrm3->second.pointFanIn.size()==1) {} //gateType of current node is an NOT gate or BRANCH. We can do nothing.
				else if (itrm3->second.pointFanIn.size()==2)
				{					
					switch(currentNode.gateType)
					{                   
						case G_AND:        //gateType of current node is an AND gate.
							
							//simNodeList.erase(currentNode.lineNumber);	
							itrm = simNodeList.find(currentNode.lineNumber);
							itrm->second.gateType = 1;
							//flag = 1;		
							//redFILE1 << "simNodeList deletes node "<<currentNode.lineNumber<<endl;
																							
							/*if (linevalue_fanin[0] == 1)								
								itrm2->second.nodeType = PO;													
							else if (linevalue_fanin[1] == 1)
								itrm1->second.nodeType = PO;*/	
							break;

						case G_OR:                                  //gateType of current node is an OR gate.
							//cout<<"		gateType: OR."<<endl;
							simNodeList.erase(currentNode.lineNumber);
							flag = 1;	
							//redFILE1 << "simNodeList deletes node "<<currentNode.lineNumber<<endl;
							//cout<<"		current node "<<currentNode.lineNumber<<" is erased."<<endl;	
							
							if (linevalue_fanin[0] == 0)
								itrm2->second.nodeType = PO;
							else if (linevalue_fanin[1] == 0)
								itrm1->second.nodeType = PO;		
							break;
					}			
				}					
			}
			break;
			
		case GT:                                                   //nodeType of current node is GT.
			//redFILE0 << "Current node is a GT."<<endl;
			if ((currentNodeValue == 0 || currentNodeValue == 1))
			{
				//Current gate is not in outputList.
				if( opl_hash.Search_Hash(currentNode.lineNumber, p) == 0)
				{
					simNodeList.erase(currentNode.lineNumber);
					flag = 1;
					//redFILE1 << "0.simNodeList deletes node "<<currentNode.lineNumber<<endl;
					/*If there are still nodes in simNodeList that are fanouts of the current node, 
					then clear the listFanIn of these nodes.*/
					for (itrm = simNodeList.begin(); itrm != simNodeList.end(); itrm++)
						for (itrs3 = itrm->second.listFanIn.begin(); itrs3 != itrm->second.listFanIn.end(); itrs3++)
						{
							if (*itrs3 == currentNode.lineNumber)
							{
								itrm->second.listFanIn.erase(currentNode.lineNumber); 
								vector_erase(itrm->second, currentNode.lineNumber, 1);
								break;
							}        
						}
				}
				else //Current gate is in outputList.
				{
					itrm3->second.listFanIn.clear(); 
					itrm3->second.pointFanIn.clear();
					if(currentNodeValue == 0)
						itrm3->second.gateType = 9;
					else if(currentNodeValue == 1)
						itrm3->second.gateType = 10;
					flag = 1;					
				}
				
				
			}
			else if ((currentNodeValue == X))
			{
				if (currentNode.pointFanIn.size() == 1){}
				else if (currentNode.pointFanIn.size() == 2)
				{               	
					switch(currentNode.gateType)
					{
						case G_AND:    //gateType of current node is an AND gate.
							//cout<<"		gateType: AND."<<endl;
							if (linevalue_fanin[0] == 1 || linevalue_fanin[1] == 1)
							{
								if( opl_hash.Search_Hash(currentNode.lineNumber, p) == 0)
								{
									simNodeList.erase(currentNode.lineNumber);
									flag = 1;
									//redFILE1<< "1.simNodeList deletes node "<<currentNode.lineNumber<<endl;							
									
									/*If there are still nodes in simNodeList that are fanouts of the current node, 
									then update the listFanIn of these nodes.*/
									for (itrm = simNodeList.begin(); itrm != simNodeList.end(); itrm++)
									{
										for (itrs3 = itrm->second.listFanIn.begin(); itrs3 != itrm->second.listFanIn.end(); itrs3++)
										{
											if (*itrs3 == currentNode.lineNumber)
											{
												itrm->second.listFanIn.erase(currentNode.lineNumber); 
												vector_erase(itrm->second, currentNode.lineNumber, 1);
												if (linevalue_fanin[0] == 1)
												{		
													itrm->second.listFanIn.insert(itrm2->second.lineNumber); 
													vector_insert(itrm->second, itrm2->second.lineNumber, 1, simNodeList);
													vector_insert(itrm2->second, itrm->second.lineNumber, 2, simNodeList);
												}
												else if(linevalue_fanin[1] == 1)
												{
													itrm->second.listFanIn.insert(itrm1->second.lineNumber); 
													vector_insert(itrm->second, itrm1->second.lineNumber, 1, simNodeList); 
													vector_insert(itrm1->second, itrm->second.lineNumber, 2, simNodeList);   
												}   
											}       
										}
									}		
								}
								else 
								{
									if(linevalue_fanin[0] == 1)
									{
										itrm3->second.listFanIn.erase(itrm1->second.lineNumber); 
										vector_erase(itrm3->second, itrm1->second.lineNumber, 1);
									}
									else if(linevalue_fanin[1] == 1)
									{
										itrm3->second.listFanIn.erase(itrm2->second.lineNumber); 
										vector_erase(itrm3->second, itrm2->second.lineNumber, 1);
									}	
									itrm3->second.gateType = 11;
									itrm3->second.numberFanIn = 1;
								}		
							}
							break;
						case G_OR:                                     //gateType of current node is an OR gate.
							//cout<<"		gateType: OR."<<endl;
							if (linevalue_fanin[0] == 0 || linevalue_fanin[1] == 0)
							{
								if( opl_hash.Search_Hash(currentNode.lineNumber, p) == 0)
								{
									simNodeList.erase(currentNode.lineNumber);
									flag = 1;
									//redFILE1 << "2.simNodeList deletes node "<<currentNode.lineNumber<<endl;
									
									/*If there are still nodes in simNodeList that are fanouts of the current node, 
									then update the listFanIn of these nodes.*/
									for (itrm = simNodeList.begin(); itrm != simNodeList.end(); itrm++)
									{
										for (itrs3 = itrm->second.listFanIn.begin(); itrs3 != itrm->second.listFanIn.end(); itrs3++)
										{
											if (*itrs3 == currentNode.lineNumber)
											{
												itrm->second.listFanIn.erase(currentNode.lineNumber); 
												vector_erase(itrm->second, currentNode.lineNumber, 1);
												if (linevalue_fanin[0] == 0)
												{	
													itrm->second.listFanIn.insert(itrm2->second.lineNumber); 
													vector_insert(itrm->second, itrm2->second.lineNumber, 1, simNodeList);
													vector_insert(itrm2->second, itrm->second.lineNumber, 2, simNodeList);
												}
												else if(linevalue_fanin[1] == 0)    
												{
													itrm->second.listFanIn.insert(itrm1->second.lineNumber);     
													vector_insert(itrm->second, itrm1->second.lineNumber, 1, simNodeList);  
													vector_insert(itrm1->second, itrm->second.lineNumber, 2, simNodeList);     
												}    
											}  
										}	
									}
								}
								else 
								{
									if(linevalue_fanin[0] == 0)
									{
										itrm3->second.listFanIn.erase(itrm1->second.lineNumber); 
										vector_erase(itrm3->second, itrm1->second.lineNumber, 1);
									}
									else if(linevalue_fanin[1] == 0)
									{
										itrm3->second.listFanIn.erase(itrm2->second.lineNumber); 
										vector_erase(itrm3->second, itrm2->second.lineNumber, 1);
									}
									itrm3->second.gateType = 11;
									itrm3->second.numberFanIn = 1;
									
									
								}  
							}
							break;
					}
				}
			}
			break;

		case FB:                                                      //nodeType of current node is FB.
			//redFILE0 << "Current node is a FB."<<endl;
			if (currentNodeValue == 0 || currentNodeValue == 1 || currentNode.pointFanOut.empty())
			{	
				if( opl_hash.Search_Hash(currentNode.lineNumber, p) == 0)
				{
					simNodeList.erase(currentNode.lineNumber); 
					flag = 1;				
					//redFILE1 << "3. simNodeList deletes node "<<currentNode.lineNumber<<endl; 
					
					/*If there are still nodes in simNodeList that are fanouts of the current node, 
						then clear the listFanIn of these nodes.*/
					for (itrm = simNodeList.begin(); itrm != simNodeList.end(); itrm++)
						for (itrs3 = itrm->second.listFanIn.begin(); itrs3 != itrm->second.listFanIn.end(); itrs3++)
						{
							if (*itrs3 == currentNode.lineNumber)
							{
								itrm->second.listFanIn.erase(currentNode.lineNumber); 
								vector_erase(itrm->second, currentNode.lineNumber, 1);
								break;
							}        
						}
				}
				else 
				{
					itrm3->second.listFanIn.clear(); 
					itrm3->second.pointFanIn.clear();
					if(currentNodeValue == 0)
						itrm3->second.gateType = 9;
					else if(currentNodeValue == 1)
						itrm3->second.gateType = 10;
					itrm3->second.nodeType = 0;
					flag = 1;					
				}								
			}			
			break;

		case PI:
			break;
		
	}
	
	//Erase the current node from the fanout list of the current node's fanins
	if (flag == 1)
	{
		if(tempFanIn.size() == 0) {}
		else if (tempFanIn.size() == 1)
		{
			vector_erase(itrm1->second, currentNode.lineNumber, 2);
			itrm1->second.numberFanOut = itrm1->second.pointFanOut.size();
		}
		else if (tempFanIn.size() == 2)
		{
			vector_erase(itrm1->second, currentNode.lineNumber, 2);
			itrm1->second.numberFanOut = itrm1->second.pointFanOut.size();
			vector_erase(itrm2->second, currentNode.lineNumber, 2);
			itrm2->second.numberFanOut = itrm2->second.pointFanOut.size();
		}
		
	}

	//Delete the current node from the implication node list.
	/*for(itrv0 = implicationNodeList.begin(); itrv0 != implicationNodeList.end(); itrv0++)  
		if(*itrv0 == currentNode.lineNumber)
		{
			implicationNodeList.erase(itrv0);
			break;			
		}*/
	bool res = implicationNodeList.Search_Hash(currentNode.lineNumber, p);
	if(res == true)
		implicationNodeList.Del_Data(p);
		
	//implicationNodeList.Traverse_HashTable();

	
	/*termination condition: if none of the inputs to the current node is in implication node list(con = 0), return.*/
	for (i=0; i<tempFanIn.size(); i++)
	{
		int p;
		bool res = implicationNodeList.Search_Hash(tempFanIn[i], p);
		if(res == true)
		{
			con = 1;
			break;
		}
		if (con == 1)
			break;		
	}

	if (con == 1)
	{
		for (i=0; i<tempFanIn.size(); i++)
		{	
			itrm = simNodeList.find(tempFanIn[i]);
			DFS_sim (simNodeList, implicationNodeList, itrm->second);
		}					
	}
	else 
		return;

}

void red_forward_deletion_v2(map <int, CircuitNode> &simNodeList, HashTable &implicationNodeList, int red_line, bool red_stuck)
{
	
	int i, level = 0;
	CircuitNode currentNode(GT);
	vector<int>::iterator itrv0;
	vector<CircuitNode>::iterator itrv;
	vector<CircuitNode*>::iterator itrv1;
	map<int, CircuitNode>::iterator itrm, itrm1;
	set<int>::iterator itrs, itrs1, itrs2;
	vector <int> highestNodeList;


	/*************************************/
	/*redundancy forward implication*/
	/*************************************/
	red_forward_implication_v2(simNodeList, implicationNodeList, red_line, red_stuck);
	redFILE1 << "-------------------------------------------"<<endl;
	redFILE1 <<"Before adding, the nodes in implicationNodeList are:"<<endl;
	for (i=0; i<implicationNodeList.Get_Size(); i++)
	{
		if(implicationNodeList.Get_Data(i) == -1 || implicationNodeList.Get_Data(i) == -2)
			continue;
		redFILE1 <<implicationNodeList.Get_Data(i)<<" ";
	}
	redFILE1<<endl;
	
	/*For those nodes not in implicationNodeList but who have inputs in implicationNodeList , 
	find them and push them into implicationNodeList.*/
	//vector<int> tempNodeList = implicationNodeList;
	HashTable tempNodeList;
	tempNodeList.Init_HashTable();
	for(int i = 0; i < implicationNodeList.Get_Size(); i++)
		if(implicationNodeList.Get_Data(i) != -1 && implicationNodeList.Get_Data(i) != -2)
		{
			int res = tempNodeList.Insert_Hash(implicationNodeList.Get_Data(i));
			while(res == 0)
				res = tempNodeList.Insert_Hash(implicationNodeList.Get_Data(i));
		}
		
	//redFILE1 << "tempNodeList:"<<endl;
	//tempNodeList.Traverse_HashTable();
	for (i = 0; i < tempNodeList.Get_Size(); i++)
	{
		if(tempNodeList.Get_Data(i) == -1 || tempNodeList.Get_Data(i) == -2)
			continue;
		itrm = simNodeList.find(tempNodeList.Get_Data(i));
		for(itrv1 = itrm->second.pointFanOut.begin(); itrv1 != itrm->second.pointFanOut.end(); itrv1++)
		{
			itrm1 = simNodeList.find((*itrv1)->lineNumber);		
			int p;
			if(implicationNodeList.Search_Hash(itrm1->second.lineNumber, p) == 0)
			{
				int res2 = implicationNodeList.Insert_Hash(itrm1->second.lineNumber);
				while(res2 == 0)	
					res2 = implicationNodeList.Insert_Hash(itrm1->second.lineNumber);
			}
							
		}
	}
	tempNodeList.Destroy_HashTable();
	
	redFILE1 << "-------------------------------------------"<<endl;
	redFILE1 <<"After adding, the nodes in implicationNodeList are:"<<endl;
	for (i=0; i<implicationNodeList.Get_Size(); i++)
		if(implicationNodeList.Get_Data(i) != -1 && implicationNodeList.Get_Data(i) != -2) 
			redFILE1<<implicationNodeList.Get_Data(i)<<" ";
	redFILE1<<endl;

   while(!implicationNodeList.empty())
   {
		//Get the node with minimum linenumber on highest level and store it in "currentNode".
		SetLineLevel(simNodeList);
		sel_highest(simNodeList, implicationNodeList, highestNodeList);		
		insertion_sort_asc(highestNodeList, highestNodeList.size());
		redFILE1 << "Nodes in highestNodeList: "<<endl;
		for(i = 0; i<highestNodeList.size(); i++)
			redFILE1 << highestNodeList[i]<<" ";
		redFILE1 << endl;		
		print_circuit(simNodeList, redFILE1);
		for(itrv0 =highestNodeList.begin(); itrv0 != highestNodeList.end(); itrv0++)
		{
		 	redFILE1 << "currentNode is "<< *itrv0<<endl;
			itrm = simNodeList.find(*itrv0);			
			DFS_sim(simNodeList, implicationNodeList, itrm->second);
		}
		highestNodeList.clear();
	}

	
}



