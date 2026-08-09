// There exists a class called Submission. It contains these things, and these operations can be performed on it.

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP

// std::string is used for IDs, file paths and language names.
#include <string>

// This enum represents every major state that a submission
// can move through during the evaluation process.
enum class SubmissionStatus
{

    // The server has received the submission,
    // but processing has not started yet.
    RECEIVED,

    // The compiler is currently compiling the submitted code.
    COMPILING,

    // Compilation failed.
    COMPILATION_ERROR,

    // Compilation succeeded and the program is being executed.
    RUNNING,

    // The program crashed or terminated abnormally.
    RUNTIME_ERROR,

    // Execution succeeded and output is being checked.
    JUDGING,

    // Program output matched the expected output.
    ACCEPTED,

    // Program executed successfully but produced incorrect output.
    WRONG_ANSWER
};

// This class represents one CodeForge submission.
class Submission
{

private:
    // Unique identifier for this submission.
    //
    // Examples:
    // submission_1
    // submission_2
    std::string id;

    // Path of the submitted source-code file.
    //
    // Example:
    // ../samples/solution.cpp
    std::string sourcePath;

    // Programming language used by the submission.
    //
    // Currently we only support C++,
    // but later this could be "cpp", "python", "java", etc.
    std::string language;

    // Current state of this submission.
    SubmissionStatus status;

public:
    // Constructor.
    //
    // Called whenever we create a new Submission object.
    Submission(
        const std::string &id,
        const std::string &sourcePath,
        const std::string &language);

    // Returns the submission ID.
    std::string getId() const;

    // Returns the path of the source-code file.
    std::string getSourcePath() const;

    // Returns the programming language.
    std::string getLanguage() const;

    // Returns the current submission status.
    SubmissionStatus getStatus() const;

    // Changes the current submission status.
    void setStatus(SubmissionStatus newStatus);
};

#endif