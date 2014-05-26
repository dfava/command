#ifndef __CODEGEN_H_
#define __CODEGEN_H_
#include "scope.h"
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/GenericValue.h>

class NBlock;

class CodeGenContext {
public:
  Scope* scope; 
  llvm::Module* module;
  CodeGenContext(Scope* scope, llvm::Module* module) : scope(scope), module(module) { }
};

class CodeGen {
private:
  char* filename;
  bool verbose = false;
  llvm::Function *mainFunction;

public:
  CodeGenContext* context;
  CodeGen() { };
  ~CodeGen() {
    if (context != NULL) {
      if (context->scope != NULL) delete context->scope;
      if (context->module != NULL) delete context->module;
      delete context;
    }
  };
  void init();
  void setFileName(char* filename) {this->filename = filename; };
  void generateCode(NBlock& root);
  llvm::GenericValue runCode();
  void setVerbose(bool v) { verbose = v; };
  bool getVerbose() { return verbose; };
};
#endif // __CODEGEN_H_
