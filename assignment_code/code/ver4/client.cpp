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
#include <string>

using namespace std;

const int MAX_BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 100;


//Utility Function to send a file of any size to the grading server
int send_file(int sockfd, char* file_path)
{
    char buffer[MAX_BUFFER_SIZE]; //buffer to read  from  file
    bzero(buffer, MAX_BUFFER_SIZE); //initialize buffer to all NULLs
    FILE *file = fopen(file_path, "r"); //open the file for reading, get file descriptor 
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
    bzero(file_size_bytes,MAX_FILE_SIZE_BYTES);
    //copy the bytes of the file size integer into the char buffer
    snprintf(file_size_bytes, sizeof(file_size_bytes), "%d", file_size);
    
    //send file size to server, return -1 if error
    if (send(sockfd, &file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        perror("Error sending file size");
        fclose(file);
        return -1;
    }

	//now send the source code file 
	int bytes_read;
    while ((bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE-1, file)) > 0) {
	    // send to server
	    buffer[MAX_BUFFER_SIZE-1]='\0';
	    if (send(sockfd, buffer, bytes_read, 0) == -1) {
		   perror("Error sending file data");
		   fclose(file);
		   return -1;
	    }

	    // clean out buffer before reading into it again
	    bzero(buffer, MAX_BUFFER_SIZE);
	}
    //close file
    fclose(file);
    return 0;
}

void status_check(char* req_id_str, int clientSocket){
    
    char tempbuffer[MAX_FILE_SIZE_BYTES];
	snprintf(tempbuffer, sizeof(tempbuffer), "%s", req_id_str);

    //send req_id to server
    if (send(clientSocket, tempbuffer, sizeof(tempbuffer), 0) == -1)
    {
        perror("Error sending request id");
        return;
    }
    
    char buffer[MAX_BUFFER_SIZE];
   memset(buffer, 0, sizeof(buffer));
   size_t bytes_read,bytesRead=0;
   cout << "Request " << req_id_str << " -> Autograding Result: ";
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

   if (bytesRead == 0) {
	// Check if the error was due to a timeout
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
	    cerr << "Receive timeout." << endl;
	} else {
	    cerr << "Error receiving data." << endl;
	}
 }
    
}



int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <new|status> <serverIP:port> <sourceCodeFileTobeGraded|requestID>" << endl;
        return 1;
    }
    
    
    char* serverAddress = strtok(argv[2], ":");
    char* serverPort = strtok(NULL, ":");

    float totalResponseTime = 0.0;
    float responseTime, avgResponseTime;

    int clientSocket;
    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverPort));
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);


   // Create socket
   if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
       cerr << "Error creating socket." << endl;
       return 1;
   }
   
   // Set a timeout for receiving data
   struct timeval timeout;
   timeout.tv_sec = 10;
   timeout.tv_usec = 0;
   if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
       cerr << "Error setting receive timeout." << endl;
   }

   // Connect to the server
   if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
	// Check if the error was due to a timeout
	if (errno == EINPROGRESS || errno == EWOULDBLOCK) {
	    cerr << "Connection timeout." << endl;
	} else {
	    cerr << "Error connecting to the server." << endl;
	}
	close(clientSocket);
	return 1;
    }

   
   //doing action based on request type
   string req_type=argv[1];
    if(req_type=="status"){
    		string val=to_string(1);
    		char tempbuffer[MAX_FILE_SIZE_BYTES];
    		bzero(tempbuffer, MAX_FILE_SIZE_BYTES);
		snprintf(tempbuffer, sizeof(tempbuffer), "%s", val.c_str());
    		send(clientSocket, tempbuffer, sizeof(tempbuffer), 0);
    		status_check(argv[3],clientSocket);
    }
    else if(req_type=="new"){
    		//send the file by calling the send file utility function
    		string val=to_string(0);
    		char tempbuffer[MAX_FILE_SIZE_BYTES];
    		bzero(tempbuffer, MAX_FILE_SIZE_BYTES);
		snprintf(tempbuffer, sizeof(tempbuffer), "%s", val.c_str());
		send(clientSocket, tempbuffer, sizeof(tempbuffer), 0);
	    if (send_file(clientSocket, argv[3]) != 0)
	    {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
		    cerr << "Send timeout." << endl;
		} else {
		    cerr << "Error sending data." << endl;
		}
	    }
	    cout<<"Your request id is:";
	    char buffer[MAX_FILE_SIZE_BYTES];
	   memset(buffer, 0, sizeof(buffer));
	   size_t bytes_read,bytesRead=0;
	    while (true)
	    {
	    
	    	//read server response
		bytes_read = recv(clientSocket, buffer, MAX_FILE_SIZE_BYTES, 0);
		if (bytes_read <= 0)
		    break;
		bytesRead+=bytes_read;
		cout<<buffer;
	    }
	    cout<<endl;

	   if (bytesRead == 0) {
		// Check if the error was due to a timeout
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
		    cerr << "Receive timeout." << endl;
		} else {
		    cerr << "Error receiving data." << endl;
		}
	 }
    }
    else{
    		cerr << "Usage: " << argv[0] << " <new|status> <serverIP:port> <sourceCodeFileTobeGraded|requestID>" << endl;
    		close(clientSocket);
        return 1;
    }
   
   close(clientSocket);

    return 0;
}

