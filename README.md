# CodeForge

A scalable and fault-tolerant online code evaluation platform.

## Goals

CodeForge will provide:

- Concurrent code submission and execution
- Asynchronous job processing
- Worker-pool based request handling
- Automated test-case evaluation
- Fault-tolerant submission recovery
- Resource-isolated code execution
- Performance monitoring and load testing
- Web-based code editor and dashboard

## Installation

### macOS

The following setup is recommended for macOS, including Apple Silicon Macs such as M1, M2, M3, and M4.

#### 1. Install Xcode Command Line Tools

The Command Line Tools provide the macOS SDK, compiler tools, and Apple's Clang compiler.

```bash
xcode-select --install

clang++ --version
git --version
make --version

brew --version

brew update
brew install git gh cmake llvm


clang --version
clang++ --version

## The responsibilities will be:
Submission: represents one submitted program, with an ID, source path, language, status, etc.
Compiler: compiles the submitted C++ file and returns success or compilation error.
Executor: runs the compiled binary with a test input and captures output.
Judge: compares actual output against expected output and returns ACCEPTED or WRONG_ANSWER.
main.cpp: only coordinates these pieces.