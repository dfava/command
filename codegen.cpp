#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace std;

static IRBuilder<> Builder(getGlobalContext());
void CodeGen::init()
{
  Module* m = new Module("main", getGlobalContext());
  Scope* s = new Scope();
  context = new CodeGenContext(s, m);
}

/* Compile the AST into a module */
void CodeGen::generateCode(NBlock& root)
{
	std::cout << "Generating code...\n";
  // Create a global scope
  context->scope->InitializeScope("global");
  // Create a main function and add the first basic block to it
  ArrayRef<llvm::Type *> argTypes;
	FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", context->module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
  // Set the Builder's basic block to this first block from the main function
  Builder.SetInsertPoint(bblock);
  // Traverse AST and generate code
  root.codeGen(context->scope);
  Builder.CreateRetVoid();
  // Cleanup scopes
  assert(context->scope->depth() == 1);
  context->scope->FinalizeScope();
  // Validate the generated code, checking for consistency.
  verifyFunction(*mainFunction);
  // Dump IR to screen
	std::cout << "Code is generated." << std::endl;
  context->module->dump();
}

/* Executes the AST by running the main function */
GenericValue CodeGen::runCode() {
	std::cout << "Running code...\n";
  InitializeNativeTarget();
  ExecutionEngine* ee = EngineBuilder(context->module).create();
  assert(ee != 0);
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	std::cout << "Code was run.\n";
  return v;
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

/* -- Code Generation -- */

Value* NInteger::codeGen(Scope* scope)
{
	std::cout << "Creating integer: " << value << std::endl;
	return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

Value* NDouble::codeGen(Scope* scope)
{
	std::cout << "Creating double: " << value << std::endl;
	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* NBool::codeGen(Scope* scope)
{
	std::cout << "Creating boolean: " << value << std::endl;
  if (value.compare("true") == 0)
	  return ConstantInt::getTrue(getGlobalContext());
  assert (value.compare("false") == 0);
	return ConstantInt::getFalse(getGlobalContext());
}

Value* NType::codeGen(Scope* scope)
{
  assert(0);
}

Value* NIdentifier::codeGen(Scope* scope)
{
	std::cout << "Creating identifier reference: " << name << std::endl;
	if (scope->LookUp(name) == NULL) {
		std::cerr << "undeclared variable " << name << std::endl;
		return NULL;
	}
	return new LoadInst(scope->LookUp(name), "", false, Builder.GetInsertBlock());
}

Value* NAssignment::codeGen(Scope* scope)
{
	std::cout << "Creating assignment for " << lhs.name << std::endl;
	if (scope->LookUp(lhs.name) == NULL) {
		std::cerr << "undeclared variable " << lhs.name << std::endl;
		return NULL;
	}
	return new StoreInst(rhs.codeGen(scope), scope->LookUp(lhs.name), false, Builder.GetInsertBlock());
}

Value* NVariableDeclaration::codeGen(Scope* scope)
{
	std::cout << "Creating variable declaration " << type.name << " " << id.name << std::endl;
	AllocaInst *alloc = new AllocaInst( (llvm::Type *) typeOf(type), id.name.c_str(), Builder.GetInsertBlock());
  scope->Insert(id.name, alloc);
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		assn.codeGen(scope);
  }
	return alloc;
}

Value* NBinaryOperator::codeGen(Scope* scope)
{
	std::cout << "Creating binary operation " << op << std::endl;
	Instruction::BinaryOps instr;
	switch (op) {
		case TPLUS: 	instr = Instruction::Add; goto math;
		case TMINUS: 	instr = Instruction::Sub; goto math;
		case TMUL: 		instr = Instruction::Mul; goto math;
		case TDIV: 		instr = Instruction::SDiv; goto math;
				
		/* TODO comparison */
	}

	return NULL;
math:
	return BinaryOperator::Create(instr, lhs.codeGen(scope), 
		rhs.codeGen(scope), "", Builder.GetInsertBlock());
}

Value* NIfExpression::codeGen(Scope* scope)
{
	std::cout << "Generating code for if-then-else" << std::endl;

  // TODO: Must initialize a new scope for the two sides of the branches!
  Value *CondV = iguard.codeGen(scope);
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
  
  Value *ThenV = ithen.codeGen(scope);
  if (ThenV == 0) return 0;
  
  Builder.CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = Builder.GetInsertBlock();
  
  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);
  
  Value *ElseV = ielse.codeGen(scope);
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
}

Value* NExpressionStatement::codeGen(Scope* scope)
{
	std::cout << "Generating code for " << typeid(expression).name() << std::endl;
	return expression.codeGen(scope);
}

Value* NBlock::codeGen(Scope* scope)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		std::cout << "Generating code for " << typeid(**it).name() << std::endl;
		last = (**it).codeGen(scope);
	}
	std::cout << "Creating block" << std::endl;
	return last;
}

Value* NMethodCall::codeGen(Scope* scope)
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
		args.push_back((**it).codeGen(st));
	}
	CallInst *call = CallInst::Create(function, args, "", context.currentBlock());
	std::cout << "Creating method call: " << id.name << std::endl;
	return call;
  //*/
  return NULL;
}

// TODO: Treat argument list as "NType NIdentifier, ...." 
// as opposed to "NTIdentifier NIdentifier, ...."
Value* NFunctionDeclaration::codeGen(Scope* scope)
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
		(**it).codeGen(st);
	}
	
	block.codeGen(st);
	ReturnInst::Create(getGlobalContext(), bblock);

	context.popBlock();
	std::cout << "Creating function: " << id.name << std::endl;
	return function;
  //*/
  return NULL;
}
