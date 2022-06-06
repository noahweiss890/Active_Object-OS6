#include <iostream>
#include <pthread.h>

pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

template < typename T >
class Singleton{
    private:
        static Singleton* instance_; //instance is private, must use Instance() method to access instance which is locked
    protected:
        T value_; //template value of what is inside Singleton
        //constuctor and deconstructor are not public
        Singleton(T value): value_(value) {}
        ~Singleton() {}
    public:
        static Singleton *Instance(const T& value){
            pthread_mutex_lock(&mutex_); //lock the storage location
            if(instance_ == nullptr){
                instance_ = new Singleton(value);
            }
            pthread_mutex_unlock(&mutex_); //unlock the storage location
            return instance_;
        }

        // deconstructor is not public so must refer to Destroy() method
        static void Destroy(){
            pthread_mutex_lock(&mutex_); //lock storage location
            delete instance_;
            instance_ = nullptr;
            pthread_mutex_unlock(&mutex_); //unlock storage location
        }
};