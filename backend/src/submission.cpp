// Include the declaration of our Submission class.
#include "submission.hpp"


// Constructor implementation.
//
// The values passed while creating a Submission
// are copied into the object's member variables.
Submission::Submission(
    const std::string& id,
    const std::string& sourcePath,
    const std::string& language
)
    : id(id),                   // Store the submission ID.
      sourcePath(sourcePath),   // Store the source-code path.
      language(language),       // Store the selected language.
      status(SubmissionStatus::RECEIVED) // Every new submission starts as RECEIVED.
{
}


// Return this submission's unique ID.
std::string Submission::getId() const
{
    return id;
}


// Return the path containing the submitted source code.
std::string Submission::getSourcePath() const
{
    return sourcePath;
}


// Return the language used for the submission.
std::string Submission::getLanguage() const
{
    return language;
}


// Return the current processing status.
SubmissionStatus Submission::getStatus() const
{
    return status;
}


// Update the processing status.
//
// Later Compiler, Executor and Judge will call this.
//
// Example:
//
// submission.setStatus(SubmissionStatus::COMPILING);
void Submission::setStatus(SubmissionStatus newStatus)
{
    status = newStatus;
}