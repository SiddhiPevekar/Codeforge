#include "serverHeaders.h"
const int MAX_QUEUE_SIZE=100;

queue<ThreadArgs *> q;

void queue_insert(ThreadArgs*& item)
{
    if(q.size()==MAX_QUEUE_SIZE) return;
    pthread_mutex_lock(&queueMutex);
    q.push(item);
    taskCount++;
    pthread_cond_signal(&taskReady);
    pthread_mutex_unlock(&queueMutex);
}

ThreadArgs* queue_delete()
{
    ThreadArgs* item=NULL;;
    if (q.empty()==true)
    {
        printf("\nThe Queue is empty\n");
    }
    else
    {
        item = q.front();
        q.pop();
    }
    return item;
}



