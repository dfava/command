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
#include "node.h"
#include "scope.h"
#include "typecheckVis.h"
#include "parser.hpp"
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <fstream>
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
    passed = false;
    return;
	}
  types.push_front(sym->stype);
}

void TypeCheckerVisitor::visit(NAssignment* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
  Symbol* sym = scope->LookUp(element->lhs.name);
	if (sym == NULL) {
    printErrorMessage("Undeclared variable " + element->lhs.name, element->lineno);
    passed = false;
    return;
	}
  SType* dtype = sym->stype;
  if (dtype == NULL) {
    assert(!passed);
    return;
  }
  SType* atype = types.front();
  if (atype == NULL) {
    assert(!passed);
    return;
  }
  types.pop_front();
  // Check if the scope allow us to write to a low variable
  if (dtype->sec == "low" && scope->getSecurityContext() == "high") {
    printErrorMessage("Failed when trying to assign to a low var from a high context (implicit flow)", element->lineno);
    passed = false;
    return;
  }
  if (dtype->type != atype->type) {
    // TODO: Print legible types:
    std::cout << dtype->type << " " << atype->type << std::endl;
    printErrorMessage("Failed on types", element->lineno);
    passed = false;
    return;
  }
  // If the right hand side expression doesn't have a type,
  // its because it doesn't operate on variables.
  // In this case, its safe to allow this to proceed.
  if (dtype->sec == "low" && atype->sec == "high") {
    printErrorMessage("Failed on security (explicit flow)", element->lineno);
    passed = false;
    return;
  }
}

void TypeCheckerVisitor::visit(NVariableDeclaration* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << " " << element->type.name << " " << element->id.name << std::endl;
  Symbol* sym = scope->LookUp(element->id.name);
  if (sym != NULL) {
    printErrorMessage("Variable redeclaration " + element->id.name, element->lineno);
    passed = false;
    return;
  }
  SType* tmp;
  // Get info about NSecurity
  tmp =  types.front();
  types.pop_front();
  if (tmp == NULL) {
    assert(!passed);
    return;
  }
  std::string sec = tmp->sec;
  delete tmp;
  // Get info about NType
  tmp = types.front();
  types.pop_front();
  if (tmp == NULL) {
    assert(!passed);
    return;
  }
  Type* dtype = tmp->type;
  delete tmp;
  scope->Insert(element->id.name, new Symbol(NULL, new SType(dtype, sec)));
}

void TypeCheckerVisitor::visit(NBinaryOperator* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
	SType* trhs = types.front();
  types.pop_front();
  SType* tlhs = types.front();
  types.pop_front();
  if (tlhs == NULL || trhs == NULL) {
    assert(!passed);
    return;
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
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext())) {
        types.push_front(new SType(Type::getInt64Ty(getGlobalContext()), sec));
        return;
      } else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext())) {
        types.push_front(new SType(Type::getDoubleTy(getGlobalContext()), sec));
        return;
      }
    case TCEQ:
    case TCNE:
    case TCLT:
    case TCLE:
    case TCGT:
    case TCGE :
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext())) {
        types.push_front(new SType(Type::getInt1Ty(getGlobalContext()), sec));
        return;
      } else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext())) {
        types.push_front(new SType(Type::getInt1Ty(getGlobalContext()), sec));
        return;
      }
    default:
      printErrorMessage( "Type mismatch on binary operator", element->lineno );
      passed = false;
      return;
	}
}

void TypeCheckerVisitor::visit(NIfExpression* element, uint64_t flag)
{
  switch (flag)
  {
    case V_FLAG_GUARD | V_FLAG_EXIT:
      {
        if (verbose) std::cout << "TypeCheckerVisitor if-guard-enter " << typeid(element).name() << std::endl;
        SType* gtype = types.front();
        types.pop_front();
        assert(gtype != NULL);
        if (gtype->type != Type::getInt1Ty(getGlobalContext())) {
          printErrorMessage("Failed on the guard", element->lineno);
          passed = false;
          return;
        }
        guard_sec = gtype->sec;
      }
      return;
    case V_FLAG_EXIT:
      guard_sec = "";
      return;
    default:
      return;
  }
}

void TypeCheckerVisitor::visit(NWhileExpression* element, uint64_t flag)
{
  switch (flag)
  {
    case V_FLAG_GUARD | V_FLAG_EXIT:
      {
        if (verbose) std::cout << "TypeCheckerVisitor while-guard-enter " << typeid(element).name() << std::endl;
        SType* gtype = types.front();
        types.pop_front();
        assert(gtype != NULL);
        if (gtype->type != Type::getInt1Ty(getGlobalContext())) {
          printErrorMessage("Failed on the guard", element->lineno);
          passed = false;
          return;
        }
        guard_sec = gtype->sec;
      }
      return;
    case V_FLAG_EXIT:
      guard_sec = "";
      return;
    default:
      return;
  }
}

void TypeCheckerVisitor::visit(NExpressionStatement* element, uint64_t flag)
{
  if (verbose) std::cout << "TypeCheckerVisitor " << typeid(element).name() << std::endl;
}

void TypeCheckerVisitor::visit(NBlock* element, uint64_t flag)
{
  static int size_on_entering = 0;
  static int size_on_leaving = 0;
  switch (flag)
  {
    case V_FLAG_ENTER:
      {
        if (verbose) std::cout << "TypeCheckerVisitor entering " << typeid(element).name() << std::endl;
        size_on_entering = types.size();
        //std::cout << "Size on entering: " << size_on_entering << std::endl;;
        std::string next_sec = scope->getSecurityContext() == "high" ? "high" : guard_sec;
        next_sec = (next_sec == "" ? "low" : next_sec);
        if (verbose) std::cout << "TypeCheckerVisitor initializing scope to: " << next_sec << std::endl;;
        scope->InitializeScope("", next_sec);
      }
      break;
    case V_FLAG_EXIT:
      if (verbose) std::cout << "TypeCheckerVisitor leaving " << typeid(element).name() << std::endl;
      size_on_leaving = types.size();
      //std::cout << "Size on leaving: " << size_on_leaving << std::endl;;
      scope->FinalizeScope();
      break;
    default:
      assert(0);
  }
}
