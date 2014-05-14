#include <iostream>
#include <stdio.h> // fopen
#include <unistd.h> // getopt
#include <libgen.h> // basename
#include "node.h"
#include "codegen.h"
#include "typecheck.h"

#define DEBUG 0
#define DPRNT(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

using namespace std;

extern int yyparse();
extern FILE* yyin;
extern NBlock* programBlock;

TypeChecker typechecker;

void usage(int argc, char** argv) {
    printf("%s:\n", basename(argv[0]));
    printf("  A compiler for the command language.\n");
    printf("    -f [fname] : Input file.\n");
    printf("    -g [0,1]   : Turn code generation off (0) or on (1). Defaults to on.\n");
    printf("    -h         : Print usage.\n");
    printf("    -t [0,1]   : Turn type checking off (0) or on (1). Defaults to on.\n");
}

int main(int argc, char **argv)
{
    bool typechecking = true;
    bool codegen = true;
    char* filename = NULL;
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "f:g:ht:")) != -1)
       switch (c)
       {
       case 'f':
         filename = optarg;
         break;
       case 'g':
         if (strncmp(optarg, "0", 1)==0) {
           codegen = false;
         } else if (strncmp(optarg, "1", 1)==0) {
           codegen = true;
         } else {
           fprintf(stderr, "ERR: Options to -g are either 0 for no codegen or 1 for codegen\n" );
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
    if (int ret = yyparse()) return ret;
    DPRNT("programBlock: %p\n", programBlock);
    if (typechecking) {
      if (filename != NULL) typechecker.setFileName(filename); // For printing error messages
      if (!typechecker.check(*programBlock)) {
        printf("Type checker failed\n");
        return 1;
      }
    }
    if (codegen) {
      CodeGen codegen;
      codegen.init();
      codegen.generateCode(*programBlock);
      codegen.runCode();
    }
    
    return 0;
}
