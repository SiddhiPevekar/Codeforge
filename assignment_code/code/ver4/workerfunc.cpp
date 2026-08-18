#include "serverHeaders.h"
int taskCount=0;
bool isOutputCorrect(string& outputFile) {
    // Load the expected output from the file code_expected_output.txt
    ifstream expectedOutputFile("initialFiles/code_expected_output.txt");
    if (!expectedOutputFile.is_open()) {
        cerr << "Error: Could not open code_expected_output.txt" << std::endl;
        return false; 
    }
    // Use the system function to execute the diff command
    string diffCommand = "diff " + outputFile + " initialFiles/code_expected_output.txt";
    int diffResult = system(diffCommand.c_str());

    // Check the return value of the diff command
    return diffResult==0;
}

//Utility Function to send a response to client
int send_file(int sockfd, const char* file_path, char* req_type)
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
    
    // Read the initial text into the buffer
    snprintf(buffer, MAX_BUFFER_SIZE, "Your grading request ID %s  processing is done, here are the results:\n", req_type);

    // Get the length of the initial text
    size_t initial_text_length = strlen(buffer);

    // Now read the file data into the remaining space in the buffer
    size_t bytes_read = fread(buffer + initial_text_length, 1, MAX_BUFFER_SIZE - initial_text_length, file);

    // Send the entire buffer, including the initial text and file data
    if (send(sockfd, buffer, initial_text_length + bytes_read, 0) == -1)
    {
        perror("Error sending response and file data");
        fclose(file);
        return -1;
    }
	bzero(buffer, MAX_BUFFER_SIZE);

	//now send the response
    while (!feof(file))  //while not reached end of file
    {
    
    		//read buffer from file
        size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, file);
        
     		//send to server
        if (send(sockfd, buffer, bytes_read, 0) == -1)
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

//Utility Function to receive a file of any size to the grading server
int recv_file(int sockfd, ofstream& file)
//Arguments: socket fd, file name (can include path) into which we will store the received file
{
    char buffer[MAX_BUFFER_SIZE]; //buffer into which we read  the received file chars
    memset(buffer, 0, sizeof(buffer)); //initialize buffer to all NULLs

	//buffer for getting file size as bytes
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    //first receive  file size bytes
    if (recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        perror("Error receiving file size");
        return -1;
    }
   
    int file_size;
    //copy bytes received into the file size integer variable
    file_size=atoi(file_size_bytes);
    
    //some local printing for debugging
    printf("File size is: %d\n", file_size);
    
    //now start receiving file data
    size_t bytes_read = 0, total_bytes_read =0;
    while (true)
    {
    	  //read max MAX_BUFFER_SIZE amount of file data
        bytes_read = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
		if (bytes_read <= 0)
        {
            perror("Error receiving file data");
            return -1;
        }

        //total number of bytes read so far
        total_bytes_read += bytes_read;

        
		
		//write the buffer to the file
        file.write(buffer, bytes_read);

	// reset buffer
        bzero(buffer, MAX_BUFFER_SIZE);
        
       //break out of the reading loop if read file_size number of bytes
        if (total_bytes_read >= file_size)
            break;
    }
    return 0;
}

//Function to Grade a request
void handleClient(int req_id) {
   
    // Generate unique file names based on the client's port and a timestamp
    string reqid = to_string(req_id);
    string studentFileName = reqid + "_file.cpp";
    string fullPath = "ToGrade/" + studentFileName;
    string executableFileName = "student_program_" + reqid;
    string compileErrorFileName = "compile_error_" + reqid+".txt";
    string runtimeErrorFileName = "runtime_error_" + reqid + ".txt";
    string outputFileName = "student_output_" + reqid + ".txt";
    string finalFile= "Graded/"+reqid+".txt";
    filesystem::create_directory("Graded");

    // Compile and run the code
    string compileCmd = "g++ -o " + executableFileName + " " + fullPath + " 2> " + compileErrorFileName;
    int compileResult = system(compileCmd.c_str());

    if (compileResult != 0) {
        // Compilation failed, send the error message back
        ifstream errorFile(compileErrorFileName);
        string compileError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
        string response = reqid + "\nCOMPILER ERROR\n" + compileError;
        ofstream outFile(finalFile);
        outFile<<response;
        outFile.close();
        remove(compileErrorFileName.c_str()); // Delete the compile error file
        remove(studentFileName.c_str()); // Delete the student code file
        remove(fullPath.c_str());
        processingst.erase(req_id);
        return;
    }

    string executionCmd = "./" + executableFileName + " 1> " + outputFileName + " 2> " + runtimeErrorFileName;
    int executionResult = system(executionCmd.c_str());

    if (executionResult != 0) {
        // Runtime error, send the error message back
        ifstream errorFile(runtimeErrorFileName);
        string runtimeError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
        string response = "RUNTIME ERROR\n" + runtimeError;
        ofstream outFile(finalFile);
        outFile<<response;
        outFile.close();
        remove(runtimeErrorFileName.c_str()); // Delete the runtime error file
        remove(compileErrorFileName.c_str());
        remove(studentFileName.c_str()); // Delete the student code file
        remove(executableFileName.c_str()); // Delete the executable file
        remove(fullPath.c_str());
        processingst.erase(req_id);
        return;
    }

    // Capture the program's output
    ifstream output_file(outputFileName);

    // Check if the output is correct
    bool outputCorrect = isOutputCorrect(outputFileName);

    // Send result back to the client
    string response = outputCorrect ? "PASS" : "OUTPUT ERROR";
	ofstream outFile(finalFile);
   outFile<<response;
   outFile.close();

    // Clean up by deleting temporary files
    remove(runtimeErrorFileName.c_str());
    remove(studentFileName.c_str());
    remove(compileErrorFileName.c_str());
    remove(executableFileName.c_str());
    remove(outputFileName.c_str());
    remove(fullPath.c_str());
	processingst.erase(req_id);
    return;
}

