#include<bits/stdc++.h>
using namespace std;
int main(){
    string temp = "HELLO";
    const char* temp_A = temp.data();

    cout<<"string: "<<temp<<endl<<"const char*: "<<temp_A<<endl;
    cout<<"After changes:"<<endl;
    
    temp = "OLLEH";
    cout<<"string: "<<temp<<endl<<"const char*: "<<temp_A<<endl;
    return 0;
}