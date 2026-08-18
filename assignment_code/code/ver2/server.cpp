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
#include<vector>

using namespace std;

const int MAX_BUFFER_SIZE = 1024;

// Create a structure to hold client socket and any other necessary data
struct ThreadArgs {
    int clientSocket;
};

vector<pthread_t> threadList; // Global vector to store thread IDs

bool isOutputCorrect(const string& programOutput) {
    string expectedOutput = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n";
    return programOutput == expectedOutput;
}

void* handleClient(void* clientSocketPtr) {
    ThreadArgs *args = (ThreadArgs *)clientSocketPtr;
    int clientSocket = args->clientSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    if (getpeername(clientSocket, (struct sockaddr*)&clientAddr, &clientAddrLen) == -1) {
        cerr << "Error getting client address." << endl;
        close(clientSocket);
        pthread_exit(NULL);
    }

    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        cerr << "Error receiving data." << endl;
        close(clientSocket);
        pthread_exit(NULL);
    }

    // Generate unique file names based on the client's port and a timestamp
    string portStr = to_string(clientAddr.sin_port);
    auto now = chrono::system_clock::now();
    auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
    string timestampStr = to_string(timestamp);
    string studentFileName = "student_code_" + portStr + "_" + timestampStr + ".cpp";
    string executableFileName = "student_program_" + portStr + "_" + timestampStr;
    string compileErrorFileName = "compile_error_" + portStr + "_" + timestampStr + ".txt";
    string runtimeErrorFileName = "runtime_error_" + portStr + "_" + timestampStr + ".txt";
    string outputFileName = "student_output_" + portStr + "_" + timestampStr + ".txt";

    // Save the received code to a temporary file
    ofstream codeFile(studentFileName);
    codeFile << buffer;
    codeFile.close();

    // Compile and run the code
    string compileCmd = "g++ -o " + executableFileName + " " + studentFileName + " 2> " + compileErrorFileName;
    int compileResult = system(compileCmd.c_str());

    if (compileResult != 0) {
        // Compilation failed, send the error message back
        ifstream errorFile(compileErrorFileName);
        string compileError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
        string response = "COMPILER ERROR\n" + compileError;
        send(clientSocket, response.c_str(), response.size(), 0);
        close(clientSocket);
        remove(compileErrorFileName.c_str()); // Delete the compile error file
        remove(studentFileName.c_str()); // Delete the student code file
        return (void*)NULL;
    }

    string executionCmd = "./" + executableFileName + " 1> " + outputFileName + " 2> " + runtimeErrorFileName;
    int executionResult = system(executionCmd.c_str());

    if (executionResult != 0) {
        // Runtime error, send the error message back
        ifstream errorFile(runtimeErrorFileName);
        string runtimeError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
        string response = "RUNTIME ERROR\n" + runtimeError;
        send(clientSocket, response.c_str(), response.size(), 0);
        close(clientSocket);
        remove(runtimeErrorFileName.c_str()); // Delete the runtime error file
        remove(compileErrorFileName.c_str());
        remove(studentFileName.c_str()); // Delete the student code file
        remove(executableFileName.c_str()); // Delete the executable file
        return (void*)NULL;
    }

    // Capture the program's output
    ifstream output_file(outputFileName);
    string programOutput((istreambuf_iterator<char>(output_file)), istreambuf_iterator<char>());

    // Check if the output is correct
    bool outputCorrect = isOutputCorrect(programOutput);

    // Send result back to the client
    string response = outputCorrect ? "PASS" : "OUTPUT ERROR";

    send(clientSocket, response.c_str(), response.size(), 0);

    close(clientSocket);

    // Clean up by deleting temporary files
    remove(runtimeErrorFileName.c_str());
    remove(studentFileName.c_str());
    remove(compileErrorFileName.c_str());
    remove(executableFileName.c_str());
    remove(outputFileName.c_str());

    return (void*)NULL;
}


void* joinThreads(void*) {
    for (size_t i = 0; i < threadList.size(); ++i) {
        pthread_join(threadList[i], nullptr); // Join each client-handling thread
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
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
    
    pthread_t joinThread;
    if (pthread_create(&joinThread, NULL, joinThreads, nullptr) != 0) {
        cerr << "Error creating join thread." << endl;
        return 1;
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

        pthread_t thread;
        if (pthread_create(&thread, NULL, handleClient, args) != 0) {
            cerr << "Error creating thread." << endl;
            close(clientSocket);
            continue;
        }
    }

    close(serverSocket);

    return 0;
}
