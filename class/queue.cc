#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "queue.h"

extern ofstream redFILE0;

using namespace std; 
	
//Constructor
queue::queue()                          
{  
   front = rear = new NODE; 
    if(!front) 
       cout << "Can't malloc new space"; 
    front->next=NULL; 
}  
	  
//Push into the queue
void queue::push(int e)  
{  
	NODE *p = front->next;
/*	int flag = 0;
	while(p != NULL)
	{
		if(p->data == e)
		{
			flag = 1;
			break;
		}
		p = p->next;
	}
	if(flag == 1)
		return;
	else*/
	{
		NODE* p = new NODE;  
		if (NULL == p)  
		{  
		    cout << "Failed to malloc the node." << endl;  
		}  
		p->data = e;  
		p->next = NULL;  
		rear->next = p;  
		rear = p; 
	} 
}  
	  
//Pop from the queue 
int queue::pop()  
{  
	int e;  
	if (front == rear)  
	{  
		cout << "The queue is empty." << endl;  
	    return -1;  
	}  
	else  
	{  
	    NODE* p = front->next;  
	    front->next = p->next;  
	    e = p->data;  
	    if (rear == p)  
	    {  
	        rear = front;  
	    }  
	    delete p;  
	    return e;  
	}  
}  
	  
//Get the front element
int queue::front_element()  
{  
	if (front == rear)  
	{  
	    cout << "The queue is empty." << endl;  
	    return -1;  
	}  
	else  
	{  
	    return front->next->data;  
	}  
}  
	  
	  	//Get the back element
int queue::back_element()  
{  
	if (front == rear)  
	{  
	    cout << "The queue is empty." << endl;  
	    return -1;  
	}  
	else  
	{  
	    return rear->data;  
	}  
}  
	      
	    //Get the size of the queue  
int queue::size()  
{  
	int count(0);  
	  
	NODE* p = front;  
	  
	while (p != rear)  
	{  
	    p = p->next;  
	    count++;  
	}  
	return count;  
}  
	      
	    //Judge if the queue is empty  
bool queue::empty()  
{  
	if (front == rear)  
	{  
	    return true;  
	}  
	else  
	{  
	    return false;  
	}  
}  

void queue::traverse()
{
	NODE *p = front->next;
	while(p != NULL)
	{
		redFILE0 << p->data <<" ";
		p = p->next;
	}
	redFILE0 << endl;
}
	
	
