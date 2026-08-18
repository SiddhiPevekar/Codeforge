//roll nos - 23m0759, 23m0742
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>

using namespace std;

const int MAX_BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 5;


//Utility Function to send a file of any size to the grading server
int send_file(int sockfd, char* file_path)
{
    char buffer[MAX_BUFFER_SIZE]; //buffer to read  from  file
    bzero(buffer, MAX_BUFFER_SIZE); //initialize buffer to all NULLs
    FILE *file = fopen(file_path, "rb"); //open the file for reading, get file descriptor 
    if (!file)
    {
        perror("Error opening file");
        return -1;
    }
		
		//for finding file size in bytes
    fseek(file, 0L, SEEK_END); 
    int file_size = ftell(file);
    printf("File size is: %d\n", file_size);
    
    //Reset file descriptor to beginning of file
    fseek(file, 0L, SEEK_SET);
		
		//buffer to send file size to server
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    //copy the bytes of the file size integer into the char buffer
    memcpy(file_size_bytes, &file_size, sizeof(file_size));
    
    //send file size to server, return -1 if error
    if (send(sockfd, &file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        perror("Error sending file size");
        fclose(file);
        return -1;
    }

	//now send the source code file 
    while (!feof(file))  //while not reached end of file
    {
    
    		//read buffer from file
        size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE -1, file);
        
     		//send to server
        if (send(sockfd, buffer, bytes_read+1, 0) == -1)
        {
            perror("Error sending file data");
            fclose(file);
            return -1;
        }
        
        //clean out buffer before reading into it again
        bzero(buffer, MAX_BUFFER_SIZE);
    }
    //close file
    fclose(file);
    return 0;
}



int main(int argc, char* argv[]) {
    if (argc != 6) {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds> <timeout-seconds>" << endl;
        return 1;
    }

    char* serverAddress = strtok(argv[1], ":");
    char* serverPort = strtok(NULL, ":");
    char* sourceCodeFile = argv[2];
    int loopNum = atoi(argv[3]);
    int sleepTimeSeconds = atoi(argv[4]);

    float totalResponseTime = 0.0;
    float responseTime, avgResponseTime;

    int numSuccessResponse = 0,numTimeouts=0;

    int clientSocket;
    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverPort));
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);

    // Declaring the time variables
    struct timeval start1, end1, start2, end2;

    // Getting the start time of for loop
    gettimeofday(&start1, NULL);

    for (int i = 0; i < loopNum; i++) {

        // Getting the start time of sending data

        // Create socket
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            cerr << "Error creating socket." << endl;
            continue;
        }
        
        // Set a timeout for receiving data
        struct timeval timeout;
        timeout.tv_sec = atoi(argv[5]);
        timeout.tv_usec = 0;
        if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            cerr << "Error setting receive timeout." << endl;
        }

        // Connect to the server
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		// Check if the error was due to a timeout
		if (errno == EINPROGRESS || errno == EWOULDBLOCK) {
		    cerr << "Connection timeout." << endl;
		    numTimeouts++;
		} else {
		    cerr << "Error connecting to the server." << endl;
		}
		close(clientSocket);
		continue; // Continue to the next iteration of the loop
	    }

        gettimeofday(&start2, NULL);
        
        //send the file by calling the send file utility function

	    if (send_file(clientSocket, sourceCodeFile) != 0)
	    {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
		    cerr << "Send timeout." << endl;
		    numTimeouts++;
		} else {
		    cerr << "Error sending data." << endl;
		}
		close(clientSocket);
		continue; // Continue to the next iteration of the loop
	    }

        char buffer[MAX_BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        size_t bytes_read,bytesRead=0;
        cout << "Request " << i + 1 << " -> Autograding Result: ";
	    while (true)
	    {
	    
	    	//read server response
		bytes_read = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
		if (bytes_read <= 0)
		    break;
		bytesRead+=bytes_read;
		cout<<buffer;
	    }
	    cout<<endl;

        // Getting the end time of receiving data
        gettimeofday(&end2, NULL);

        // Calculating Response Time
        int Tsend = start2.tv_sec * 1000000 + start2.tv_usec;
        int Trecv = end2.tv_sec * 1000000 + end2.tv_usec;
        responseTime = Trecv - Tsend;

        if (bytesRead == 0) {
		// Check if the error was due to a timeout
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
		    cerr << "Receive timeout." << endl;
		    numTimeouts++;
		} else {
		    cerr << "Error receiving data." << endl;
		}
	 }
        else {
            // Accumulating the response times
            totalResponseTime += responseTime;
            // Incrementing the No. Successful Response
            numSuccessResponse += 1;
        }
        close(clientSocket);
        sleep(sleepTimeSeconds);
    }

    // Getting the end time of receiving data
    gettimeofday(&end1, NULL);

    // Calculating Loop Time
    int Tstart = start1.tv_sec * 1000000 + start1.tv_usec;
    int Tend = end1.tv_sec * 1000000 + end1.tv_usec;
    float loopTime = (Tend - Tstart) / 1000000.0;

    // Calculating Average Response Time
    avgResponseTime = (numSuccessResponse==0)?atoi(argv[5]):totalResponseTime/(numSuccessResponse*1000000.0);
    cout << "Average Response Time:" << avgResponseTime << endl;
    cout << "Number of Successful Responses:" << numSuccessResponse << endl;
    cout << "Time for Completing the Loop (seconds):" << loopTime << endl;
    cout << "Request sent rate:"<<1.0*loopNum/loopTime<<endl;
    cout << "Successful request rate:"<<1.0*numSuccessResponse/loopTime<<endl;
    cout << "Timeout rate:"<<1.0*numTimeouts/loopTime<<endl;
    cout << "Error rate:"<<1.0*(loopNum-numSuccessResponse-numTimeouts)/loopTime<<endl;
    cout << "Throughput:" << (float)loopNum/loopTime<<endl;

    return 0;
}

