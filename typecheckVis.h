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
#ifndef __TYPE_CHECKER_VISITOR_H_
#define __TYPE_CHECKER_VISITOR_H_
#include "node.h"
#include "scope.h"
#include "visitor.h"
#include <map>
#include <string>

class TypeCheckerVisitor : public Visitor {
private:
  char* filename;
  std::map<int, std::string> fmap;
  bool verbose = false;
  Scope* scope; 
  std::list<SType*> types;
  std::string guard_sec = "low";
  bool passed = true;

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

  TypeCheckerVisitor();
  ~TypeCheckerVisitor();
  bool check(NBlock& root);
  void setFileName(char* filename);
  void printErrorMessage(std::string message, int lineno);
  void setVerbose(bool v) { verbose = v; };
  bool getVerbose() { return verbose; };
  bool getPassed() { return passed; };
};
#endif // __TYPE_CHECKER_VISITOR_H_
