#include <iostream>   // std::cout
#include <fstream>    // std::ifstream
#include <iterator>   // std::istreambuf_iterator
#include <string>     // std::string
#include <cstdlib>    // std::system

// Reads the entire contents of a file and returns it as a string.
std::string readFile(const std::string& path) {

    // Open the file located at 'path' for reading.
    std::ifstream file(path);

    // If the file could not be opened, return an empty string.
    if (!file.is_open()) {
        return "";
    }

    // Read every character from the file until EOF
    // and construct one std::string containing the complete file.
    return std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
}
std::string trimTrailingWhitespace(std::string str) {
    while (!str.empty() &&
           (str.back() == ' ' ||
            str.back() == '\n' ||
            str.back() == '\r' ||
            str.back() == '\t')) {
        str.pop_back();
    }

    return str;
}

int main() {

    // Path of the user's submitted C++ source code.
    const std::string sourceFile = "../samples/solution.cpp";

    // Input that will be supplied to the submitted program.
    const std::string inputFile = "../testcases/input.txt";

    // Correct output that the submitted program should produce.
    const std::string expectedFile = "../testcases/expected.txt";

    // Location where the compiled executable will be created.
    const std::string executable = "../runtime/program";

    // File where the submitted program's output will be stored.
    const std::string outputFile = "../runtime/output.txt";

    // File where compiler errors will be stored.
    const std::string compileErrorFile =
        "../runtime/compile_error.txt";

    // Display program heading.
    std::cout << "CodeForge Evaluator\n";
    std::cout << "-------------------\n";


    // =========================================================
    // STEP 1: COMPILE THE SUBMITTED SOURCE CODE
    // =========================================================

    // Build a terminal command such as:
    //
    // clang++ -std=c++17 ../samples/solution.cpp
    //         -o ../runtime/program
    //         2> ../runtime/compile_error.txt
    //
    // -std=c++17 -> compile using C++17
    // -o          -> specify executable output path
    // 2>          -> redirect stderr into compile_error.txt

    std::string compileCommand =
        "clang++ -std=c++17 " +
        sourceFile +
        " -o " +
        executable +
        " 2> " +
        compileErrorFile;

    // Execute the compiler command using the operating system shell.
    //
    // A return value of 0 generally means compilation succeeded.
    // A non-zero value means compilation failed.
    int compileResult = std::system(compileCommand.c_str());

    // Check whether compilation failed.
    if (compileResult != 0) {

        std::cout << "Verdict: COMPILATION_ERROR\n\n";

        // Print the compiler error that was redirected
        // into compile_error.txt.
        std::cout << readFile(compileErrorFile);

        // Stop evaluation because an executable was not produced.
        return 0;
    }

    std::cout << "Compilation successful.\n";


    // =========================================================
    // STEP 2: EXECUTE THE COMPILED PROGRAM
    // =========================================================

    // Build a command such as:
    //
    // ../runtime/program
    //     < ../testcases/input.txt
    //     > ../runtime/output.txt
    //
    // < redirects input.txt to stdin of the program.
    // > redirects stdout of the program to output.txt.

    std::string runCommand =
        executable +
        " < " +
        inputFile +
        " > " +
        outputFile;

    // Execute the compiled submission.
    int runResult = std::system(runCommand.c_str());

    // If the process exits abnormally or returns a non-zero code,
    // currently classify it as a runtime error.
    if (runResult != 0) {

        std::cout << "Verdict: RUNTIME_ERROR\n";

        return 0;
    }

    std::cout << "Execution successful.\n";


    // =========================================================
    // STEP 3: READ ACTUAL AND EXPECTED OUTPUT
    // =========================================================

    // Read output produced by the submitted program.
    std::string actualOutput = readFile(outputFile);

    // Read the correct output stored with the testcase.
    std::string expectedOutput = readFile(expectedFile);


    // =========================================================
    // STEP 4: JUDGE THE SUBMISSION
    // =========================================================

    // Compare the program's output with the expected output.
    if (trimTrailingWhitespace(actualOutput) ==
    trimTrailingWhitespace(expectedOutput)) {

        // Exact match means the solution passed this testcase.
        std::cout << "Verdict: ACCEPTED\n";

    } else {

        // Outputs do not exactly match.
        std::cout << "Verdict: WRONG_ANSWER\n";

        // Show correct output.
        std::cout << "\nExpected:\n";
        std::cout << expectedOutput;

        // Show output generated by the submission.
        std::cout << "\nReceived:\n";
        std::cout << actualOutput;
    }

    return 0;
}