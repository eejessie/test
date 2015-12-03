#include<iostream>  
using namespace std;  
  
void printbinary(const unsigned int val)  
{  
    for(int i = 16; i >= 0; i--)  
    {  
        if(val & (1 << i))  
            cout << "1";  
        else  
            cout << "0";  
    }  
    cout << endl;
}  
  
int main()  
{  
    printbinary(15);  
    return 0;  
}  
