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
  class If {
  public:
    llvm::Function *function = NULL;
    llvm::BasicBlock *thenBB = NULL;
    llvm::BasicBlock *elseBB = NULL;
    llvm::BasicBlock *mergeBB = NULL;
  };
  class While {
  public:
    llvm::Function *function = NULL;
    llvm::BasicBlock *condBB = NULL;
    llvm::BasicBlock *bodyBB = NULL;
    llvm::BasicBlock *endBB = NULL;
  };

  char* filename;
  bool verbose = false;
  llvm::Function *mainFunction;
  //llvm::IRBuilder<> *Builder = NULL;
  std::list<llvm::Value*> vals;
  std::list<If*> ifs;
  std::list<While*> whiles;

  class CodeGenContext {
  public:
    Scope* scope; 
    llvm::Module* module;
    CodeGenContext(Scope* scope, llvm::Module* module) : scope(scope), module(module) { }
  };

public:
  virtual void visit(NSkip* nSkip, uint64_t flag);
  virtual void visit(NInteger* nInteger, uint64_t flag);
  virtual void visit(NBool* nBool, uint64_t flag);
  virtual void visit(NDouble* nDouble, uint64_t flag);
  virtual void visit(NType* nType, uint64_t flag);
  virtual void visit(NSecurity* nSecurity, uint64_t flag);
  virtual void visit(NIdentifier* nIdentifier, uint64_t flag);
  virtual void visit(NIfExpression* nIfExpression, uint64_t flag);
  virtual void visit(NWhileExpression* nWhileExpression, uint64_t flag);
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
