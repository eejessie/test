#include <iostream>
#include "queue.h"

using namespace std;

int main(int argc, char* argv[])
{
 queue myqueue;
 cout << myqueue.size() << endl;
 myqueue.push(10);
 myqueue.push(20);
 myqueue.push(30);
 cout << myqueue.front_element() << endl;
 cout << myqueue.back_element() << endl;
 //myqueue.pop();
 if (myqueue.empty())
 {
  cout << "The queue is empty now." << endl;
 }
 else
 {
  cout << "The queue has " << myqueue.size() << " elements now." << endl;
 }
 myqueue.pop();
 myqueue.pop();
 if (myqueue.empty())
 {
  cout << "The queue is empty now." << endl;
 }
 else
 {
  cout << "The queue has " << myqueue.size() << " elements now." << endl;
 }
 return 0;
}
