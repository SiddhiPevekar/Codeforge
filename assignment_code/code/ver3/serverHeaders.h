#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <cstdio>
#include <string>
#include <pthread.h>
#include <thread>
#include <chrono> // Include for timestamp
#include<vector>
#include<queue>
using namespace std;
// Create a structure to hold client socket and any other necessary data
struct ThreadArgs {
    int clientSocket;
};
void queue_insert(ThreadArgs*&);
ThreadArgs* queue_delete();
void* handleClient(void*); 
void *workerFunc(void*);
void masterFunc(ThreadArgs*&);
extern pthread_mutex_t queueMutex;
extern pthread_mutex_t taskMutex;
extern pthread_cond_t taskReady;
extern const int MAX_BUFFER_SIZE;
extern const int MAX_FILE_SIZE_BYTES;
extern const int MAX_QUEUE_SIZE;
extern int taskCount;
extern queue<ThreadArgs *> q;
