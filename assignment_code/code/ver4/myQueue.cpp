#include "serverHeaders.h"
const int MAX_QUEUE_SIZE=500;

queue<pair<int,int>> q;


void queue_insert(int itemType, int item)
{
    if(q.size()==MAX_QUEUE_SIZE) return;
    pthread_mutex_lock(&queueMutex);
    q.push({itemType,item});
    taskCount++;
    if(itemType==2)
    		queuest.insert(item);
    pthread_cond_signal(&taskReady);
    pthread_mutex_unlock(&queueMutex);
}

pair<int,int> queue_delete()
{
    pair<int,int> element;
    if (q.empty()==true)
    {
        printf("\nThe Queue is empty\n");
    }
    else
    {
        element = q.front();
        q.pop();
        if(element.first==2)
        	queuest.erase(element.second);
    }
    return element;
}



