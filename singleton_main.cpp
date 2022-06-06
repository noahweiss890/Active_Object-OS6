#include <pthread.h>
#include <stdio.h>
#include <mutex>
#include <iostream>
#include "singleton.cpp"

using namespace std;

template <typename T>
Singleton<T> *Singleton<T>::instance_ = nullptr;


//tester using a Singleton of type FILE
int main(){
    FILE *pointer_to_file;
    pointer_to_file = fopen("singleton_test.txt","w");
    if(pointer_to_file == NULL)
    {
        cout << "Error opening file!" << endl;   
        exit(1);             
    }
    else{
        cout << "File created successfully!" << endl;
        Singleton<FILE*> *singleton_1 = Singleton<FILE*>::Instance(pointer_to_file); 
        Singleton<FILE*> *singleton_2 = Singleton<FILE*>::Instance(pointer_to_file);
        //print out location to show the location is the same
        cout << singleton_1 << endl;
        cout << singleton_2 << endl;
        if(singleton_1 == singleton_2){
            cout<<"Reached the same instance! GOOOD!"<< endl;
        }
        else{
            cout<<"Multiple instances! BAAAD!"<< endl;
        }  
        fclose(pointer_to_file); //close file
        singleton_1->Destroy(); //destroy both singletons
        singleton_2->Destroy();         
    }
    return 0;
}