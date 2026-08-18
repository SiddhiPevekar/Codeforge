#include "serverHeaders.h"

const int MAX_BUFFER_SIZE=1024;
const int MAX_FILE_SIZE_BYTES = 5;
pthread_mutex_t queueMutex;
pthread_mutex_t taskMutex;
pthread_cond_t taskReady;

void* monitorQueueSize(void* arg) {
    while (true) {
        // Sleep for 5 seconds
        this_thread::sleep_for(std::chrono::seconds(5));

        // Get the current queue size
        int currentQueueSize;
        pthread_mutex_lock(&queueMutex);
        currentQueueSize = q.size();
        pthread_mutex_unlock(&queueMutex);

        // Append the queue size to the "queuesize.txt" file
        std::ofstream file("queuesize.txt", std::ios_base::app);
        if (file.is_open()) {
            file << "Queue Size: " << currentQueueSize << "\n";
            file.close();
        }
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <port> <thread_pool_size>" << endl;
        return 1;
    }

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Error binding socket." << endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1000) == -1) {
        cerr << "Error listening for connections." << endl;
        return 1;
    }

    cout << "Server listening on port " << atoi(argv[1]) << "..." << endl;
    
    int noThreads=atoi(argv[2]);
    pthread_t threads[noThreads];
    pthread_mutex_init(&queueMutex, NULL);
    pthread_mutex_init(&taskMutex, NULL);
    pthread_cond_init(&taskReady, NULL);
    // Create threads
    for (int i = 0; i < noThreads; i++)
    {
        if (pthread_create(&threads[i], NULL, workerFunc, NULL) != 0)
        {
            printf("ERROR: Could not create thread %d", i);
            exit(EXIT_FAILURE);
        }
    }
    
    // Create a thread for monitoring queue size
    pthread_t monitorThread;
    if (pthread_create(&monitorThread, nullptr, monitorQueueSize, nullptr) != 0) {
        cerr << "ERROR: Could not create the monitor thread" << endl;
        exit(EXIT_FAILURE);
    }
    
    

    while (true) {
        // Accept incoming connection
        cout << "######################################################\n";
        cout << "Accepting new connection requests\n";

        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        cout << "Connected to client with port: " << clientAddr.sin_port << endl;
        ThreadArgs *args = new ThreadArgs;
        args->clientSocket = clientSocket;
        masterFunc(args);
    }
    
    // // Wait for threads to finish
    for (int i = 0; i < noThreads; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            printf("ERROR: Could not join thread %d", i);
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_destroy(&queueMutex);
    pthread_mutex_destroy(&taskMutex);

    close(serverSocket);

    return 0;
}
