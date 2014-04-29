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
  root.typeCheck(scope);//context->scope);
  assert(scope->depth() == 1);
  scope->FinalizeScope();
  delete scope;
  return true; // TODO
}

/* Returns an LLVM type based on the identifier */
static const Type *typeOf(const NType& type) 
{
	if (type.name.compare("int") == 0) {
		return Type::getInt64Ty(getGlobalContext());
	}
	else if (type.name.compare("double") == 0) {
		return Type::getDoubleTy(getGlobalContext());
	}
  else if (type.name.compare("bool") == 0) {
		return Type::getInt1Ty(getGlobalContext());
  }
	return Type::getVoidTy(getGlobalContext());
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

Type* NType::typeCheck(Scope* scope)
{
  assert(0);
}

Type* NIdentifier::typeCheck(Scope* scope)
{
	std::cout << "Type-checking identifier reference: " << name << std::endl;
	if (scope->LookUp(name) == NULL) {
		std::cerr << "undeclared variable " << name << std::endl;
		return NULL;
	}
  // TODO
	//return scope->LookUp(name)->type;
  return NULL;
}

Type* NAssignment::typeCheck(Scope* scope)
{
	std::cout << "Type-checking assignment " << lhs.name << std::endl;
	if (scope->LookUp(lhs.name) == NULL) {
		std::cerr << "undeclared variable " << lhs.name << std::endl;
		return NULL;
	}
  return NULL;
  // TODO
  /* 
  if (rhs.typeCheck(scope) == scope->LookUp(lhs.name)->type) {
  	return Type::getVoidTy(getGlobalContext());
  }
  std::cout << "Failed" << std::endl;
  */
}

Type* NVariableDeclaration::typeCheck(Scope* scope)
{
	std::cout << "Type-checking variable declaration " << type.name << " " << id.name << std::endl;
  /*
  scope->Insert(id.name, alloc);
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		assn.typeCheck(scope);
  }
	return alloc;
  */
  return NULL;
}

Type* NBinaryOperator::typeCheck(Scope* scope)
{
	std::cout << "Type-checking binary operation " << op << std::endl;
  /*
	Instruction::BinaryOps instr;
	switch (op) {
		case TPLUS: 	instr = Instruction::Add; goto math;
		case TMINUS: 	instr = Instruction::Sub; goto math;
		case TMUL: 		instr = Instruction::Mul; goto math;
		case TDIV: 		instr = Instruction::SDiv; goto math;
				
		// TODO comparison
	}

	return NULL;
math:
	return BinaryOperator::Create(instr, lhs.typeCheck(scope), 
		rhs.typeCheck(scope), "", Builder.GetInsertBlock());
  */
  return NULL;
}

Type* NIfExpression::typeCheck(Scope* scope)
{
	std::cout << "Type-checking if-then-else" << std::endl;
  /*
  // TODO: Must initialize a new scope for the two sides of the branches!
  Value *CondV = iguard.typeCheck(scope);
  if (CondV == 0) return 0;
  
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  
  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
  BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
  
  Builder.CreateCondBr(CondV, ThenBB, ElseBB);
  
  // Emit then value.
  Builder.SetInsertPoint(ThenBB);
  
  Value *ThenV = ithen.typeCheck(scope);
  if (ThenV == 0) return 0;
  
  Builder.CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = Builder.GetInsertBlock();
  
  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);
  
  Value *ElseV = ielse.typeCheck(scope);
  if (ElseV == 0) return 0;
  
  Builder.CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = Builder.GetInsertBlock();
  
  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);
  // TODO: Right now we only allow if-statements whose branches evaluate to doubles!
  PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), 2, "iftmp");
  
  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
  */
  return NULL;
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
    // TODO: Make sure each statement type is void
		//last = (**it).typeCheck(scope);
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
