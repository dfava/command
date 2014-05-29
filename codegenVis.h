#ifndef __CODEGEN_VISITOR_H_
#define __CODEGEN_VISITOR_H_
#include "node.h"
#include "visitor.h"
#include "scope.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/GenericValue.h>

class CodeGenVisitor : public Visitor {
private:
  // Some state machine
  bool sm_inBlock = false;

  char* filename;
  bool verbose = false;
  llvm::Function *mainFunction;
  //llvm::IRBuilder<> *Builder = NULL;
  std::list<llvm::Value*> vals;

  class CodeGenContext {
  public:
    Scope* scope; 
    llvm::Module* module;
    CodeGenContext(Scope* scope, llvm::Module* module) : scope(scope), module(module) { }
  };

public:
  virtual void visit(NInteger* nInteger, uint64_t flag);
  virtual void visit(NBool* nBool, uint64_t flag);
  virtual void visit(NDouble* nDouble, uint64_t flag);
  virtual void visit(NType* nType, uint64_t flag);
  virtual void visit(NSecurity* nSecurity, uint64_t flag);
  virtual void visit(NIdentifier* nIdentifier, uint64_t flag);
  virtual void visit(NIfExpression* nIfExpression, uint64_t flag);
  virtual void visit(NBinaryOperator* nBinaryOperator, uint64_t flag);
  virtual void visit(NAssignment* nAssignment, uint64_t flag);
  virtual void visit(NBlock* nBlock, uint64_t flag);
  virtual void visit(NExpressionStatement* nExpressionStatement, uint64_t flag);
  virtual void visit(NVariableDeclaration* nVariableDeclaration, uint64_t flag);

  CodeGenContext* context;
  CodeGenVisitor() { };
  ~CodeGenVisitor() {
    if (context != NULL) {
      if (context->scope != NULL) delete context->scope;
      if (context->module != NULL) delete context->module;
      delete context;
    }
  };
  void init();
  void setFileName(char* filename) {this->filename = filename; };
  void generateCode();
  llvm::GenericValue runCode();
  void setVerbose(bool v) { verbose = v; };
  bool getVerbose() { return verbose; };
};

#endif // __CODEGEN_VISITOR_H_
