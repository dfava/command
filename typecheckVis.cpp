#include "node.h"
#include "scope.h"
#include "typecheckVis.h"
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

TypeCheckerVisitor::TypeCheckerVisitor()
{
  scope = new Scope();
  scope->InitializeScope("global");
}

TypeCheckerVisitor::~TypeCheckerVisitor() {
  assert(scope->depth() == 1);
  scope->FinalizeScope();
  delete scope;
}

void TypeCheckerVisitor::setFileName(char* filename)
{
  this->filename = filename;
  std::ifstream input(filename);
  int lineno = 0;
  for(std::string line; getline(input, line);) {
    fmap[++lineno] = line;
    if (verbose) std::cout << line << std::endl;
  }
}

void TypeCheckerVisitor::printErrorMessage(std::string message, int lineno)
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

void TypeCheckerVisitor::visit(NSkip* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
	types.push_front(new SType(Type::getVoidTy(getGlobalContext()), ""));
}

void TypeCheckerVisitor::visit(NInteger* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
	types.push_front(new SType(Type::getInt64Ty(getGlobalContext()), ""));
}

void TypeCheckerVisitor::visit(NDouble* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
	types.push_front(new SType(Type::getDoubleTy(getGlobalContext()), ""));
}

void TypeCheckerVisitor::visit(NBool* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
  types.push_front(new SType(Type::getInt1Ty(getGlobalContext()), ""));
}

void TypeCheckerVisitor::visit(NSecurity* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << " " << element->name << std::endl;
  if (element->name == "") element->name = "low";
  types.push_front(new SType(NULL, element->name));
}

void TypeCheckerVisitor::visit(NType* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
	if (element->name.compare("int") == 0) {
		types.push_front(new SType(Type::getInt64Ty(getGlobalContext()), ""));
    return;
	} else if (element->name.compare("double") == 0) {
		types.push_front(new SType(Type::getDoubleTy(getGlobalContext()), ""));
    return;
	} else if (element->name.compare("bool") == 0) {
		types.push_front(new SType(Type::getInt1Ty(getGlobalContext()), ""));
    return;
  }
	types.push_front(new SType(Type::getVoidTy(getGlobalContext()), ""));
  return;
}

void TypeCheckerVisitor::visit(NIdentifier* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << " " << element->name << std::endl;
  Symbol* sym = scope->LookUp(element->name);
	if (sym == NULL) {
    printErrorMessage("Undeclared variable " + element->name, element->lineno);
    // TODO
		//return NULL;
	}
  // TODO
	// return sym->stype;
}

void TypeCheckerVisitor::visit(NAssignment* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
  Symbol* sym = scope->LookUp(element->lhs.name);
	if (sym == NULL) {
    printErrorMessage("Undeclared variable " + element->lhs.name, element->lineno);
    // TODO
    assert(0);
		//return NULL;
	}
  SType* dtype = sym->stype;
  SType* atype = types.front();
  types.pop_front();
  if (dtype == NULL || atype == NULL) {
    // TODO
    assert(0);
    //return NULL;
  }
  // Check if the scope allow us to write to a low variable
  if (dtype->sec == "low" && scope->getSecurityContext() == "high") {
    printErrorMessage("Failed when trying to assign to a low var from a high context (implicit flow)", element->lineno);
    // TODO
    assert(0);
    //return NULL;
  }
  if (dtype->type != atype->type) {
    // TODO: Print legible types:
    std::cout << dtype->type << " " << atype->type << std::endl;
    printErrorMessage("Failed on types", element->lineno);
    // TODO
    assert(0);
    //return NULL;
  }
  // If the right hand side expression doesn't have a type,
  // its because it doesn't operate on variables.
  // In this case, its safe to allow this to proceed.
  if (dtype->sec == "low" && atype->sec == "high") {
    printErrorMessage("Failed on security (explicit flow)", element->lineno);
    // TODO
    assert(0);
    //return NULL;
  }
}

