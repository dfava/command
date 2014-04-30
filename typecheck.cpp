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
  Type* type = root.typeCheck(scope);//context->scope);
  assert(scope->depth() == 1);
  scope->FinalizeScope();
  delete scope;
  if (type != Type::getVoidTy(getGlobalContext()))
    return false;
  return true;
}

Type* NInteger::typeCheck(Scope* scope)
{
	std::cout << "Type-checking integer: " << value << std::endl;
	return Type::getInt64Ty(getGlobalContext());
}

Type* NDouble::typeCheck(Scope* scope)
{
	std::cout << "Type-checking double: " << value << std::endl;
	return Type::getDoubleTy(getGlobalContext());
}

Type* NBool::typeCheck(Scope* scope)
{
	std::cout << "Type-checking boolean: " << value << std::endl;
  return Type::getInt1Ty(getGlobalContext());
}

Type* NSecurity::typeCheck(Scope* scope)
{
  // TODO
	std::cout << "Type-checking security: " << name << std::endl;
  return NULL;
}

Type* NType::typeCheck(Scope* scope)
{
	if (name.compare("int") == 0) {
		return Type::getInt64Ty(getGlobalContext());
	}
	else if (name.compare("double") == 0) {
		return Type::getDoubleTy(getGlobalContext());
	}
  else if (name.compare("bool") == 0) {
		return Type::getInt1Ty(getGlobalContext());
  }
	return Type::getVoidTy(getGlobalContext());
}

Type* NIdentifier::typeCheck(Scope* scope)
{
	std::cout << "Type-checking identifier reference: " << name << std::endl;
	if (scope->LookUp(name) == NULL) {
		std::cerr << "undeclared variable " << name << std::endl;
		return NULL;
	}
	return scope->LookUp(name)->type;
}

Type* NAssignment::typeCheck(Scope* scope)
{
	std::cout << "Type-checking assignment " << lhs.name << std::endl;
	if (scope->LookUp(lhs.name) == NULL) {
		std::cerr << "undeclared variable " << lhs.name << std::endl;
		return NULL;
	}
  Type* dtype = scope->LookUp(lhs.name)->type;
  Type* atype = rhs.typeCheck(scope);
  if (dtype != atype) {
    // TODO: Better error message
    std::cout << "Failed" << std::endl;
    return NULL;
  }
	return Type::getVoidTy(getGlobalContext());
}

Type* NVariableDeclaration::typeCheck(Scope* scope)
{
	std::cout << "Type-checking variable declaration " << type.name << " " << id.name << std::endl;
  Type* dtype = ((NType&) type).typeCheck(scope); // Drop the "const"
  Symbol* sym = new Symbol(NULL, dtype);
  scope->Insert(id.name, sym);
  // Check that the assignment part
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		Type* atype = assn.typeCheck(scope);
  }
	return Type::getVoidTy(getGlobalContext());
}

Type* NBinaryOperator::typeCheck(Scope* scope)
{
	std::cout << "Type-checking binary operation " << op << std::endl;

  Type* tlhs = lhs.typeCheck(scope);
	Type* trhs = rhs.typeCheck(scope);

	switch (op) {
		case TPLUS:
		case TMINUS:
		case TMUL:
		case TDIV:
      if (tlhs == trhs && tlhs == Type::getInt64Ty(getGlobalContext()))
        return Type::getInt64Ty(getGlobalContext());
      else if (tlhs == trhs && tlhs == Type::getDoubleTy(getGlobalContext()))
        return Type::getDoubleTy(getGlobalContext());
    case TCEQ:
    case TCNE:
    case TCLT:
    case TCLE:
    case TCGT:
    case TCGE :
      if (tlhs == trhs && tlhs == Type::getInt64Ty(getGlobalContext()))
        return Type::getInt1Ty(getGlobalContext());
      else if (tlhs == trhs && tlhs == Type::getDoubleTy(getGlobalContext()))
        return Type::getInt1Ty(getGlobalContext());
    default:
      std::cout << "Failed on NBinaryOperator" << std::endl;
	    return NULL;
      break;
	}
}

Type* NIfStatement::typeCheck(Scope* scope)
{
	std::cout << "Type-checking if-then-else" << std::endl;
  Type* gtype = iguard.typeCheck(scope);
  if (gtype != Type::getInt1Ty(getGlobalContext())) {
    // TODO: Better error message
    std::cout << "Failed on the guard" << std::endl;
	  return NULL;
  }
  Type* ttype = ithen.typeCheck(scope);
  if (ttype != Type::getVoidTy(getGlobalContext())) {
    // TODO: Better error message
    std::cout << "Failed on the then" << std::endl;
	  return NULL;
  }
  Type* etype = ielse.typeCheck(scope);
  if (etype != Type::getVoidTy(getGlobalContext())) {
    // TODO: Better error message
    std::cout << "Failed on the else" << std::endl;
	  return NULL;
  }
  return Type::getVoidTy(getGlobalContext());
}

Type* NExpressionStatement::typeCheck(Scope* scope)
{
	std::cout << "Type-checking " << typeid(expression).name() << std::endl;
	return expression.typeCheck(scope);
}

Type* NBlock::typeCheck(Scope* scope)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		std::cout << "Type-checking " << typeid(**it).name() << std::endl;
		Type* type = (**it).typeCheck(scope);
    if (type != Type::getVoidTy(getGlobalContext())) {
      std::cout << "Failed to typecheck to void" << std::endl;
      return NULL;
    }
	}
	std::cout << "Type-checking block" << std::endl;
	return Type::getVoidTy(getGlobalContext());
}

Type* NMethodCall::typeCheck(Scope* scope)
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
Type* NFunctionDeclaration::typeCheck(Scope* scope)
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
