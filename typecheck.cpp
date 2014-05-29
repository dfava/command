#include "node.h"
#include "scope.h"
#include "typecheck.h"
#include "parser.hpp"
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace llvm;

extern TypeChecker typechecker;

void TypeChecker::setFileName(char* filename)
{
  this->filename = filename;
  std::ifstream input(filename);
  int lineno = 0;
  for(std::string line; getline(input, line);) {
    fmap[++lineno] = line;
    if (verbose) std::cout << line << std::endl;
  }
}

void TypeChecker::printErrorMessage(std::string message, int lineno)
{
  std::cerr << "ERR: " << message << std::endl;
  if (filename != NULL) {
    std::cerr << filename;
    if (lineno > 0) {
      std::cerr << " line " << lineno << ": " << std::endl << fmap[lineno] << std::endl;
    } else {
      std::cerr << std::endl;
    }
  }
}

/* Compile the AST into a module */
bool TypeChecker::check(NBlock& root)
{
  if (verbose) {
    std::cout << "Type-checking code...\n";
  }
  // Create a global scope
  Scope* scope = new Scope();
  scope->InitializeScope("global");
  SType* stype = root.typeCheck(scope);//context->scope);
  assert(scope->depth() == 1);
  scope->FinalizeScope();
  delete scope;
  if (stype == NULL || stype->type != Type::getVoidTy(getGlobalContext()))
    return false;
  return true;
}

SType* NInteger::typeCheck(Scope* scope)
{
  if (typechecker.getVerbose()) std::cout << "Type-checking integer: " << value << std::endl;
	return new SType(Type::getInt64Ty(getGlobalContext()), "");
}

SType* NDouble::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking double: " << value << std::endl;
	return new SType(Type::getDoubleTy(getGlobalContext()), "");
}

SType* NBool::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking boolean: " << value << std::endl;
  return new SType(Type::getInt1Ty(getGlobalContext()), "");
}

SType* NSecurity::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking security: " << name << std::endl;
  if (name == "") name = "low";
  return new SType(NULL, name);
}

SType* NType::typeCheck(Scope* scope)
{
	if (name.compare("int") == 0) {
		return new SType(Type::getInt64Ty(getGlobalContext()), "");
	}
	else if (name.compare("double") == 0) {
		return new SType(Type::getDoubleTy(getGlobalContext()), "");
	}
  else if (name.compare("bool") == 0) {
		return new SType(Type::getInt1Ty(getGlobalContext()), "");
  }
	return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NIdentifier::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking identifier reference: " << name << std::endl;
  Symbol* sym = scope->LookUp(name);
	if (sym == NULL) {
    typechecker.printErrorMessage("Undeclared variable " + name, lineno);
		return NULL;
	}
	return sym->stype;
}

