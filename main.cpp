#include <iostream>
#include <stdio.h> // fopen
#include <unistd.h> // getopt
#include <libgen.h> // basename
#include "node.h"
#include "codegen.h"
#include "typecheck.h"

#define DEBUG 1
#define DPRNT(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

using namespace std;

extern int yyparse();
extern FILE* yyin;
extern NBlock* programBlock;

void usage(int argc, char** argv) {
    printf("%s:\n", basename(argv[0]));
    printf("  A compiler for the command language.\n");
}

int main(int argc, char **argv)
{
    bool typechecking = true;
    char* file = NULL;
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "f:ht:")) != -1)
       switch (c)
       {
       case 'f':
         file = optarg;
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
         abort();
       }
    if (file != NULL) {
      FILE* fhandle = fopen(file, "r");
      if (fhandle == NULL) {
        fprintf(stderr, "ERR: Could not open file %s\n", file);
        return 1;
      }
      printf( "%s\n", file);
      yyin = fhandle;
    }
    if (int ret = yyparse()) return ret;
    DPRNT("programBlock: %p\n", programBlock);
    if (typechecking) {
      TypeChecker typechecker;
      if (!typechecker.check(*programBlock)) {
        printf("Type checker failed\n");
        return 1;
      }
    }
    CodeGen codegen;
    codegen.init();
    codegen.generateCode(*programBlock);
    codegen.runCode();
    
    return 0;
}
