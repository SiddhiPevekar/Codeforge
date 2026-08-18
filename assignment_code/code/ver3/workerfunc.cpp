#include "serverHeaders.h"
int taskCount=0;
bool isOutputCorrect(string& outputFile) {
    // Load the expected output from the file code_expected_output.txt
    ifstream expectedOutputFile("code_expected_output.txt");
    if (!expectedOutputFile.is_open()) {
        cerr << "Error: Could not open code_expected_output.txt" << std::endl;
        return false; 
    }
    string expectedOutput((std::istreambuf_iterator<char>(expectedOutputFile)), std::istreambuf_iterator<char>());

    // Use the system function to execute the diff command
    string diffCommand = "diff " + outputFile + " code_expected_output.txt";
    int diffResult = system(diffCommand.c_str());

    // Check the return value of the diff command
    if (diffResult == 0) {
        // The files are identical, so the output is correct
        return true;
    } else {
        // The files are different, indicating a mismatch
        return false;
    }
}

//Utility Function to receive a file of any size to the grading server
int recv_file(int sockfd, ofstream& file)
//Arguments: socket fd, file name (can include path) into which we will store the received file
{
    char buffer[MAX_BUFFER_SIZE]; //buffer into which we read  the received file chars
    bzero(buffer, MAX_BUFFER_SIZE); //initialize buffer to all NULLs

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
    memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));
    
    //some local printing for debugging
    printf("File size is: %d\n", file_size);
    
    //now start receiving file data
    size_t bytes_read = 0, total_bytes_read =0;;
    while (true)
    {
    	  //read max MAX_BUFFER_SIZE amount of file data
        bytes_read = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);

        //total number of bytes read so far
        total_bytes_read += bytes_read;

        if (bytes_read <= 0)
        {
            perror("Error receiving file data");
            return -1;
        }

		//write the buffer to the file
        file<<buffer;

	// reset buffer
        bzero(buffer, MAX_BUFFER_SIZE);
        
       //break out of the reading loop if read file_size number of bytes
        if (total_bytes_read >= file_size)
            break;
    }
    return 0;
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
    ssize_t bytesRead = recv_file(clientSocket, codeFile);
    if (bytesRead != 0) {
        cerr << "Error receiving data." << endl;
        close(clientSocket);
        pthread_exit(NULL);
    }
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

    // Check if the output is correct
    bool outputCorrect = isOutputCorrect(outputFileName);

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

void *workerFunc(void *args)
{
    while (true)
    {
        bool found=false;
        ThreadArgs* item=NULL;
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
            handleClient(item);
        }
    }
    return NULL;
}

void masterFunc(ThreadArgs*& item)
{
    queue_insert(item);
}

