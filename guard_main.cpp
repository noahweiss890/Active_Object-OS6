#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mutex>
#include <thread>
#include <iostream>
#include "guard.cpp"

using namespace std;

pthread_mutex_t lock_mut = PTHREAD_MUTEX_INITIALIZER;

int num = 0;
int* num_ptr = &num;

//the use of the lock proves that only once the first thread has finished looping and updating the number,
//only then will the second thread be allowed to enter the loop
void* update_num( void *ptr )
{
    Guard g(&lock_mut);
    for (int i = 0; i < 100000; i++)
    {
        pthread_t p_id = pthread_self();
        (*num_ptr)++;
        cout << p_id << ": " << *num_ptr << endl;
    } 
    return nullptr;
}
int main(int argc, char const *argv[])
{
     pthread_t thread1, thread2;
     char message1[] = "Thread 1";
     char message2[] = "Thread 2";

    /* Create 2 independent threads both executing the update_num function */
     pthread_create( &thread1, NULL, update_num, (void*) message1);
     pthread_create( &thread2, NULL, update_num, (void*) message2);

     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL); 
     return 0;
}