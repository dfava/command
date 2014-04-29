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
  llvm::Function *mainFunction;

public:
  CodeGenContext* context;

  CodeGen() { };
  ~CodeGen() { delete context->scope; delete context->module; delete context; };
  void init();
  void generateCode(NBlock& root);
  llvm::GenericValue runCode();
  //llvm::Value *ErrorV(const char *Str) { Error(Str); return 0; } // dsf
};
#endif // __CODEGEN_H_
