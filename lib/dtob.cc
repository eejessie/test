#include<iostream>  
#include <cstring>
#include <vector>
#include <cstdlib>
#include <cmath>

using namespace std; 

void dtob(int d, vector<int> &bstr)
{
    //if (d < 0)
    //    return -1;
 
    int mod = 0;
    vector <int> tmpstr;
    //bzero(tmpstr, sizeof(tmpstr));
    //bzero(bstr, sizeof(bstr));
 
    int i = 0;
    while (d > 0)
    {
        mod = d % 2;
        d /= 2;
        tmpstr.push_back(mod);
        i++;
    }
 
    //copy the string
    unsigned int len = tmpstr.size();
    for (i = len-1; i >= 0; i--)
    {
        bstr.push_back(tmpstr[i]);
    }
 	for(i = 0; i < bstr.size(); i++)
 		cout << bstr[i];
 	cout << endl;
}

/*int main(int argc, char *argv[])
{
	int d = atoi(argv[1]);
	vector <int> bstr;
	dtob(d, bstr);
	
	return 0;
}*/
