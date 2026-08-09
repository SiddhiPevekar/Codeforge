# Minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.16)

# Name of our project.
project(CodeForgeEvaluator)

# Compile using the C++17 standard.
set(CMAKE_CXX_STANDARD 17)

# Do not silently fall back to an older C++ standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Build one executable called codeforge_evaluator.
add_executable(codeforge_evaluator

    # Existing evaluator entry point.
    src/main.cpp

    # Implementation of our Submission class.
    src/submission.cpp
)


# Tell the compiler where our .hpp header files are located.
target_include_directories(
    codeforge_evaluator

    PRIVATE

    ${CMAKE_CURRENT_SOURCE_DIR}/include
)