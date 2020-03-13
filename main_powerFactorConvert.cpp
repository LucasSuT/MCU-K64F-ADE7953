/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>

using namespace std;

int main()
{
    int a=0x8001;
    if(a<32768)printf("%f\n",a/32767.0);
    else 
    {
        //short b=a;
        printf("%f\n",(short)a/32768.0);
    }
    return 0;
}