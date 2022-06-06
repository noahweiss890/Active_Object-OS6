CXX = gcc
CPPXX = clang++-9
FLAGS = -Wall -g

all: Client_Reactor Client Guard_Main Main1 Pollserver Singleton_Main libsix.so

Client_Reactor: client_reactor.o libsix.so
	$(CXX) $(FLAGS) -pthread client_reactor.o -o Client_Reactor ./libsix.so

Client: client.o libsix.so
	$(CXX) $(FLAGS) client.o -o Client ./libsix.so

Guard_Main: guard_main.o libsix.so
	$(CPPXX) $(FLAGS) -pthread guard_main.o -o Guard_Main ./libsix.so

Main1: main1.o libsix.so
	$(CXX) $(FLAGS) -pthread main1.o -o Main1 ./libsix.so

Pollserver: pollserver.o libsix.so
	$(CPPXX) $(FLAGS) -pthread pollserver.o -o Pollserver ./libsix.so

Singleton_Main: singleton_main.o libsix.so
	$(CPPXX) $(FLAGS) singleton_main.o -o Singleton_Main ./libsix.so

libsix.so: ActiveObject.o MyQueue.o reactor.o guard.o singleton.o
	$(CPPXX) --shared -fPIC -fPIE -pthread -o libsix.so ActiveObject.o MyQueue.o reactor.o guard.o singleton.o

ActiveObject.o: ActiveObject.c ActiveObject.h
	$(CXX) $(FLAGS) -fPIC ActiveObject.c -c

client_reactor.o: client_reactor.c
	$(CXX) $(FLAGS) client_reactor.c -c

client.o: client.c
	$(CXX) $(FLAGS) client.c -c

guard_main.o: guard_main.cpp guard.cpp
	$(CPPXX) $(FLAGS) -fPIC -pthread guard_main.cpp -c

guard.o: guard.cpp
	$(CPPXX) $(FLAGS) -fPIC -fPIE -pthread guard.cpp -c

main1.o: main1.c ActiveObject.h
	$(CXX) $(FLAGS) -pthread main1.c -c

MyQueue.o: MyQueue.c MyQueue.h
	$(CXX) $(FLAGS) -fPIC -pthread MyQueue.c -c

pollserver.o: pollserver.cpp reactor.hpp
	$(CPPXX) $(FLAGS) pollserver.cpp -c

reactor.o: reactor.cpp reactor.hpp
	$(CPPXX) $(FLAGS) -fPIC -pthread reactor.cpp -c

singleton_main.o: singleton_main.cpp singleton.cpp
	$(CPPXX) $(FLAGS) singleton_main.cpp -c

singleton.o: singleton.cpp
	$(CPPXX) $(FLAGS) -fPIC -pthread singleton.cpp -c

clean:
	 rm -f *.o Client_Reactor Client Guard_Main Main1 Pollserver Singleton_Main libsix.so singleton_test.txt