void TypeCheckerVisitor::visit(NVariableDeclaration* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << " " << element->type.name << " " << element->id.name << std::endl;
  Symbol* sym = scope->LookUp(element->id.name);
  if (sym != NULL) {
    printErrorMessage("Variable redeclaration " + element->id.name, element->lineno);
    // TODO
    assert(0);
    // return NULL;
  }
  SType* tmp;
  // Get info about NSecurity
  tmp =  types.front();
  types.pop_front();
  if (tmp == NULL) {
    // TODO
    assert(0);
    //return NULL;
  }
  std::string sec = tmp->sec;
  delete tmp;
  // Get info about NType
  tmp = types.front();
  types.pop_front();
  if (tmp == NULL) {
    // TODO
    assert(0);
    // return NULL;
  }
  Type* dtype = tmp->type;
  delete tmp;
  scope->Insert(element->id.name, new Symbol(NULL, new SType(dtype, sec)));
}

void TypeCheckerVisitor::visit(NBinaryOperator* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
  SType* tlhs = element->lhs.typeCheck(scope);//TODO
	SType* trhs = element->rhs.typeCheck(scope);//TODO
  if (tlhs == NULL || trhs == NULL) {
    // TODO
    // return NULL;
  }
  std::string sec = "";
  if (tlhs->sec == "high" || trhs->sec == "high") {
    sec = "high";
  }

	switch (element->op) {
		case TPLUS:
		case TMINUS:
		case TMUL:
		case TDIV:
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext()))
        // TODO
        new SType(Type::getInt64Ty(getGlobalContext()), sec);
      else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext()))
        // TODO
        new SType(Type::getDoubleTy(getGlobalContext()), sec);
    case TCEQ:
    case TCNE:
    case TCLT:
    case TCLE:
    case TCGT:
    case TCGE :
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext()))
        // TODO
        new SType(Type::getInt1Ty(getGlobalContext()), sec);
      else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext()))
        // TODO
        new SType(Type::getInt1Ty(getGlobalContext()), sec);
    default:
      printErrorMessage( "Type mismatch on binary operator", element->lineno );
      // TODO
	    // return NULL;
	}
}

void TypeCheckerVisitor::visit(NIfExpression* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
  SType* gtype = element->iguard.typeCheck(scope);//TODO
  if (gtype == NULL) {
    // TODO
    //return NULL;
  }
  if (gtype->type != Type::getInt1Ty(getGlobalContext())) {
    printErrorMessage("Failed on the guard", element->lineno);
    // TODO
	  //return NULL;
  }

  if (getVerbose()) std::cout << "(Branch) Creating a new scope of security type: " << gtype->sec << std::endl;
  scope->InitializeScope("branch", (scope->getSecurityContext() == "high" ? "high" : gtype->sec)); // Create a new scope for this branch
  SType* ttype = element->ithen.typeCheck(scope);//TODO
  if (ttype == NULL) {
    scope->FinalizeScope();
    // TODO
    //return NULL;
  }
  if (ttype->type != Type::getVoidTy(getGlobalContext())) {
    printErrorMessage("Failed on the then", element->lineno);
    scope->FinalizeScope();
    // TODO
	  //return NULL;
  }
  scope->FinalizeScope();

  scope->InitializeScope("branch", (scope->getSecurityContext() == "high" ? "high" : gtype->sec)); // Create a new scope for this branch
  SType* etype = element->ielse.typeCheck(scope);//TODO
  if (etype == NULL) {
    scope->FinalizeScope();
    // TODO
    //return NULL;
  }
  if (etype->type != Type::getVoidTy(getGlobalContext())) {
    printErrorMessage("Failed on the else", element->lineno);
    scope->FinalizeScope();
    // TODO
	  //return NULL;
  }
  scope->FinalizeScope();

  // TODO
  new SType(Type::getVoidTy(getGlobalContext()), "");
}

void TypeCheckerVisitor::visit(NExpressionStatement* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
  // TODO
	element->expression.typeCheck(scope);//TODO
}

void TypeCheckerVisitor::visit(NBlock* element, uint64_t flag)
{
  static int size_on_entering = 0;
  static int size_on_leaving = 0;
  switch (flag)
  {
    case V_FLAG_ENTER:
      if (verbose) std::cout << "TypeCheckerVisitor entering " << typeid(element).name() << std::endl;
      size_on_entering = types.size();
      std::cout << "Size on entering: " << size_on_entering << std::endl;;
      scope->InitializeScope();
      break;
    case V_FLAG_EXIT:
      if (verbose) std::cout << "TypeCheckerVisitor leaving " << typeid(element).name() << std::endl;
      size_on_leaving = types.size();
      std::cout << "Size on leaving: " << size_on_leaving << std::endl;;
      scope->FinalizeScope();
      break;
    default:
      assert(0);
  }
}
