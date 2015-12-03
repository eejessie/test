#include <iostream>

using namespace std;  

#ifndef QUEUE_H
#define QUEUE_H

   
struct NODE  
{  
    NODE* next;  
    int data;  
};  
  
class queue
{  
	private:  
    	NODE* front; 
    	NODE* rear;
    	
	public:  
		queue();
		void push(int e);
		int pop();
		int front_element();
		int back_element();
		int size();
		bool empty();
		void traverse();
		
};  

#endif
