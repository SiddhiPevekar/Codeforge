#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/time.h>

using namespace std; // Add this line to avoid using std:: everywhere

const int MAX_BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds>" << endl;
        return 1;
    }

    char* serverAddress = strtok(argv[1], ":");
    char* serverPort = strtok(NULL, ":");
    char* sourceCodeFile = argv[2];
    int loopNum = atoi(argv[3]);
    int sleepTimeSeconds = atoi(argv[4]);
    
    float totalResponseTime = 0.0;
    float responseTime, avgResponseTime;
    
    int numSuccessResponse = 0;

    int clientSocket;
    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverPort));
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);

    // Read the source code file
    ifstream file(sourceCodeFile);
    string sourceCode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

	// Declaring the time variables
    struct timeval start1, end1, start2, end2;
    
	// Getting the start time of for loop
	gettimeofday(&start1, NULL);
	
    for(int i=0; i<loopNum; i++) {

		// Getting the start time of sending data
		
		
	    // Create socket
		if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		    cerr << "Error creating socket." << endl;
		    return 1;
		}
		
		// Connect to the server
		if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		    cerr << "Error connecting to server." << endl;
		    return 1;
		}
		
		gettimeofday(&start2, NULL);
		
		// Send the source code to the server
		ssize_t bytesSent = send(clientSocket, sourceCode.c_str(), sourceCode.size(), 0);
		if (bytesSent == -1) {
		    cerr << "Error sending data." << endl;
		    close(clientSocket);
		    return 1;
		}

		char buffer[MAX_BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));

		// Receive result from the server
		ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
		
		// Getting the end time of receiving data
		gettimeofday(&end2, NULL);
		
		// Calculating Response Time
		int Tsend = start2.tv_sec * 1000000 + start2.tv_usec;
		int Trecv = end2.tv_sec * 1000000 + end2.tv_usec;
		responseTime = Trecv - Tsend;
		
		if (bytesRead == -1) {
		    cerr << "Error receiving data." << endl;
		    //close(clientSocket);
		    //return 1;
		}
		else {
			// Accumulating the response times
			totalResponseTime += responseTime;
			// Incrementing the No. Successful Response
			numSuccessResponse += 1;
		}

		cout << "Request " << i+1 << " -> Autograding Result: " << buffer << endl;
		close(clientSocket);
		
		sleep(sleepTimeSeconds);
	}
	
	// Getting the end time of receiving data
	gettimeofday(&end1, NULL);
	
	// Calculating Loop Time
	int Tstart = start1.tv_sec * 1000000 + start1.tv_usec;
	int Tend = end1.tv_sec * 1000000 + end1.tv_usec;
	float loopTime = (Tend - Tstart)/1000000.0;
	
	// Calculating Average Response Time
	avgResponseTime = totalResponseTime/(loopNum*1000000.0);
	cout << "Average Response Time:" << avgResponseTime << endl;
	cout << "Number of Successful Responses:" << numSuccessResponse << endl;
	cout << "Time for Completing the Loop (seconds):" << loopTime << endl;
	cout << "Throughput:" << (float)numSuccessResponse/loopTime << endl;

    return 0;
}

