#include "serverHeaders.h"

const int MAX_BUFFER_SIZE=1024;
const int MAX_FILE_SIZE_BYTES = 100;
int g_req_id=0;
pthread_mutex_t queueMutex;
pthread_mutex_t taskMutex;
pthread_cond_t taskReady;
pthread_mutex_t reqidMutex;
set<int> queuest;
set<int> processingst;

//function for initializing server to make it fault tolerant
void initial_setup(){
	cout<<"********************Starting the server... please wait********************"<<endl;
	//update g_req_id from the previous state if this server was previously run and stopped
	string filePath = "initialFiles/g_req_id.txt";
	if (filesystem::exists(filePath)) {
        // Open the file for reading
        ifstream file(filePath);

        // Check if the file is open
        if (file.is_open()) {
            // Read the value from the file into g_req_id
            file >> g_req_id;
            file.close();
            cout << "Updated g_req_id: " << g_req_id << endl;
        } 
        else {
            cerr << "Error opening file: " << filePath << endl;
        }
    } 
    else {
        // The file doesn't exist, create it and write 0 into it
        ofstream newFile(filePath);
        if (newFile.is_open()) {
            newFile << "0"; // Write 0 into the file
            newFile.close();
            g_req_id = 0;
            cout << "Created and initialized file: " << filePath << endl;
        } else {
            cerr << "Error creating file: " << filePath << endl;
        }
    }
    
    //push the previously ungraded files to queue to grade them
    string toGradeDir = "ToGrade";
    // Check if the directory exists
    if (filesystem::exists(toGradeDir) && filesystem::is_directory(toGradeDir)) {
        // Iterate through the files in the directory
        for (const auto& entry : filesystem::directory_iterator(toGradeDir)) {
            // Check if the file name matches the pattern reqid_file.cpp
            string fileName = entry.path().filename().string();
            size_t pos = fileName.find("_file.cpp");
            if (pos != string::npos) {
                // Extract reqid from the file name
                int reqid = stoi(fileName.substr(0, pos));

                // Push the file to queue to grade it
                queue_insert(2, reqid);
            }
        }
    } else {
        // Directory doesn't exist
        cerr << "ToGrade directory not found." << endl;
    }
    cout<<"********************Server Ready to accept requests********************"<<endl;
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
    pthread_mutex_init(&reqidMutex, NULL);
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
    
    //do some initial tasks for server to make it fault tolerant
    initial_setup();

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
        masterFunc(clientSocket);
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
    pthread_mutex_destroy(&reqidMutex);

    close(serverSocket);

    return 0;
}