//Function for thread pool threads to pick up items in queue and do the processing
void *workerFunc(void *args)
{
    while (true)
    {
        bool found=false;
        pair<int,int> item;
        pthread_mutex_lock(&queueMutex);
        while (taskCount == 0)
        {
            pthread_cond_wait(&taskReady, &queueMutex);
        }
        if (taskCount > 0)
        {
            found = true;
            item = queue_delete();
            taskCount--;
            pthread_cond_signal(&taskReady);
        }
        pthread_mutex_unlock(&queueMutex);
        if (found == true)
        {
        	  if(item.first==0){
        	  	newReqHandler(item.second);
        	  }
        	  else if(item.first==1){
        	  	status_check(item.second);
        	  }
        	  else{
        	  	processingst.insert(item.second);
       		handleClient(item.second);
        	  }
        }
    }
    return NULL;
}

//function to handle new request and receive source code file from it
void newReqHandler(int clientSocket){
	char req_type[MAX_FILE_SIZE_BYTES];
    bzero(req_type, MAX_FILE_SIZE_BYTES); //initialize buffer to all NULLs
    
    //assign request id to the current request
	pthread_mutex_lock(&reqidMutex);
	int cur_req_id=g_req_id;
	g_req_id++;
	string filePath = "initialFiles/g_req_id.txt";
	ofstream newFile(filePath);
	newFile << to_string(g_req_id); // Write g_req_id into the file
     newFile.close();
	pthread_mutex_unlock(&reqidMutex);
	cout<<cur_req_id<<endl;
	char *s;
     s = (char*)malloc (MAX_FILE_SIZE_BYTES*sizeof(char));
     memset(s, 0, sizeof(s));
	snprintf(s, sizeof(s), "%d", cur_req_id);
	
	//receive the file from client and store in a folder to grade later
	filesystem::create_directory("ToGrade");
	string studentFileName=s;
	studentFileName+="_file.cpp";
	string fullPath = "ToGrade/" + studentFileName;
	ofstream outFile(fullPath);
	if(recv_file(clientSocket,outFile)!=0){
		printf("ERROR: Could not receive file\n");
		//send error to client
		send(clientSocket,"ERROR: Could not receive file\n",30,0);
		close(clientSocket);
		outFile.close();
       	return;
	}
	outFile.close();
	
	//send request id to client
	send(clientSocket,s,MAX_FILE_SIZE_BYTES,0);
	
	//insert req_id into queue to grade that request
	queue_insert(2,cur_req_id);
	free(s);
	close(clientSocket);
	return;
}

//function to handle status check requests and send back the status to client
void status_check(int clientSocket){
	char req_type[MAX_FILE_SIZE_BYTES];
    bzero(req_type, MAX_FILE_SIZE_BYTES); //initialize buffer to all NULLs
    //receive the request id from client
	if (recv(clientSocket, req_type, sizeof(req_type), 0) == -1)
	{
	   perror("Error receiving request id\n");
	   close(clientSocket);
	   return;
	}
	
	int req_id=atoi(req_type);
	cout<<"Received request id "<<req_id<<endl;
	
	char s[MAX_BUFFER_SIZE];
  memset(s, 0, sizeof(s));
	
	if(queuest.find(req_id)!=queuest.end()){
		//currently in queue
		strcpy(s, "Your grading request ID ");
		strcat(s, req_type);
		strcat(s, " has been accepted. It is currently in the queue and is being processed\n");
		send(clientSocket,s,MAX_BUFFER_SIZE,0);
	}
	
	else if(processingst.find(req_id)!=processingst.end()){
		//currently with a thread
		strcpy(s, "Your grading request ID ");
		strcat(s, req_type);
		strcat(s, " has been accepted and is currently being processed\n");
		send(clientSocket,s,MAX_BUFFER_SIZE,0);
	}
	else if(req_id<0 || req_id>=g_req_id){
		//not found
		strcpy(s, "Grading request ");
		strcat(s, req_type);
		strcat(s, " not found. Please check and resend your request ID or re-send your original grading request\n");
		send(clientSocket,s,MAX_BUFFER_SIZE,0);
	}
	else{
		//processing is done
		/*
		strcpy(s, "Your grading request ID ");
		strcat(s, req_type);
		strcat(s, "  processing is done, here are the results:\n");
		send(clientSocket,s,MAX_BUFFER_SIZE,0);*/
		string finalFile=req_type;
		finalFile= "Graded/"+finalFile+".txt";
		send_file(clientSocket,finalFile.c_str(),req_type);
	}
    	close(clientSocket);
	return;
}

//function to get the type of request from client and put it in queue for threads to pick it up
void masterFunc(int clientSocket)
{
	//buffer for getting request type
    char req_type[MAX_FILE_SIZE_BYTES];
    bzero(req_type, MAX_FILE_SIZE_BYTES); //initialize buffer to all NULLs
    if (recv(clientSocket, req_type, sizeof(req_type), 0) == -1)
    {
        perror("Error receiving request type\n");
        close(clientSocket);
        return;
    }
   
    int reqType=atoi(req_type);
    cout<<"Received req type "<<reqType<<endl;
    if(reqType==0){//new grading request
    		queue_insert(0,clientSocket);
    }
    else{//asking about status of old request
    		queue_insert(1,clientSocket);
    }
}

