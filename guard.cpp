#include <iostream>
#include <thread>

using namespace std;

class Guard { 
  private:            
    pthread_mutex_t *lock;
    public:
    Guard(pthread_mutex_t* lock) {  
    this->lock = lock;
    pthread_mutex_lock(this->lock);
    }
    ~Guard() { pthread_mutex_unlock(this->lock); } 
};