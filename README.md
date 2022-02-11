# My C Compiler

This is my implementation of compiler for a subset of the C language.

## Building

To build, run 

    make

To build just the code, run
    
    make mycc

To build just the docs, run 
    
    make developers

To clean up all output files, run

    make clean

## Directory Structure

- `Documentation/`: Holds documentation, particularly the developer's guide
- `Grading/`: Directory for grader comments
- `Source/`: Holds all the source code
- `Test/`: Holds a set of test files to run over compiler

## Features

### Mode 0

- mycc compiles and can output usage information to stdout or a specified file.

### Mode 1

- tokenizes an input file
- outputs stream of tokens to output file