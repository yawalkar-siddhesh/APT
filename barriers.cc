// Demonstrate simple MPI barriers and collectives
// This one uses non-blocking ISend/Irecv
// George F. Riley, Georgia Tech, Fall 2011


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "mpi.h"

using namespace std;

class barrier {
public : 
    barrier (int P0);
    void enter (int myCPU);
private:
    int P;
    int count;
    int decrement_count();
    pthread_mutex_t countMutex;
    bool* localview;
    bool  globalview;
};

barrier::barrier(int P0)
  : P(P0), count (P0)
{
pthread_mutex_init(&countMutex, 0 );
localview = new bool[P];
for (int i = 0 ;i<P; i++)
	localview[i] = true;
globalview = true;
}

void barrier::enter( int myCPU) {
	localview[myCPU] = !localview[myCPU];
        if (decrement_count() == 1)
	{
           count = P ;
           globalview = localview [myCPU];
        }
        else 
           while (globalview != localview[myCPU]);
}

int barrier::decrement_count() {
	pthread_mutex_lock(&countMutex);
        int MyCount = count;
	count--;
        pthread_mutex_unlock(&countMutex);
	return MyCount;
}




int main(int argc,char**argv)
{
barrier * this_barrier;
this_barrier = new barrier(16);
this_barrier->enter(16);
cout << "All threads finished pass 1" << endl;

}


