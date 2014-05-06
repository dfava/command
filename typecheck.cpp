#include "node.h"
#include "typecheck.h"
#include "parser.hpp"
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>

using namespace llvm;
using namespace std;

/* Compile the AST into a module */
bool TypeChecker::check(NBlock& root)
{
	std::cout << "Type-checking code...\n";
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
	std::cout << "Type-checking integer: " << value << std::endl;
	return new SType(Type::getInt64Ty(getGlobalContext()), "");
}

SType* NDouble::typeCheck(Scope* scope)
{
	std::cout << "Type-checking double: " << value << std::endl;
	return new SType(Type::getDoubleTy(getGlobalContext()), "");
}

SType* NBool::typeCheck(Scope* scope)
{
	std::cout << "Type-checking boolean: " << value << std::endl;
  return new SType(Type::getInt1Ty(getGlobalContext()), "");
}

SType* NSecurity::typeCheck(Scope* scope)
{
	std::cout << "Type-checking security: " << name << std::endl;
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
	std::cout << "Type-checking identifier reference: " << name << std::endl;
  Symbol* sym = scope->LookUp(name);
	if (sym == NULL) {
		std::cerr << "undeclared variable " << name << std::endl;
		return NULL;
	}
	return sym->stype;
}

SType* NAssignment::typeCheck(Scope* scope)
{
	std::cout << "Type-checking assignment " << lhs.name << std::endl;
  Symbol* sym = scope->LookUp(lhs.name);
	if (sym == NULL) {
		std::cerr << "undeclared variable " << lhs.name << std::endl;
		return NULL;
	}
  SType* dtype = sym->stype;
  SType* atype = rhs.typeCheck(scope);
  if (dtype == NULL || atype == NULL) return NULL;
  if (dtype->type != atype->type) {
    // TODO: Better error message
    std::cout << "Failed on types" << std::endl;
    return NULL;
  }
  // If the right hand side expression doesn't have a type,
  // its because it doesn't operate on variables.
  // In this case, its safe to allow this to proceed.
  if (dtype->sec == "low" && atype->sec == "high") {
    // TODO: Better error message
    std::cout << "Failed on security (explicit flow)" << std::endl;
    return NULL;
  }
	return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NVariableDeclaration::typeCheck(Scope* scope)
{
	std::cout << "Type-checking variable declaration " << type.name << " " << id.name << std::endl;
  Symbol* sym = scope->LookUp(id.name);
  if (sym != NULL) {
    std::cout << "Variable redeclaration" << std::endl;
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
		NAssignment assn(id, *assignmentExpr);
		SType* atype = assn.typeCheck(scope);
    if (atype == NULL) return NULL;
  }
	return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NBinaryOperator::typeCheck(Scope* scope)
{
	std::cout << "Type-checking binary operation " << op << std::endl;

  SType* tlhs = lhs.typeCheck(scope);
	SType* trhs = rhs.typeCheck(scope);
  if (tlhs == NULL || trhs == NULL) return NULL;

	switch (op) {
		case TPLUS:
		case TMINUS:
		case TMUL:
		case TDIV:
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext()))
        return new SType(Type::getInt64Ty(getGlobalContext()), "");
      else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext()))
        return new SType(Type::getDoubleTy(getGlobalContext()), "");
    case TCEQ:
    case TCNE:
    case TCLT:
    case TCLE:
    case TCGT:
    case TCGE :
      if (tlhs->type == trhs->type && tlhs->type == Type::getInt64Ty(getGlobalContext()))
        return new SType(Type::getInt1Ty(getGlobalContext()), "");
      else if (tlhs->type == trhs->type && tlhs->type == Type::getDoubleTy(getGlobalContext()))
        return new SType(Type::getInt1Ty(getGlobalContext()), "");
    default:
      std::cout << "Failed on NBinaryOperator" << std::endl;
	    return NULL;
      break;
	}
}

SType* NIfExpression::typeCheck(Scope* scope)
{
	std::cout << "Type-checking if-then-else" << std::endl;
  SType* gtype = iguard.typeCheck(scope);
  if (gtype == NULL) return NULL;
  if (gtype->type != Type::getInt1Ty(getGlobalContext())) {
    // TODO: Better error message
    std::cout << "Failed on the guard" << std::endl;
	  return NULL;
  }
  SType* ttype = ithen.typeCheck(scope);
  if (ttype == NULL) return NULL;
  if (ttype->type != Type::getVoidTy(getGlobalContext())) {
    // TODO: Better error message
    std::cout << "Failed on the then" << std::endl;
	  return NULL;
  }
  SType* etype = ielse.typeCheck(scope);
  if (etype == NULL) return NULL;
  if (etype->type != Type::getVoidTy(getGlobalContext())) {
    // TODO: Better error message
    std::cout << "Failed on the else" << std::endl;
	  return NULL;
  }
  return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NExpressionStatement::typeCheck(Scope* scope)
{
	std::cout << "Type-checking " << typeid(expression).name() << std::endl;
	return expression.typeCheck(scope);
}

SType* NBlock::typeCheck(Scope* scope)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		std::cout << "Type-checking " << typeid(**it).name() << std::endl;
		SType* stype = (**it).typeCheck(scope);
    if (stype == NULL) return NULL;
    if (stype->type != Type::getVoidTy(getGlobalContext())) {
      std::cout << "Failed to typecheck to void" << std::endl;
      return NULL;
    }
	}
	std::cout << "Type-checking block" << std::endl;
	return new SType(Type::getVoidTy(getGlobalContext()), "");
}

SType* NMethodCall::typeCheck(Scope* scope)
{
  // TODO: CONTEXT
  /*
	Function *function = TheModule->getFunction(id.name.c_str());
	if (function == NULL) {
		std::cerr << "no such function " << id.name << std::endl;
	}
	std::vector<Value*> args;
	ExpressionList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		args.push_back((**it).typeCheck(st));
	}
	CallInst *call = CallInst::Create(function, args, "", context.currentBlock());
	std::cout << "Type-checking method call: " << id.name << std::endl;
	return call;
  //*/
  return NULL;
}

// TODO: Treat argument list as "NType NIdentifier, ...." 
// as opposed to "NTIdentifier NIdentifier, ...."
SType* NFunctionDeclaration::typeCheck(Scope* scope)
{
  // TODO: CONTEXT
  /*
  SmallVector<llvm::Type *, 100> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		argTypes.push_back((llvm::Type*)typeOf((**it).type));
	}
	FunctionType *ftype = FunctionType::get((llvm::Type *)typeOf(type), argTypes, false);
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), TheModule);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);

	context.pushBlock(bblock);

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).typeCheck(st);
	}
	
	block.typeCheck(st);
	ReturnInst::Create(getGlobalContext(), bblock);

	context.popBlock();
	std::cout << "Type-checking function: " << id.name << std::endl;
	return function;
  //*/
  return NULL;
}
