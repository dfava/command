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

public:
  virtual void visit(NSkip* nSkip, uint64_t flag);
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

  TypeCheckerVisitor();
  ~TypeCheckerVisitor();
  bool check(NBlock& root);
  void setFileName(char* filename);
  void printErrorMessage(std::string message, int lineno);
  void setVerbose(bool v) { verbose = v; };
  bool getVerbose() { return verbose; };
};
#endif // __TYPE_CHECKER_VISITOR_H_
