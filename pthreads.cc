// PThreads Example
 // ECE4893/8893, Fall 2011
 // George F. Riley, Georgia Tech, Fall 2011


#include <math.h>
 #include <iostream>
 #include <string>
 #include <stdlib.h>
 #include <pthread.h>
 #include <sys/time.h>

 #include "InputImage.h"
 #include "Complex.h"

 using namespace std;

 // Global variable visible to all threads
 Complex* ImageData;
 int ImageWidth;
 int ImageHeight;
 // Global variable that threads use to count elements
 int elementCount = 0;

 // Each thread needs to know how many threads there are.
 // This is similar to MPI_Comm_size
 int nThreads = 4;

 // The mutex and condition variables allow the main thread to
 // know when all helper threads are completed.
 pthread_mutex_t startCountMutex;
 pthread_mutex_t exitMutex;
 pthread_mutex_t elementCountMutex;
 pthread_cond_t exitCond;
 int startCount;

 // Millisecond clock function
 int GetMillisecondClock()
 {
 timeval tv;
 gettimeofday(&tv, 0);
 static bool first = true;
 static int startSec = 0;
 if (first)
 {
 startSec = tv.tv_sec;
 first = false;
 }


// Time in milliseconds
 return (tv.tv_sec - startSec) * 1000 + tv.tv_usec / 1000;
 }

 // This is the starting point for each of our threads
 void* CountThread(void* v)
 {
 unsigned long myId = (unsigned long)v; // The parameter is actually the thread number
 unsigned long localCount = 0;
 // We can assume evenly divisible here. Would be a bit more complicated
 int rowsPerThread = ImageHeight / nThreads;
 int startingRow = myId * rowsPerThread;
 // Now count the number of elements in the image with a magnitude < 100.0
 for (int r = 0; r < rowsPerThread; ++r)
 {
 int thisRow = startingRow + r;
 for (int c = 0; c < ImageWidth; ++c)
 {
 Complex thisElement = ImageData[thisRow * ImageWidth + c];
 if (thisElement.Mag().real < 100.0)
 { // Count it
 //pthread_mutex_lock(&elementCountMutex);
 localCount++;
 //pthread_mutex_unlock(&elementCountMutex);
 }
 }
 }
 pthread_mutex_lock(&elementCountMutex);
 elementCount += localCount;
 pthread_mutex_unlock(&elementCountMutex);
 // This thread is done; decrement the active count and see if all
 // have finished
 pthread_mutex_lock(&startCountMutex);
 startCount--;
 if (startCount == 0)
 { // Last to exit, notify main
 pthread_mutex_unlock(&startCountMutex);
 pthread_mutex_lock(&exitMutex);
 pthread_cond_signal(&exitCond);
 pthread_mutex_unlock(&exitMutex);
 }
 else
 {
 pthread_mutex_unlock(&startCountMutex);
 }
 }
 int main(int argc, char** argv)
 {
 string fileName("Tower-Extra-Large.txt");
 // See if number of thread specified on command lind



 if (argc > 1) nThreads = atol(argv[1]);

 // See if file name specified on command line
 if (argc > 2) fileName = string(argv[2]);
 InputImage image(fileName.c_str());
 // We use a global pointer so all threads can see the
 // same image data array as well as width/height
 ImageData = image.GetImageData();
 ImageWidth = image.GetWidth();
 ImageHeight = image.GetHeight();

 // All mutex and condition variables must be "initialized"
 pthread_mutex_init(&exitMutex,0);
 pthread_mutex_init(&startCountMutex,0);
 pthread_mutex_init(&elementCountMutex,0);
 pthread_cond_init(&exitCond, 0);
 // Main holds the exit mutex until waiting for exitCond condition
 pthread_mutex_lock(&exitMutex);

 // Get elapsed milliseconds (starting time after image loaded)
 GetMillisecondClock();
 startCount = nThreads; // Total threads (to be) started
 // Now start the threads
 for (int i = 0; i < nThreads; ++i)
 {
 // Now create the thread
 pthread_t pt; // pThread variable (output param from create)
 // Third param is the thread starting function
 // Fourth param is passed to the thread starting function
 pthread_create(&pt, 0, CountThread, (void*)i);
 }
 // Main program now waits until all child threads completed
 pthread_cond_wait(&exitCond, &exitMutex);
 // At this point all thread have completed and global "count"
 // is the number of image elements with magnitude < 100.0
 cout << "Elapsed time (seconds) " << GetMillisecondClock() / 1000.0 << endl;
 cout << "Count is " << elementCount << endl;
 }

