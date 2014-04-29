#ifndef __CODEGEN_H_
#define __CODEGEN_H_
#include <stack>  // TODO: Delete
#include <list>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

class NBlock;

class SymbolTable {
private:
  std::string name;
  std::map<std::string, Value*> locals;

public:
  SymbolTable(std::string name) : name(name) { }
  void Insert(std::string name, Value* value) { locals[name] = value; }
  Value* LookUp(std::string name) {
    if (locals.find(name) == locals.end()) {
      return NULL;
    }
    return locals[name];
  }
};

class Scope {
private:
  std::list<SymbolTable*> scope;

public:
  Scope() { }
  int depth() { return scope.size();; }
  void InitializeScope(std::string name) {
    scope.push_front(new SymbolTable(name));
  }
  void FinalizeScope() {
    SymbolTable* toDelete = scope.front();
    scope.pop_front();
    delete toDelete;
  }
  void Insert(std::string name, Value* value) { scope.front()->Insert(name, value); }
  Value* LookUp(std::string name) {
    for (std::list<SymbolTable*>::iterator it=scope.begin(); it != scope.end(); ++it)
    {
      Value* val = (*it)->LookUp(name);
      if (val != NULL) return val;
    }
    return NULL;
  }
};

class CodeGenContext {
public:
  Scope* scope; 
  Module* module;
  CodeGenContext(Scope* scope, Module* module) : scope(scope), module(module) { }
};

class CodeGen {
private:
  Function *mainFunction;

public:
  CodeGenContext* context;

  CodeGen() { };
  ~CodeGen() { delete context->scope; delete context->module; delete context; };
  void init();
  void generateCode(NBlock& root);
  GenericValue runCode();
  //Value *ErrorV(const char *Str) { Error(Str); return 0; } // dsf
};
#endif // __CODEGEN_H_
