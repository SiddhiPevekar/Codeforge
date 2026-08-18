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
#include <chrono> // Include for timestamp
#include <vector>
#include <queue>
#include <set>
#include <filesystem>
#include <utility>
#include <bits/stdc++.h>
using namespace std;
// Create a structure to hold client socket and any other necessary data
struct ThreadArgs {
    int clientSocket;
};
extern set<int> queuest;
extern set<int> processingst;
void queue_insert(int,int);
pair<int,int> queue_delete();
void handleClient(int); 
void status_check(int);
void newReqHandler(int);
void *workerFunc(void*);
void masterFunc(int);
void initial_setup();
extern pthread_mutex_t queueMutex;
extern pthread_mutex_t taskMutex;
extern pthread_cond_t taskReady;
extern const int MAX_BUFFER_SIZE;
extern const int MAX_FILE_SIZE_BYTES;
extern const int MAX_QUEUE_SIZE;
extern int taskCount;
extern int g_req_id;
extern pthread_mutex_t reqidMutex;