SType* NAssignment::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking assignment " << lhs.name << std::endl;
  Symbol* sym = scope->LookUp(lhs.name);
	if (sym == NULL) {
    typechecker.printErrorMessage("Undeclared variable " + lhs.name, lineno);
		return NULL;
	}
  SType* dtype = sym->stype;
  SType* atype = rhs.typeCheck(scope);
  if (dtype == NULL || atype == NULL) return NULL;
  // Check if the scope allow us to write to a low variable
  if (dtype->sec == "low" && scope->getSecurityContext() == "high") {
    typechecker.printErrorMessage("Failed when trying to assign to a low var from a high context (implicit flow)", lineno);
    return NULL;
  }
  if (dtype->type != atype->type) {
    typechecker.printErrorMessage("Failed on types", lineno);
    return NULL;
  }
  // If the right hand side expression doesn't have a type,
  // its because it doesn't operate on variables.
  // In this case, its safe to allow this to proceed.
  if (dtype->sec == "low" && atype->sec == "high") {
    typechecker.printErrorMessage("Failed on security (explicit flow)", lineno);
    return NULL;
  }
	return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NVariableDeclaration::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking variable declaration " << type.name << " " << id.name << std::endl;
  Symbol* sym = scope->LookUp(id.name);
  if (sym != NULL) {
    typechecker.printErrorMessage("Variable redeclaration " + id.name, lineno);
    return NULL;
  }
  SType* tmp;
  tmp = ((NType&) type).typeCheck(scope);
  if (tmp == NULL) return NULL;
  Type* dtype = tmp->type;
  delete tmp;
  tmp = security.typeCheck(scope);
  if (tmp == NULL) return NULL;
  std::string sec = tmp->sec;
  delete tmp;
  sym = new Symbol(NULL, new SType(dtype, sec));
  scope->Insert(id.name, sym);
  // Check that the assignment part
	if (assignmentExpr != NULL) {
		NAssignment assign(id, *assignmentExpr);
		SType* atype = assign.typeCheck(scope);
    if (atype == NULL) return NULL;
  }
	return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NBinaryOperator::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking binary operation " << op << std::endl;

  SType* tlhs = lhs.typeCheck(scope);
	SType* trhs = rhs.typeCheck(scope);
  if (tlhs == NULL || trhs == NULL) return NULL;
  std::string sec = "";
  if (tlhs->sec == "high" || trhs->sec == "high") {
    sec = "high";
  }

	switch (op) {
		case TPLUS:
		case TMINUS:
		case TMUL:
		case TDIV:
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext()))
        return new SType(Type::getInt64Ty(getGlobalContext()), sec);
      else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext()))
        return new SType(Type::getDoubleTy(getGlobalContext()), sec);
    case TCEQ:
    case TCNE:
    case TCLT:
    case TCLE:
    case TCGT:
    case TCGE :
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext()))
        return new SType(Type::getInt1Ty(getGlobalContext()), sec);
      else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext()))
        return new SType(Type::getInt1Ty(getGlobalContext()), sec);
    default:
      typechecker.printErrorMessage( "Type mismatch on binary operator", lineno );
	    return NULL;
	}
}

SType* NIfExpression::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking if-then-else" << std::endl;
  SType* gtype = iguard.typeCheck(scope);
  if (gtype == NULL) return NULL;
  if (gtype->type != Type::getInt1Ty(getGlobalContext())) {
    typechecker.printErrorMessage("Failed on the guard", lineno);
	  return NULL;
  }

  if (typechecker.getVerbose()) std::cout << "(Branch) Creating a new scope of security type: " << gtype->sec << std::endl;
  scope->InitializeScope("branch", (scope->getSecurityContext() == "high" ? "high" : gtype->sec)); // Create a new scope for this branch
  SType* ttype = ithen.typeCheck(scope);
  if (ttype == NULL) {
    scope->FinalizeScope();
    return NULL;
  }
  if (ttype->type != Type::getVoidTy(getGlobalContext())) {
    typechecker.printErrorMessage("Failed on the then", lineno);
    scope->FinalizeScope();
	  return NULL;
  }
  scope->FinalizeScope();

  scope->InitializeScope("branch", (scope->getSecurityContext() == "high" ? "high" : gtype->sec)); // Create a new scope for this branch
  SType* etype = ielse.typeCheck(scope);
  if (etype == NULL) {
    scope->FinalizeScope();
    return NULL;
  }
  if (etype->type != Type::getVoidTy(getGlobalContext())) {
    typechecker.printErrorMessage("Failed on the else", lineno);
    scope->FinalizeScope();
	  return NULL;
  }
  scope->FinalizeScope();

  return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NExpressionStatement::typeCheck(Scope* scope)
{
	if (typechecker.getVerbose()) std::cout << "Type-checking " << typeid(expression).name() << std::endl;
	return expression.typeCheck(scope);
}

SType* NBlock::typeCheck(Scope* scope)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		if (typechecker.getVerbose()) std::cout << "Type-checking " << typeid(**it).name() << std::endl;
		SType* stype = (**it).typeCheck(scope);
    if (stype == NULL) return NULL;
    if (stype->type != Type::getVoidTy(getGlobalContext())) {
    typechecker.printErrorMessage("Failed to typecheck to void", lineno);
      return NULL;
    }
	}
	if (typechecker.getVerbose()) std::cout << "Type-checking block" << std::endl;
	return new SType(Type::getVoidTy(getGlobalContext()), "");
}
