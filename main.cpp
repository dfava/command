// Copyright (C) 2014, Daniel S. Fava
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
#include <iostream>
#include <stdio.h> // fopen
#include <unistd.h> // getopt
#include <libgen.h> // basename
#include "node.h"
#include "visitor.h"
#include "typecheckVis.h"
#include "codegenVis.h"

#define DEBUG 0
#define DPRNT(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

using namespace std;

extern int yyparse();
extern FILE* yyin;
extern NBlock* programBlock;

void usage(int argc, char** argv) {
    printf("%s:\n", basename(argv[0]));
    printf("  A compiler for the command language.\n");
    printf("    -f [fname] : Input file.\n");
    printf("    -g [0,1]   : Turn code generation off (0) or on (1). Defaults to on.\n");
    printf("    -h         : Print usage.\n");
    printf("    -r [0,1]   : Turn running of the compiled code off (0) or on (1). Defaults to on.\n");
    printf("    -t [0,1]   : Turn type checking off (0) or on (1). Defaults to on.\n");
    printf("    -v [0,1]   : Turn verbosity off (0) or on (1). Defaults to off.\n");
}

int main(int argc, char **argv)
{
    bool typechecking = true;
    bool geningcode = true;
    bool verbose = false;
    bool running = true;
    char* filename = NULL;
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "f:g:hr:t:v:")) != -1)
       switch (c)
       {
       case 'f':
         filename = optarg;
         break;
       case 'g':
         if (strncmp(optarg, "0", 1)==0) {
           geningcode = false;
         } else if (strncmp(optarg, "1", 1)==0) {
           geningcode = true;
         } else {
           fprintf(stderr, "ERR: Options to -g are either 0 for no codegen or 1 for codegen\n" );
           return 1;
         }
         break;
       case 'r':
         if (strncmp(optarg, "0", 1)==0) {
           running = false;
         } else if (strncmp(optarg, "1", 1)==0) {
           running = true;
         } else {
           fprintf(stderr, "ERR: Options to -r are either 0 for no running or 1 for running\n" );
           return 1;
         }
         break;
       case 't':
         if (strncmp(optarg, "0", 1)==0) {
           typechecking = false;
         } else if (strncmp(optarg, "1", 1)==0) {
           typechecking = true;
         } else {
           fprintf(stderr, "ERR: Options to -t are either 0 for no type checking or 1 for type checking\n" );
           return 1;
         }
         break;
       case 'h':
         usage(argc, argv);
         return 1;
       case 'v':
         if (strncmp(optarg, "0", 1)==0) {
           verbose = false;
         } else if (strncmp(optarg, "1", 1)==0) {
           verbose = true;
         } else {
           fprintf(stderr, "ERR: Options to -v are either 0 for low verbosity or 1 for higher verbosity\n" );
           return 1;
         }
         break;
       default:
         fprintf(stderr, "Invalid command line options\n\n" );
         usage(argc, argv);
         return 1;
       }
    if (filename != NULL) {
      FILE* fhandle = fopen(filename, "r");
      if (fhandle == NULL) {
        fprintf(stderr, "ERR: Could not open file %s\n", filename);
        return 1;
      }
      DPRNT( "%s\n", filename);
      yyin = fhandle;
    }
    if (running) {
      geningcode = true; // Running forces code generation
    }
    if (int ret = yyparse()) return ret;
    DPRNT("programBlock: %p\n", programBlock);
    if (typechecking) {
      TypeCheckerVisitor typeCheckVis;
      typeCheckVis.setVerbose(verbose);
      if (filename != NULL) typeCheckVis.setFileName(filename); // For printing error messages
      programBlock->accept(typeCheckVis);
      if (!typeCheckVis.getPassed()) {
        printf("Type checker failed\n");
        return 1;
      } else {
        if (verbose) printf("Type-checking passed\n");
      }
    }
    if (geningcode) {
      CodeGenVisitor codeGenVis;
      codeGenVis.init();
      if (filename != NULL) codeGenVis.setFileName("tmp.bc");
      codeGenVis.setVerbose(verbose);
      programBlock->accept(codeGenVis);
      codeGenVis.generateCode();
      if (running) {
        codeGenVis.runCode();
      }
    }
    
    return 0;
}
